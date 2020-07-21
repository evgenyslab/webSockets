#include <uWS.h>
#include <iostream>
#include <algorithm>
#include <thread>
#include <fstream>
#include <nlohmann/json.hpp>

using namespace uWS;

uint64_t now(){
    return (std::chrono::duration_cast<std::chrono::nanoseconds >(std::chrono::system_clock::now().time_since_epoch())).count();
}

typedef uWS::WebSocket<uWS::SERVER>* uServer;
typedef uWS::WebSocket<uWS::CLIENT>* uClient;

struct ct{
    uWS::Hub *h= nullptr;
    std::vector<uServer> hptr = {};
    bool connected = false;
    pthread_mutex_t _lock = PTHREAD_MUTEX_INITIALIZER;
};

struct cts{
    uWS::Hub *h= nullptr;
    std::vector<uClient> hptr = {};
    bool connected = false;
    pthread_mutex_t _lock = PTHREAD_MUTEX_INITIALIZER;
};

std::vector<char> readFile(const char* filename)
{
    // open the file:
    std::ifstream file(filename, std::ios::binary);

    // Stop eating new lines in binary mode!!!
    file.unsetf(std::ios::skipws);

    // get its size:
    std::streampos fileSize;

    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // reserve capacity
    std::vector<char> vec;
    vec.reserve(fileSize);

    // read the data:
    vec.insert(vec.begin(),
               std::istream_iterator<char>(file),
               std::istream_iterator<char>());

    return vec;
}

void * fun(void *ptr){
    auto *context = (ct*) ptr;
    printf("I've made it into my thread!\n");
    while(!context->connected){
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    context->hptr[0]->send("why hello there\n");
    pthread_exit(nullptr);
}

void *send_heartbeat(void *ptr){
    auto *context = (ct*) ptr;
    while(1){
        if(context->connected){
            auto t = now();
            char tt[19];
            sprintf(tt, "%ld",t);
            // create JSON string:
            std::string jmsg = "{\"epoch\": \"" + std::string(tt) + "\"}";
            // send to all clients:
            for(auto cptr: context->hptr)
                cptr->send(jmsg.c_str());
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));

    }

    pthread_exit(nullptr);

}

void send(const std::string msg, void *ptr){
    auto *context = (ct*) ptr;
    if(!context->connected){
        printf("No Clients connected, skipping send!\n");
        return;
    }
    // create JSON string:
    nlohmann::json j = {"message", msg};
//    std::string jmsg = "{\"message\": \"" + msg + "\"}";
    // send to all clients:
    for(auto cptr: context->hptr)
        cptr->send(j.dump().c_str());
}

void sendc(const std::string msg, void *ptr){
    auto *context = (cts*) ptr;
    if(!context->connected){
        printf("No Clients connected, skipping send!\n");
        return;
    }
    // create JSON string:
    nlohmann::json j = {"message", msg};
//    std::string jmsg = "{\"message\": \"" + msg + "\"}";
    // send to all clients:
    for(auto cptr: context->hptr)
        cptr->send(j.dump().c_str());
}

void sendImage(const std::string &msg, void *ptr){
    auto *context = (ct*) ptr;
    if(!context->connected){
        printf("No Clients connected, skipping send!\n");
        return;
    }
    // send to all clients:
    for(auto cptr: context->hptr)
        // sending as binary fixes UTF-8 encoding/decoding problem..
        // this needs to be sent as binary, it will be received as a BLOB on the brower's side
        cptr->send(msg.c_str(), msg.size(),OpCode::BINARY);
}

void * webserver(void *ptr){


    ct * localContext = (ct*) ptr;
    // does this mean the hub is a server or a client?
    localContext->h->onConnection([localContext](uServer ws, uWS::HttpRequest req) {
        std::cout << "A client connected" << std::endl;
        // seems like theres a new pointer per connected client; need to manage this better.
        pthread_mutex_lock(&localContext->_lock);
        localContext->hptr.emplace_back(ws);
        localContext->connected = true;
        pthread_mutex_unlock(&localContext->_lock);
    }
    );

    // TOOD: implement this to handle clients correctly...
    localContext->h->onDisconnection([localContext](uServer ws, int code, char *message, size_t length) {
        std::cout << "CLIENT CLOSE: " << code << std::endl;
        std::vector<uServer>::iterator it;
        it = find (localContext->hptr.begin(), localContext->hptr.end(), ws);
        if (it != localContext->hptr.end()){

            pthread_mutex_lock(&localContext->_lock);
            localContext->hptr.erase(it);
            // set connection state based on how many connected clients there are:
            localContext->connected = localContext->hptr.size() > 0;
            pthread_mutex_unlock(&localContext->_lock);
            printf("Client removed!\n");
            if (localContext->hptr.size()==0){
                printf("All Clients disconnected!\n");
            }
        }else{
            printf("client NOT found in array\n");
        }
    });

    localContext->h->onHttpRequest([](uWS::HttpResponse *res, uWS::HttpRequest req, char *data, size_t, size_t) {
        const std::string s = "<h1>Hello world!</h1>";
        if (req.getUrl().valueLength == 1)
        {
            res->end(s.data(), s.length());
        }
        else
        {
            // i guess this should be done more gracefully?
            res->end(nullptr, 0);
        }
    });

    localContext->h->onMessage([localContext](uServer ws, char *message, size_t length, uWS::OpCode opCode){
        // could match ws to client list if we really wanted to...
        // could push message into local context work queue.
        std::string rmsg(message, length);
        nlohmann::json j;
        bool jsondata = false;
        try{
            j = nlohmann::json::parse(rmsg);
            jsondata = true;
        }catch(const std::exception& e){
            // could not parse JSON
            jsondata = false;
        }
        // KNOWN ISSUE OF HANDLING '\n' in MESSAGE!
        if (jsondata){

            printf("\nJSON Message Received:\n");
            std::cout << j.dump(2) << std::endl;
//            send(j.dump(),localContext);
        }else{
            printf("\nMessage Received: <%s>\n", rmsg.c_str());
            send(rmsg, localContext);
        }

    });



    if (localContext->h->listen("0.0.0.0",13049)) {
        std::cout << "Listening on port 13049" << std::endl;
        localContext->h->run();
    }
    pthread_exit(nullptr);
}


