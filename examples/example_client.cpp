#include <uWClient_b.h>
#include <thread>
#include <iostream>
#include <random>

/* Try with new single
 * TODO: need to speed test this somehow
 * */

std::string randomString(std::size_t length)
{
    std::random_device random_device;
    std::mt19937 generator(random_device());
    std::uniform_int_distribution<int> distribution(0,255);

    std::string random_string;

    for (std::size_t i = 0; i < length; ++i)
    {
        random_string += (char) distribution(generator);

    }

    return random_string;
}

int main(){

    uWClient_b client(13049);
    client.run();

    bool exit = false;
    // wait for a moment while uwebsockets starts...
    while(!exit) {
        // read from client
        std::string msg;
        client.read_blocking(msg);
        printf("Message received on server, size: %ld bytes\n", msg.size());
        if (msg.size()<60)
            printf("Content: %s\n", msg.c_str());
    }

    return 0;
}