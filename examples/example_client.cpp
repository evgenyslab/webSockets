#include <uWClient.h>
#include <iostream>


int main(){


    std::cout << "CLIENT APPLICATION\n\n";
    uWClient client(13049);
    client.run();
    // wait for connection..
    while (!client.isConnected())
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::cout << "Client connected to server!\n";

    bool exit = false;
    // wait for a moment while uwebsockets starts...
    while(!exit) {
        printf(">> ");
        std::string cmd;
        std::getline(std::cin, cmd);
        if (cmd == "exit")
            exit = true;
        else if (cmd.substr(0,4)=="send"){
            client.sendStringAsText(cmd.substr(4,cmd.size()-1));
        }
        else if (cmd.substr(0,4)=="read"){
            std::string ret = client.readNonBlocking();
            std::cout << "Read from client buffer: " << ret << "\n";
        }
        else if (cmd.substr(0,5)=="bread"){
            std::string ret = client.readBlocking();
            std::cout << "Read from client buffer: " << ret << "\n";
        }
    }

    return 0;
}