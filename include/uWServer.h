#ifndef UWSERVER_H
#define UWSERVER_H

#include <uWGroup.h>


using namespace uWS;


typedef std::vector<uWS::WebSocket<uWS::SERVER>* > connectionList;
typedef std::vector<uWS::WebSocket<uWS::SERVER>*>::iterator connnectionListIterator;
typedef std::map<uWS::WebSocket<uWS::SERVER>*, double> connectionPingTimer;

class uWServer : public uWGroup{
private:

    pthread_mutex_t clientLock = PTHREAD_MUTEX_INITIALIZER; /**< client lock*/
    connectionList connections = {}; /**< list of connected clients*/
    connectionPingTimer pingTimer = {};




    // functions:
    // helper function
    static void *__run__(void *context) {
        ((uWServer *) context)->_run();
        return nullptr;
    }

    connnectionListIterator findConnection(uWS::WebSocket<uWS::SERVER>* ws){
        return find (this->connections.begin(), this->connections.end(), ws);
    }

    // run function:
    void _run(){
        // create hub object local to thread:
        uWS::Hub h;

        h.onConnection([this](uWS::WebSocket<uWS::SERVER>* ws, uWS::HttpRequest req) {
            syslog(LOG_INFO, "uWServer: New client connected");
            pthread_mutex_lock(&this->clientLock);
            this->connections.emplace_back(ws);
            this->connected = true;
            pthread_mutex_unlock(&this->clientLock);
            }
        );

        h.onDisconnection([this](uWS::WebSocket<uWS::SERVER>* ws, int code, char *message, size_t length) {
            syslog(LOG_INFO, "uWServer: Client disconnected");
            auto it = findConnection(ws);
            if (it != connections.end()){
                pthread_mutex_lock(&this->clientLock);
                this->connections.erase(it);
                // set connection state based on how many connected clients there are:
                this->connected = !this->connections.empty();
                pthread_mutex_unlock(&this->clientLock);
                syslog(LOG_INFO, "uWServer: Disconnected client removed");
                if (this->connections.empty())
                    syslog(LOG_INFO, "uWServer: All clients disconnected");
            }else
                syslog(LOG_WARNING, "uWServer: Disconnected client not found in client list");
        });


        h.onPong([this](uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length) {
            auto timeOfFlight = (now() - this->pingTimer[ws])/1e6;
            syslog(LOG_INFO, "uWServer: client ping response in %f ms", timeOfFlight);
        });


        h.onMessage([this](uWS::WebSocket<uWS::SERVER>* ws, char *message, size_t length, uWS::OpCode opCode){
            // lock queue
            this->addMessageToQueue(message, length);
        });

        if (h.listen(this->host.c_str(),this->port)) {
            syslog(LOG_INFO, "uWServer: Starting server on %s:%d", this->host.c_str(), this->port);
            // add async to server hub to correctly handle asynchronous sending...
            this->started = true;
            h.getDefaultGroup<uWS::SERVER>().addAsync();
            h.run();
        }

    }

public:

    uWServer(int port, int messageQueueLength=MAX_MESSAGE_QUEUE){
        this->port = port;
        this->host = "0.0.0.0";
        this->maxMessageQueue = messageQueueLength;
    };

    ~uWServer() {
        if (this->started)
            this->stop();
    };

    void config() override{

    };
    // creates and runs a thread with a hub based on config
    void run() override{
        pthread_create(&this->_tid, nullptr, this->__run__, this);
        this->waitForStart();
    };

    void stop() override{
        pthread_kill(this->_tid, 0);
        this->started = false;
    };

    void sendStringAsBinary(const std::string &msg) override{
        for (auto c:this->connections)
            c->send(msg.c_str(),msg.size(),OpCode::BINARY);
    }

    void sendStringAsText(const std::string &msg) override{
        for (auto c:this->connections)
            c->send(msg.c_str(),msg.size(),OpCode::TEXT);
    }

    void ping() override{
        for (auto c:this->connections){
            c->ping("ping-from-server");
            this->pingTimer[c] = now();
        }
    }


};

#endif /* UWSERVER_H */