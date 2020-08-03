#ifndef UWCLIENT_H
#define UWCLIENT_H

#include <uWGroup.h>

using namespace uWS;

class uWClient : public uWGroup{
private:
    // run thread
    pthread_t  _tih;
    std::string connectionInfo;
    //
    double pingTravelTime = 0;

    bool exit = false;
    // client
    uWS::WebSocket<uWS::CLIENT>* client;

    // functions:
    // helper function
    static void *__run__(void *context) {
        ((uWClient *) context)->_run();
        return nullptr;
    }

    // hall monitor function
    static void *__monitor__(void *context) {
        ((uWClient *) context)->_hallMonitor();
        return nullptr;
    }
    // run function:
    void _run(){
        // create hub object local to thread:
        uWS::Hub h;
        // TODO: need to only allow one server at a time; can't handle this right now, seems to have
        // problems when server connects

        h.onConnection([this](uWS::WebSocket<uWS::CLIENT>* ws, uWS::HttpRequest req) {
//           std::cout << "Client connected to Server on port: " << this->port << std::endl;
           // seems like theres a new pointer per connected client; need to manage this better.
//            printf("%s\n",req.headers->value);
           this->connected = true;
           this->client = ws;
           }
        );

        h.onPong([this](uWS::WebSocket<uWS::CLIENT> *ws, char *message, size_t length) {
            this->pingTravelTime = this->now() - this->pingTravelTime;
            std::cout << "Ping time: " << this->pingTravelTime/1e6 << "ms" << std::endl;
        });

        h.onDisconnection([this](uWS::WebSocket<uWS::CLIENT>* ws, int code, char *message, size_t length) {
//            std::cout << "CLIENT Disconnected from Server with code: " << code << std::endl;
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
        h.connect( this->connectionInfo, (void *) 1);
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

    uWClient(int port){
        this->port = port;
        this->host = "127.0.0.1:";
        this->connectionInfo = "ws://" + this->host + std::to_string(this->port);
    };

    ~uWClient() = default;

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
        pthread_kill(this->_tih,0);
    };

    void ping(){
        if(this->isConnected()){
            this->pingTravelTime = now();
            this->client->ping("ping-from-client");
        }
    }


    // send message
    void sendStringAsBinary(const std::string &msg){
        if(this->isConnected())
            this->client->send(msg.c_str(),msg.size(),OpCode::BINARY);
    }

    void sendStringAsText(const std::string &msg){
        if(this->isConnected())
            this->client->send(msg.c_str(),msg.size(),OpCode::TEXT);
    }

};

#endif /* UWCLIENT_H */