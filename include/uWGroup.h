#ifndef UWGROUP_H
#define UWGROUP_H

#include <uWS.h>
#include <thread>
#include <deque>
#include <iostream>
#include <syslog.h>

#define MAX_MESSAGE_QUEUE 20

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
    bool started = false;
    int maxMessageQueue = MAX_MESSAGE_QUEUE;

    /**
     *
     * @return double epoch time in ns
     */
    uint64_t now(){
        return (std::chrono::duration_cast<std::chrono::nanoseconds >(std::chrono::system_clock::now().time_since_epoch())).count();
    }

public:

    /**
     * Checked whether connection is established
     * @return bool
     */
    bool isConnected(){
        return this->connected;
    }

    /**
     * Check whether uWS::Hub listen has been called.
     * @return
     */
    bool isStarted(){
        return this->started;
    }

    /**
     * Waits until hub is started
     * this is used to ensure uWS::Hub thread allows creation & allocation before any
     * operation on IO object.
     */
    void waitForStart(){
        while(!this->isStarted())
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    /**
     * Waits until connection is established
     */
    void waitForConnection(){
        while(!this->isConnected())
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    /**
     * Interface to add message to internal read queue
     * @param message
     * @param length
     */
    void addMessageToQueue(char *message, size_t length){
        pthread_mutex_lock(&this->_rxmutex);
        if(this->rxqueue.size() == this->maxMessageQueue)
            this->rxqueue.pop_front();
        // put message into queue
        this->rxqueue.emplace_back(std::string(message,length));
        pthread_mutex_unlock(&this->_rxmutex);
    }


    /**
     * Checks whether messages exist in queue
     * @return bool
     */
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
     * Destructively read the queue
     * @return
     */
    std::string readLastNonBlocking(){
        if (!this->isConnected())
            return "";
        std::string ret;
        // lock queue
        pthread_mutex_lock(&this->_rxmutex);
        if (this->rxqueue.empty()){
            pthread_mutex_unlock(&this->_rxmutex);
            return "";
        }
        while(this->rxqueue.size()>1)
            this->rxqueue.pop_front();
        ret = this->rxqueue.front();
        this->rxqueue.pop_front();
        pthread_mutex_unlock(&this->_rxmutex);
        return ret;
    }

    /**
     * Interface for configuration
     */
    virtual void config() = 0;

    /**
     * Interface for running IO object
     */
    virtual void run() = 0 ;

    /**
     * Interface for stopping IO object
     */
    virtual void stop() = 0;

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

#endif /* UWGROUP */