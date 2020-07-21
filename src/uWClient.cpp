//
//

#include "uWClient.h"

using namespace uWS;

namespace uWClient{

    uWClient::uWClient(int _port):port(_port) {
        //create object hub instance:
        this->h =  new uWS::Hub();
    };

    void uWClient::run() {
        pthread_create(&this->_t, nullptr, this->runThread, this);
    };

    void uWClient::send(std::string msg) {
        if(!this->connected){
            printf("No Clients connected, skipping send!\n");
            return;
        }
        // send to all clients:
        for(auto cptr: this->connections)
                cptr->send(msg.c_str());
    }

    void uWClient::send(std::vector<char> *cArray) {
        // send binary char array
        if(!this->connected){
            printf("No Clients connected, skipping send!\n");
            return;
        }
        std::string msg(cArray->begin(), cArray->end());
        // send to all clients:
        for(auto cptr: this->connections)
            cptr->send(msg.c_str(),msg.size(),OpCode::BINARY);
    }


    void uWClient::_tryReconnect(){
        pthread_mutex_lock(&this->_lock);
        bool _connected = this->connected;
        pthread_mutex_unlock(&this->_lock);
        while(!_connected){
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            this->h->connect("ws://127.0.0.1:" + std::to_string(this->port), (void *) 1);
        }
    }

    void uWClient::_run(){
        // do actual work here...
        // does this mean the hub is a server or a client?
        // TODO: this should be connecting to ONE server!
        this->h->onConnection([this](uClient ws, uWS::HttpRequest req) {
              std::cout << "Client Connected to Server!" << std::endl;
              printf("%s\n",req.headers->value);
              // seems like theres a new pointer per connected client; need to manage this better.
              pthread_mutex_lock(&this->_lock);
              this->connections.emplace_back(ws);
              this->connected = true;
              pthread_mutex_unlock(&this->_lock);
          }
        );

        // TODO: this needs to try to reconnect:
        this->h->onDisconnection([this](uClient ws, int code, char *message, size_t length) {
            std::cout << "CLIENT CLOSE: " << code << std::endl;
            std::vector<uClient>::iterator it;
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
                    this->_tryReconnect();
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

        this->h->onMessage([this](uClient ws, char *message, size_t length, uWS::OpCode opCode){
            // could match ws to client list if we really wanted to...
            // could push message into local context work queue.
            std::vector<char> msg(message, message+length);
            std::string rmsg(message, length);
            pthread_mutex_lock(&this->_lockRead);
            // fix amount of memory used for buffer... it will drop oldest messages if exceeded.
            if (this->buffer.size()==MAX_DEQUE_LENGTH)
                this->buffer.pop_front();
            this->buffer.emplace_back(msg);
            pthread_mutex_unlock(&this->_lockRead);
        });

        // TODO: loop this so reconnection attempts if not connected to server.
        this->h->connect("ws://127.0.0.1:" + std::to_string(this->port), (void *) 1);
        printf("Starting Client\n");
        this->h->run(); // <- blocking call
        pthread_exit(nullptr);
    };

    void uWClient::stop(){
        pthread_kill(this->_t, 0);
    }

    std::vector<char> uWClient::read(){
        pthread_mutex_lock(&this->_lockRead);
        std::vector<char> ret;
        if (!this->buffer.empty()){
            ret = this->buffer.front();
            this->buffer.pop_front();
        }
        pthread_mutex_unlock(&this->_lockRead);
        return ret;
    }



}