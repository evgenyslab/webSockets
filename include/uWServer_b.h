/*
 *
 * server
 *
 * thread -> set up + poll in cycle // runs indefinately until quit message
 * thread -> read from queue, write to queue
 *
 * */

#include <uWS.h>
#include <thread>
#include <deque>
#include <iostream>

#define MAX_MESSAGE_QUEUE 100

using namespace uWS;

class uWServer_b{
private:
    // run thread
    pthread_t _tid;
    // received queue
    std::deque<std::string> rxqueue;
    pthread_mutex_t _rxmutex = PTHREAD_MUTEX_INITIALIZER;
    // client lock:
    pthread_mutex_t _lock = PTHREAD_MUTEX_INITIALIZER;
    // connection list:
    std::vector<uWS::WebSocket<uWS::SERVER>* > connections = {};
    // port
    int port = 0;
    //
    bool connected = false;

    // functions:
    // helper function
    static void *__run__(void *context) {
        ((uWServer_b *) context)->_run();
        return nullptr;
    }
    // run function:
    void _run(){
        // create hub object local to thread:
        uWS::Hub h;

        h.onConnection([this](uWS::WebSocket<uWS::SERVER>* ws, uWS::HttpRequest req) {
           std::cout << "A client connected" << std::endl;
           printf("%s\n",req.headers->value);
           // seems like theres a new pointer per connected client; need to manage this better.
           pthread_mutex_lock(&this->_lock);
           this->connections.emplace_back(ws);
           this->connected = true;
           pthread_mutex_unlock(&this->_lock);
                       }
        );

        h.onDisconnection([this](uWS::WebSocket<uWS::SERVER>* ws, int code, char *message, size_t length) {
            std::cout << "CLIENT CLOSE: " << code << std::endl;
            std::vector<uWS::WebSocket<uWS::SERVER>*>::iterator it;
            it = find (this->connections.begin(), this->connections.end(), ws);
            if (it != connections.end()){

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
                // this should be an error...
                printf("client NOT found in array\n");
            }
        });


        h.onMessage([this](uWS::WebSocket<uWS::SERVER>* ws, char *message, size_t length, uWS::OpCode opCode){
            // lock queue
            pthread_mutex_lock(&this->_rxmutex);
            // put message into queue
            this->rxqueue.emplace_back(std::string(message,length));
            pthread_mutex_unlock(&this->_rxmutex);
        });

        if (h.listen("0.0.0.0",this->port)) {
            printf("Server listening on port: %d\n", this->port);
            // add async to server hub to correctly handle asynchronous sending...
            h.getDefaultGroup<uWS::SERVER>().addAsync();
            h.run();
        }

    }

public:

    uWServer_b(int port):port(port){};
    ~uWServer_b() = default;

    void config(){

    };
    // creates and runs a thread with a hub based on config
    void run(){
        pthread_create(&this->_tid, nullptr, this->__run__, this);
    };
    void stop(){
        pthread_kill(this->_tid, 0);
    };
    // send message
    void _send(char * cmsg, size_t l, bool BINARY = true){
        std::string msg(cmsg,l);
        if (BINARY){
            for (auto c:this->connections)
                c->send(msg.c_str(),msg.size(),OpCode::BINARY);
        }else{
            for (auto c:this->connections)
                c->send(msg.c_str(),msg.size(),OpCode::TEXT);
        }
    }

    // blocking reads message from queue
    void read_blocking(std::string &ret){
        bool received = false;
        while(!received){
            // lock queue
            pthread_mutex_lock(&this->_rxmutex);
            if(!this->rxqueue.empty()){
                ret = this->rxqueue.front();
                this->rxqueue.pop_front();
                received = true;
            }
            pthread_mutex_unlock(&this->_rxmutex);
            if(!received)
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }


    };
    // reads message from queue
    void read(std::string &ret){
        // lock queue
        pthread_mutex_lock(&this->_rxmutex);
        if(!this->rxqueue.empty()){
            ret = this->rxqueue.front();
            this->rxqueue.pop_front();
            }
        pthread_mutex_unlock(&this->_rxmutex);
    };
    // reads all message from queue
    void read(std::vector<std::string> &ret){
        // lock queue
        pthread_mutex_lock(&this->_rxmutex);
        while(!this->rxqueue.empty()){
            std::string r;
            this->read(r);
            ret.emplace_back(r);
            this->rxqueue.pop_front();
        }
        pthread_mutex_unlock(&this->_rxmutex);
    };

};