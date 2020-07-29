#include "uWClient.h"
#include "uWServer.h"
#include <gtest/gtest.h>



TEST(BasicMessageSend, sendReceiveTest){
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

TEST(BasicMessageSend, clientNoServerPing){
    uWClient client(8888);
    client.run();

    client.pingServer();

}

TEST(BasicMessageSend, serverNoServerPing){
uWServer server(8888);
server.run();

server.pingAllClients();

}



int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}