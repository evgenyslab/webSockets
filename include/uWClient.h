//
//

#ifndef WEBSOCKETS_TESTING_UWCLIENT_H
#define WEBSOCKETS_TESTING_UWCLIENT_H

#include <uWS.h>
#include <iostream>
#include <algorithm>
#include <thread>
#include <fstream>
#include <deque>

#define MAX_DEQUE_LENGTH 100

typedef uWS::WebSocket<uWS::CLIENT>* uClient;

using namespace uWS;

namespace uWClient{

    class uWClient {
        int port = 0;
        std::vector<uClient> connections = {};
        std::deque<std::vector<char>> buffer = {}; // inbound messages
        bool connected = false;
        pthread_mutex_t _lock = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_t _lockRead = PTHREAD_MUTEX_INITIALIZER;
        pthread_t _t;
        uWS::Hub *h = nullptr;


        // helper function
        static void *runThread(void *context) {
            ((uWClient *) context)->_run();
            return nullptr;
        }

        void _tryReconnect();


    public:

        explicit uWClient(int _port = 11111);

        ~uWClient(){
            this->stop();
        };

        void run();

        void stop();

        void send(std::string msg);
        void send(std::vector<char> *cArray);

        std::vector<char> read();


    private:

        // run thread definition, class-based:
        void _run();

    };

};


#endif //WEBSOCKETS_TESTING_UWCLIENT_H
