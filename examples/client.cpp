#include <uWGroup.h>
#include <iostream>
#include <random>

int main() {


    uWClient::uWClient uC(13049);

    uC.run();

    while(1){
        auto rr = uC.read();
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