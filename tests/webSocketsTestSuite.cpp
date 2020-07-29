#include "uWClient.h"
#include "uWServer.h"
#include <gtest/gtest.h>



TEST(BasicMessageSend, testSendAsText){
    uWServer server(8888);
    server.run();

    uWClient client(8888);
    client.run();

    std::string r = "sending message";

    server.waitForConnection();
    client.waitForConnection();

    server.sendStringAsText(r);

    std::string ret = client.readBlocking();

    EXPECT_EQ(r,ret);
}

TEST(BasicMessageSend, testSendAsBinary){
    uWServer server(8888);
    server.run();

    uWClient client(8888);
    client.run();

    std::string r = "sending message";

    server.waitForConnection();
    client.waitForConnection();

    server.sendStringAsBinary(r);

    std::string ret = client.readBlocking();

    EXPECT_EQ(r,ret);
}

TEST(PingTests, testPingPong){
    uWServer server(8888);
    server.run();

    uWClient client(8888);
    client.run();

    server.waitForConnection();
    client.waitForConnection();

    server.ping();
    client.ping();
}

TEST(PingTests, testMultiPingPong){
    uWServer server(8888);
    server.run();

    uWClient client(8888);
    client.run();

    uWClient client2(8888);
    client2.run();

    server.waitForConnection();
    client.waitForConnection();
    client2.waitForConnection();

    server.ping();
    client.ping();
    client2.ping();

}


TEST(PingTests, clientNoServerPing){
    uWClient client(8888);
    client.run();

    client.ping();

}

TEST(PingTests, serverNoServerPing){
    uWServer server(8888);
    server.run();

    server.ping();

}



int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}