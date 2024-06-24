#include "InitSessionMessage.hpp"
#include "ClientSession.hpp"

#include <spdlog/spdlog.h>

#include <iostream>


int main(int argc, char *argv[]) {
    (void) argv;
    std::string parsed_test_file_path{"/home/kuba/send_test_file.txt"};
    nlohmann::json message_json;
    if (argc == 1) {
        message_json = InitSessionMessage::createReceiveMessage("some code");
    } else {
        message_json = InitSessionMessage::createSendMessage(parsed_test_file_path);
    }

    ClientSession client("localhost", 12345, "/home/kuba/CLionProjects/drop-file/example_assets/cert.pem");
    client.start();

    client.send(message_json.dump());

    auto received = client.receive();
    spdlog::info("Server response: {}", received);

    return 0;
}
