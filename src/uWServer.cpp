//
//

#include "uWServer.h"

using namespace uWS;

namespace uWServer{

    uWServer::uWServer(int _port):port(_port) {
        //create object hub instance:
        this->h =  new uWS::Hub();
    };

    void uWServer::run() {
        pthread_create(&this->_t, nullptr, this->runThread, this);
    };

    void uWServer::send(std::string msg) {
        if(!this->connected){
            printf("No Clients connected, skipping send!\n");
            return;
        }
        // send to all clients:
        for(auto cptr: this->connections)
                cptr->send(msg.c_str());
    }

    void uWServer::send(std::vector<char> *cArray) {
        // send binary char array
        if(!this->connected){
            printf("No Clients connected, skipping send!\n");
            return;
        }
        std::string msg(cArray->begin(), cArray->end());
        auto m = reinterpret_cast<char*>(cArray->data());
        // send to all clients:
        for(auto cptr: this->connections){
            cptr->send(msg.c_str(),msg.size(),OpCode::BINARY);
        }
    }

    void uWServer::sendRaw(std::vector<char> *cArray) {
        // send binary char array
        if(!this->connected){
            printf("No Clients connected, skipping send!\n");
            return;
        }
        // send to all clients:
        for(auto cptr: this->connections){
            cptr->send(cArray->data(),cArray->size(),OpCode::BINARY);
        }
    }

    void uWServer::sendRaw(const std::string &msg) {
        if(!this->connected){
            printf("No Clients connected, skipping send!\n");
            return;
        }
        // send to all clients:
        for(auto cptr: this->connections)
            cptr->send(msg.c_str(), msg.size(), OpCode::BINARY);
//            std::this_thread::sleep_for(std::chrono::milliseconds(10));}
    }

    void uWServer::_run(){
        // do actual work here...
        // does this mean the hub is a server or a client?
        this->h->onConnection([this](uServer ws, uWS::HttpRequest req) {
              std::cout << "A client connected" << std::endl;
              printf("%s\n",req.headers->value);
              // seems like theres a new pointer per connected client; need to manage this better.
              pthread_mutex_lock(&this->_lock);
              this->connections.emplace_back(ws);
              this->connected = true;
              pthread_mutex_unlock(&this->_lock);
          }
        );

        this->h->onDisconnection([this](uServer ws, int code, char *message, size_t length) {
            std::cout << "CLIENT CLOSE: " << code << std::endl;
            std::vector<uServer>::iterator it;
            it = find (this->connections.begin(), this->connections.end(), ws);
            if (it != this->connections.end()){

                pthread_mutex_lock(&this->_lock);
                this->connections.erase(it);
                // set connection state based on how many connected clients there are:
                this->connected = !this->connections.empty();
                pthread_mutex_unlock(&this->_lock);
                printf("Client removed!\n");
                if (this->connections.empty()){
                    printf("All Clients disconnected!\n");
                }
            }else{
                printf("client NOT found in array\n");
            }
        });

//        this->h->onHttpRequest([](uWS::HttpResponse *res, uWS::HttpRequest req, char *data, size_t, size_t) {
//            const std::string s = "<h1>Hello world!</h1>";
//            if (req.getUrl().valueLength == 1)
//            {
//                res->end(s.data(), s.length());
//            }
//            else
//            {
//                // i guess this should be done more gracefully?
//                res->end(nullptr, 0);
//            }
//        });

        this->h->onMessage([this](uServer ws, char *message, size_t length, uWS::OpCode opCode){
            // could match ws to client list if we really wanted to...
            // could push message into local context work queue.
            std::string rmsg(message, length);
            pthread_mutex_lock(&this->_lockRead);
            // fix amount of memory used for buffer... it will drop oldest messages if exceeded.
            if (this->buffer.size()==MAX_DEQUE_LENGTH)
                this->buffer.pop_front();
            this->buffer.emplace_back(rmsg);
            pthread_mutex_unlock(&this->_lockRead);
        });



        if (this->h->listen("0.0.0.0",this->port)) {
            std::cout << "Listening on port " << this->port << std::endl;
            this->h->run();
        }
        pthread_exit(nullptr);
    };

    void uWServer::stop(){
        pthread_kill(this->_t, 0);
    }

    std::string uWServer::read(){
        pthread_mutex_lock(&this->_lockRead);
        std::string ret;
        if (!this->buffer.empty()){
            ret = this->buffer.front();
            this->buffer.pop_front();
        }
        pthread_mutex_unlock(&this->_lockRead);
        return ret;
    }



}