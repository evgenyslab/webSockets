#ifndef UWCLIENT_H
#define UWCLIENT_H

#include <uWGroup.h>

using namespace uWS;

class uWClient : public uWGroup{
private:
    // run thread
    pthread_t  _tih;
    std::string connectionInfo;
    double pingTimeSent = 0;

    bool exit = false;
    uWS::WebSocket<uWS::CLIENT>* client; /**< Client object*/

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
        try {
            // create hub object local to thread:
            uWS::Hub h;
            h.onConnection([this](uWS::WebSocket<uWS::CLIENT>* ws, uWS::HttpRequest req) {
                               syslog(LOG_INFO, "uWClient: Connected to server");
                               this->connected = true;
                               this->client = ws;
                           }
            );

            h.onPong([this](uWS::WebSocket<uWS::CLIENT> *ws, char *message, size_t length) {
                auto timeOfFlight = (this->now() - this->pingTimeSent)/1e6;
                syslog(LOG_INFO, "uWClient: server ping response in %f ms", timeOfFlight);
            });

            h.onDisconnection([this](uWS::WebSocket<uWS::CLIENT>* ws, int code, char *message, size_t length) {
                syslog(LOG_WARNING, "uWClient: Disconnected from server");
                this->connected = false;
            });


            h.onMessage([this](uWS::WebSocket<uWS::CLIENT>* ws, char *message, size_t length, uWS::OpCode opCode){
                this->addMessageToQueue(message, length);
            });

            this->started = true;
            h.connect( this->connectionInfo, (void *) 1); // does not block if no server present
            h.run(); // <- blocking call
        } catch (const std::exception& e) {
            syslog(LOG_ERR, "uWClient: HUB ERROR");
        }
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
                pthread_kill(this->_tid, 0);
                this->started = false;
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

    ~uWClient(){
        if (this->started)
            this->stop();
    };

    void config() override{

    };

    // creates and runs a thread with a hub based on config
    void run() override{
        pthread_create(&this->_tid, nullptr, this->__run__, this);
        // add run monitor...
        pthread_create(&this->_tih, nullptr, this->__monitor__, this);
        this->waitForStart();
    };

    void stop() override{
        pthread_kill(this->_tid, 0);
        pthread_kill(this->_tih,0);
        this->started = false;
    };

    void ping() override{
        if(this->isConnected()){
            this->pingTimeSent = now();
            this->client->ping("ping-from-client");
        }
    }


    // send message
    void sendStringAsBinary(const std::string &msg) override{
        if(this->isConnected())
            this->client->send(msg.c_str(),msg.size(),OpCode::BINARY);
    }

    void sendStringAsText(const std::string &msg) override{
        if(this->isConnected())
            this->client->send(msg.c_str(),msg.size(),OpCode::TEXT);
    }

};

#endif /* UWCLIENT_H */