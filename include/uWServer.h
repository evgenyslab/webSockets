//
//

#ifndef WEBSOCKETS_TESTING_UWSERVER_H
#define WEBSOCKETS_TESTING_UWSERVER_H

#include <uWS.h>
#include <iostream>
#include <algorithm>
#include <thread>
#include <fstream>
#include <deque>

#define MAX_DEQUE_LENGTH 100

typedef uWS::WebSocket<uWS::SERVER>* uServer;

using namespace uWS;

namespace uWServer{

    class uWServer {
        int port = 0;
        std::vector<uServer> connections = {};
        std::deque<std::string> buffer = {}; // inbound messages
        bool connected = false;
        pthread_mutex_t _lock = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_t _lockRead = PTHREAD_MUTEX_INITIALIZER;
        pthread_t _t;
        uWS::Hub *h = nullptr;


        // helper function
        static void *runThread(void *context) {
            ((uWServer *) context)->_run();
            return nullptr;
        }


    public:

        explicit uWServer(int _port = 11111);

        ~uWServer(){
            this->stop();
        };

        void run();

        void stop();

        void send(std::string msg);
        void send(std::vector<char> *cArray);
        void sendRaw(std::vector<char> *cArray);
        void sendRaw(char *cArray, int len);
        void sendRaw(const std::string &cArray);

        std::string read();


    private:

        // run thread definition, class-based:
        void _run();

    };

};


#endif //WEBSOCKETS_TESTING_UWSERVER_H
