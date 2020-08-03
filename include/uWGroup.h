
#pragma once
#include <uWS.h>
#include <thread>
#include <deque>
#include <iostream>

using namespace uWS;

/**
 * uWGroup Interface class for Client and Server webSocket classes.
 * */
class uWGroup{

protected:
    pthread_t _tid; /**< thread allocator for running uWS hub */
    std::deque<std::string> rxqueue; /**< message receive queue */
    pthread_mutex_t _rxmutex = PTHREAD_MUTEX_INITIALIZER; /**< message receive queue mutex */

    int port = 0; /**< input port */
    std::string host; /**< host address */
    bool connected = false;

    /**
     *
     * @return double epoch time in ns
     */
    uint64_t now(){
        return (std::chrono::duration_cast<std::chrono::nanoseconds >(std::chrono::system_clock::now().time_since_epoch())).count();
    }

public:

    ///
    /// \return
    bool isConnected(){
        return this->connected;
    }

    /**
     *
     */
    void waitForConnection(){
        while(!this->isConnected())
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    bool hasMessages(){
        return !this->rxqueue.empty();
    }

    /**
     *
     * @return
     */
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

    /**
     *
     * @return
     */
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

    /**
     * Interface for sending a string object as binary array.
     * @param msg
     */
    virtual void sendStringAsBinary(const std::string &msg) = 0;

    /**
     * Interface for sending a string object as UTF-8 encoded array.
     * @param msg
     */
    virtual void sendStringAsText(const std::string &msg) = 0;

    /**
     * Interface for running ping on interface.
     */
    virtual void ping() = 0;

};