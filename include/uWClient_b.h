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

class uWClient_b{
private:
    // run thread
    pthread_t _tid, _tih;
    // received queue
    std::deque<std::string> rxqueue;
    pthread_mutex_t _rxmutex = PTHREAD_MUTEX_INITIALIZER;
    // send queue
    std::deque<std::string> txqueue;
    pthread_mutex_t _txmutex = PTHREAD_MUTEX_INITIALIZER;
    // port
    int port = 0;
    //
    bool connected = false;
    bool exit = false;
    // client
    uWS::WebSocket<uWS::CLIENT>* client;

    // functions:
    // helper function
    static void *__run__(void *context) {
        ((uWClient_b *) context)->_run();
        return nullptr;
    }

    // hall monitor function
    static void *__monitor__(void *context) {
        ((uWClient_b *) context)->_hallMonitor();
        return nullptr;
    }
    // run function:
    void _run(){
        // create hub object local to thread:
        uWS::Hub h;
        // TODO: need to only allow one server at a time; can't handle this right now, seems to have
        // problems when server connects

        h.onConnection([this](uWS::WebSocket<uWS::CLIENT>* ws, uWS::HttpRequest req) {
           std::cout << "Client connected to Server on port: " << this->port << std::endl;
           // seems like theres a new pointer per connected client; need to manage this better.
            printf("%s\n",req.headers->value);
           this->connected = true;
           this->client = ws;
           }
        );

        h.onDisconnection([this](uWS::WebSocket<uWS::CLIENT>* ws, int code, char *message, size_t length) {
            std::cout << "CLIENT Disconnected from Server with code: " << code << std::endl;
            this->connected = false;
        });


        h.onMessage([this](uWS::WebSocket<uWS::CLIENT>* ws, char *message, size_t length, uWS::OpCode opCode){
            // lock queue
            pthread_mutex_lock(&this->_rxmutex);
            // place message:
            this->rxqueue.emplace_back(std::string(message,length));
            // unlock queue
            pthread_mutex_unlock(&this->_rxmutex);
        });

        // try to connect, I suspect might need to wrap this on the whole thread...
        h.connect("ws://127.0.0.1:" + std::to_string(this->port), (void *) 1);
        h.run(); // <- blocking call

    }

    void _hallMonitor(){
        /*
         * this is a hallway monitor...
         * it periodically checks if the client has been connected or is not connected
         * for a period of time. If not, then it kills the hub thread & restarts is.
         * */
        while(!this->exit){
            std::this_thread::sleep_for(std::chrono::seconds(5));
            if (!this->connected){
                this->stop();
                pthread_create(&this->_tid, nullptr, this->__run__, this);
            }
        }
    }

public:

    uWClient_b(int port):port(port){};
    ~uWClient_b() = default;

    void config(){

    };
    // creates and runs a thread with a hub based on config
    void run(){
        pthread_create(&this->_tid, nullptr, this->__run__, this);
        // add run monitor...
        pthread_create(&this->_tih, nullptr, this->__monitor__, this);
    };
    void stop(){
        pthread_kill(this->_tid, 0);
    };

    // send message
    void _send(char * cmsg, size_t l, bool BINARY = true){
        std::string msg(cmsg,l);
        if (BINARY){
            this->client->send(msg.c_str(),msg.size(),OpCode::BINARY);
        }else{
            this->client->send(msg.c_str(),msg.size(),OpCode::TEXT);
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