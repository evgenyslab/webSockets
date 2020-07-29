/*
 *
 * server
 *
 * thread -> set up + poll in cycle // runs indefinately until quit message
 * thread -> read from queue, write to queue
 *
 * */

#include <uWGroup.h>

#define MAX_MESSAGE_QUEUE 100


using namespace uWS;


typedef std::vector<uWS::WebSocket<uWS::SERVER>* > connectionList;
typedef std::vector<uWS::WebSocket<uWS::SERVER>*>::iterator connnectionListIterator;
typedef std::map<uWS::WebSocket<uWS::SERVER>*, double> connectionPingTimer;

class uWServer : public uWGroup{
private:

    // client lock:
    pthread_mutex_t _lock = PTHREAD_MUTEX_INITIALIZER;
    // connection list:
    connectionList connections = {};
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
//           std::cout << "A client connected" << std::endl;
//           printf("%s\n",req.headers->value);
           // seems like theres a new pointer per connected client; need to manage this better.
           pthread_mutex_lock(&this->_lock);
           this->connections.emplace_back(ws);
           this->connected = true;
           pthread_mutex_unlock(&this->_lock);
                       }
        );

        h.onDisconnection([this](uWS::WebSocket<uWS::SERVER>* ws, int code, char *message, size_t length) {
//            std::cout << "CLIENT CLOSE: " << code << std::endl;
            auto it = findConnection(ws);
//            std::vector<uWS::WebSocket<uWS::SERVER>*>::iterator it;
//            it = find (this->connections.begin(), this->connections.end(), ws);
            if (it != connections.end()){

                pthread_mutex_lock(&this->_lock);
                this->connections.erase(it);
                // set connection state based on how many connected clients there are:
                this->connected = !this->connections.empty();
                pthread_mutex_unlock(&this->_lock);
//                printf("Client removed!\n");
                if (this->connections.empty()){
//                    printf("All Clients disconnected!\n");
                }
            }else{
                // this should be an error...
//                printf("client NOT found in array\n");
            }
        });


        h.onPing([this](uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length) {
//            auto timeOfFlight = (now() - this->pingTimer[ws])/1e6;
//            std::cout << "Client response: " << timeOfFlight << "ms" << std::endl;
        });

        h.onPong([this](uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length) {
            auto timeOfFlight = (now() - this->pingTimer[ws])/1e6;
            std::cout << "Client response: " << timeOfFlight << "ms" << std::endl;
        });


        h.onMessage([this](uWS::WebSocket<uWS::SERVER>* ws, char *message, size_t length, uWS::OpCode opCode){
            // lock queue
            pthread_mutex_lock(&this->_rxmutex);
            // put message into queue
            this->rxqueue.emplace_back(std::string(message,length));
            pthread_mutex_unlock(&this->_rxmutex);
        });

        if (h.listen(this->host.c_str(),this->port)) {
//            printf("Server listening on port: %d\n", this->port);
            // add async to server hub to correctly handle asynchronous sending...
            h.getDefaultGroup<uWS::SERVER>().addAsync();
            h.run();
        }

    }

public:

    uWServer(int port){
        this->port = port;
        this->host = "0.0.0.0";
    };
    ~uWServer() = default;

    void config(){

    };
    // creates and runs a thread with a hub based on config
    void run(){
        pthread_create(&this->_tid, nullptr, this->__run__, this);
    };

    void stop(){
        pthread_kill(this->_tid, 0);
    };

    void sendStringAsBinary(const std::string &msg){
        for (auto c:this->connections)
            c->send(msg.c_str(),msg.size(),OpCode::BINARY);
    }

    void sendStringAsText(const std::string &msg){
        for (auto c:this->connections)
            c->send(msg.c_str(),msg.size(),OpCode::TEXT);
    }

    void pingAllClients(){
        for (auto c:this->connections){
            c->ping("ping-from-server");
            this->pingTimer[c] = now();
        }
    }


};