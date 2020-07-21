#include <uWServer.h>
#include <thread>
#include <iostream>
#include <random>
#include <msgpack.hpp>


/* Try with new single
 * TODO: need to speed test this somehow
 * */

uint64_t now(){
    return (std::chrono::duration_cast<std::chrono::nanoseconds >(std::chrono::system_clock::now().time_since_epoch())).count();
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

int main(){

    uWServer server(8890);
    server.run();

    msgpack::sbuffer streamBuffer;  // stream buffer
    // create a key-value pair packer linked to stream-buffer
    msgpack::packer<msgpack::sbuffer> packer(&streamBuffer);


    bool exit = false;
    // wait for a moment while uwebsockets starts...
    while(!exit) {
        printf(">> ");
        std::string cmd;
        std::getline(std::cin, cmd);
        if(cmd == "exit")
            exit = true;
        else if(cmd.substr(0,9) == "speedtest"){
            // try to get bytpes after:
            int l;
            if (cmd.size()>9)
                l = atoi(cmd.substr(9,cmd.size()-1).c_str());
            else
                l = 1;
            auto I = randomString(l);
            // create speed test
            streamBuffer.clear();
            packer.pack_map(1);
            packer.pack("loopback");
            packer.pack_bin(I.size());
            packer.pack_bin_body(I.data(), I.size());
            std::vector<double> dts;
            for (int k =0;k<100;k++){
                // time from here:
                auto ta = now();
                std::string msg(streamBuffer.data());
                server.sendStringAsBinary(msg);
//                server._send(streamBuffer.data(), streamBuffer.size()); // streamsize isnt correct yet...
                // now to read...
                std::string ret;
                server.read_blocking(ret);
                // to here...
                dts.emplace_back(now() - ta);
            }
            double sum=0, mu;
            for (auto k : dts){
                sum += k;
            }
            mu = (sum/double(dts.size()))*1e-6;
            printf("Average roundtrip time: %f ms\n",mu);
            int j = 0;
        }
        else if(!cmd.empty()){
            streamBuffer.clear();
            packer.pack_map(1);
            packer.pack("message");
            packer.pack(cmd);
            std::string msg(streamBuffer.data());
            server.sendStringAsBinary(msg);
//            server._send(streamBuffer.data(), streamBuffer.size()); // streamsize isnt correct yet...

        }
    }

    return 0;
}