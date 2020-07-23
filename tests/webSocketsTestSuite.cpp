#include "uWClient.h"
#include "uWServer.h"
#include <gtest/gtest.h>


TEST(BasicMessageSend, testName){
    uWServer server(8888);
    server.run();

    uWClient client(8888);
    client.run();

    std::string r = "sending message";

    server.sendStringAsText(r);

    while (!server.isConnected() && !client.isConnected())
        std::this_thread::sleep_for(std::chrono::milliseconds(100));


    std::string ret = client.readBlocking();

    EXPECT_EQ(r,ret);

}

TEST(ClientReadNonBlocking, testName){
    uWServer server(8888);
    server.run();

    uWClient client(8888);
    client.run();


    while (!server.isConnected() && !client.isConnected())
        std::this_thread::sleep_for(std::chrono::milliseconds(100));


    std::string ret = client.readNonBlocking();

    EXPECT_EQ(ret, "");

}


int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}