void * webclient(void *ptr){


    cts * localContext = (cts*) ptr;
    // does this mean the hub is a server or a client?
    localContext->h->onConnection([localContext](uClient ws, uWS::HttpRequest req) {
                                      std::cout << "Client: A client connected" << std::endl;
                                      // seems like theres a new pointer per connected client; need to manage this better.
                                      pthread_mutex_lock(&localContext->_lock);
                                      localContext->hptr.emplace_back(ws);
                                      localContext->connected = true;
                                      pthread_mutex_unlock(&localContext->_lock);
                                  }
    );

    // TOOD: implement this to handle clients correctly...
    localContext->h->onDisconnection([localContext](uClient ws, int code, char *message, size_t length) {
        std::cout << "Client CLIENT CLOSE: " << code << std::endl;
        std::vector<uClient>::iterator it;
        it = find (localContext->hptr.begin(), localContext->hptr.end(), ws);
        if (it != localContext->hptr.end()){

            pthread_mutex_lock(&localContext->_lock);
            localContext->hptr.erase(it);
            // set connection state based on how many connected clients there are:
            localContext->connected = localContext->hptr.size() > 0;
            pthread_mutex_unlock(&localContext->_lock);
            printf("Client removed!\n");
            if (localContext->hptr.size()==0){
                printf("All Clients disconnected!\n");
            }
        }else{
            printf("client NOT found in array\n");
        }
    });

    localContext->h->onHttpRequest([](uWS::HttpResponse *res, uWS::HttpRequest req, char *data, size_t, size_t) {
        const std::string s = "<h1>Hello world!</h1>";
        if (req.getUrl().valueLength == 1)
        {
            res->end(s.data(), s.length());
        }
        else
        {
            // i guess this should be done more gracefully?
            res->end(nullptr, 0);
        }
    });

    localContext->h->onMessage([localContext](uClient ws, char *message, size_t length, uWS::OpCode opCode){
        // could match ws to client list if we really wanted to...
        // could push message into local context work queue.
        std::string rmsg(message, length);
        nlohmann::json j;
        bool jsondata = false;
        try{
            j = nlohmann::json::parse(rmsg);
            jsondata = true;
        }catch(const std::exception& e){
            // could not parse JSON
            jsondata = false;
        }
        // KNOWN ISSUE OF HANDLING '\n' in MESSAGE!
        if (jsondata){

            printf("\nClient JSON Message Received:\n");
            std::cout << j.dump(2) << std::endl;
            sendc(j.dump(),localContext);
        }else{
            printf("\nClient Message Received: <%s>\n", rmsg.c_str());
            sendc(rmsg, localContext);
        }

    });


    // connect?
    localContext->h->connect("ws://127.0.0.1:13049", (void *) 1);
    printf("Starting Client\n");
    localContext->h->run();

    pthread_exit(nullptr);
}


int main() {

    // instantiate object...
    ct localContext;
    // create local uWS hub object:
    uWS::Hub h;
    // attach hub to local context object:
    localContext.h = &h;
    // create websever thread
    pthread_t ws;
    // run webserver thread (handles client connections/disconnections):
    pthread_create(&ws, nullptr, webserver, &localContext);

    // instantiate object...
    cts localContexts;
    // create local uWS hub object:
    uWS::Hub hs;
    // attach hub to local context object:
    localContexts.h = &hs;
    // create websever thread
    pthread_t wss;
    // run webserver thread (handles client connections/disconnections):
    pthread_create(&wss, nullptr, webclient, &localContexts);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    // create clock thread (sends clock info to client):
    pthread_t clock;
    // run the clock thread:
    pthread_create(&clock, nullptr, send_heartbeat, &localContext);

    // dummy string for grabbing keyboard input:
    std::string cmd;
    // initialize exit condition:
    bool exit = false;
    // wait for a moment while uwebsockets starts...
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::vector<char> f;
    while(!exit){
        printf(">> ");
        std::getline (std::cin,cmd);
        if (cmd == "exit"){
            exit = true;
            pthread_kill(ws, 0);
            pthread_kill(clock,0);
            continue;
        }else if (cmd =="load") {
            f = readFile("/Users/en/Git/Sockets/uWebSockets/test.jpeg");
            std::string immsg(f.begin(), f.end());
            sendImage(immsg, &localContext);
        }else{
                // send message to connected client (browser)
                send(cmd, &localContext);
        }
    }
}