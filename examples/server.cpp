//
//

#include <uWGroup.h>
#include <iostream>
#include <random>
//

std::vector<char> readFile(const char* filename)
{
    // open the file:
    std::ifstream file(filename, std::ios::binary);

    // Stop eating new lines in binary mode!!!
    file.unsetf(std::ios::skipws);

    // get its size:
    std::streampos fileSize;

    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // reserve capacity
    std::vector<char> vec;
    vec.reserve(fileSize);

    // read the data:
    vec.insert(vec.begin(),
               std::istream_iterator<char>(file),
               std::istream_iterator<char>());

    return vec;
}

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

void * client(void * ctx){
    uWClient::uWClient * uC = (uWClient::uWClient*) ctx;

    while(1){
        auto rr = uC->read();
        if (!rr.empty()){
            if (rr.size()>100)
                printf("Client read: %ld bytes\n>> ", rr.size());
            else{
                std::string ret(rr.begin(), rr.end());
                printf("Client read: %ld bytes: %s\n>> ", rr.size(), ret.c_str());
            }


        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));

    }
}

int main() {

    uWServer::uWServer uS(13049);

    uS.run();

    // initialize exit condition:
    bool exit = false;
    // wait for a moment while uwebsockets starts...
    std::this_thread::sleep_for(std::chrono::seconds(2));
    while(!exit){
        printf(">> ");
        std::string cmd;
        std::getline (std::cin,cmd);
        if (cmd == "exit"){
            exit = true;
            continue;
        }else if (cmd == "read") {
            // try to read from uS buffer
            auto r = uS.read();
            printf("received:\n%s\n", r.c_str());
        }else if (cmd =="jload") {
            // can't use json with images in any way, UTF-8 decoding breaks it.
            // TODO: look at using msgpack; browers should support
            auto I = readFile("/Users/en/Git/Sockets/uWebSockets/test.jpeg");
            uS.send(&I);
        }else if (cmd =="pload") {
            // can't use json with images in any way, UTF-8 decoding breaks it.
            // TODO: look at using msgpack; browers should support
            auto I = readFile("/Users/en/Git/Sockets/uWebSockets/pngc.png");
            uS.send(&I);
        }else if (cmd =="plarge") {
            // can't use json with images in any way, UTF-8 decoding breaks it.
            // TODO: look at using msgpack; browers should support
            auto I = readFile("/Users/en/Git/Sockets/uWebSockets/large_png.png");
            uS.send(&I);
        }else if (cmd.substr(0,4) =="rand") {
            // get length of random:
            int l;
            if (cmd.size()>4)
                l = atoi(cmd.substr(4,cmd.size()-1).c_str());
            else
                l = 260000;
            // can't use json with images in any way, UTF-8 decoding breaks it.
            // TODO: I THINK THERE IS A BYTE SEND LIMIT! CHROME ISSUE!!!!
            auto I = randomString(l);
            uS.sendRaw(I); // TODO FOR SOME REASON THIS ONLY WORKS AFTER A JLOAD;
            // TODO: I think uws adjusts sizes on the go
            // TODO: seems like on connection to uWS, initial limit is between 250-260kB, why? what about Safari?
            // Chrome seems to be ok with 260kB but not more...

        }else if (!cmd.empty()){
                // send message to connected client (browser)
                nlohmann::json j = {{"message", cmd}};
                uS.send(j);
        }
    }
}
