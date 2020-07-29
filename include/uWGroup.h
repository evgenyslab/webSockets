/* uWGroup Interface for Server and Client
 *
 *
 * */

#pragma once
#include <uWS.h>
#include <thread>
#include <deque>
#include <iostream>

using namespace uWS;

class uWGroup{

protected:
    // run thread
    pthread_t _tid;
    // received queue
    std::deque<std::string> rxqueue;
    pthread_mutex_t _rxmutex = PTHREAD_MUTEX_INITIALIZER;

    // port
    int port = 0;
    // host:
    std::string host;
    //
    bool connected = false;

    uint64_t now(){
        return (std::chrono::duration_cast<std::chrono::nanoseconds >(std::chrono::system_clock::now().time_since_epoch())).count();
    }

public:

    bool isConnected(){
        return this->connected;
    }

    void waitForConnection(){
        while(!this->isConnected())
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    bool hasMessages(){
        return !this->rxqueue.empty();
    }

    std::string readBlocking(){
        if (!this->isConnected())
            return "";
        bool received = false;
        std::string ret;
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
        return ret;
    }

    std::string readNonBlocking(){
        if (!this->isConnected())
            return "";
        std::string ret;
        // lock queue
        pthread_mutex_lock(&this->_rxmutex);
        if(!this->rxqueue.empty()){
            ret = this->rxqueue.front();
            this->rxqueue.pop_front();
        }
        pthread_mutex_unlock(&this->_rxmutex);
        return ret;
    }

    virtual void sendStringAsBinary(const std::string &msg) = 0;
    virtual void sendStringAsText(const std::string &msg) = 0;

    virtual void ping() = 0;

};