#include <uWServer.h>
#include <iostream>


int main(){

    std::cout << "SERVER APPLICATION\n\n";
    uWServer server(13049, 5);
    server.run();
    // wait for connection..
    while (!server.isConnected())
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    bool exit = false;
    // wait for a moment while uwebsockets starts...
    while(!exit) {
        printf(">> ");
        std::string cmd;
        std::getline(std::cin, cmd);
        if (cmd == "exit")
            exit = true;
        else if (cmd.substr(0,4)=="send"){
            server.sendStringAsText(cmd.substr(4,cmd.size()-1));
        }
        else if (cmd.substr(0,4)=="read"){
            std::string ret = server.readNonBlocking();
            std::cout << "Read from client buffer: " << ret << "\n";
        }
        else if (cmd.substr(0,5)=="bread"){
            std::string ret = server.readBlocking();
            std::cout << "Read from client buffer: " << ret << "\n";
        }
        else if (cmd.substr(0,5)=="dread"){
            std::string ret = server.readLastNonBlocking();
            std::cout << "Read Last from client buffer: " << ret << "\n";
        }
        else if (cmd.substr(0,4)=="ping"){
            server.ping();
        }
    }

    return 0;
}