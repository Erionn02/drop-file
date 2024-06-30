#include "InitSessionMessage.hpp"
#include "ClientSession.hpp"
#include "ArgParser.hpp"

#include <spdlog/spdlog.h>

#include <iostream>
#include <fstream>



int main(int argc, char *argv[]) {
    spdlog::set_level(spdlog::level::debug);
    ClientArgs args = parse(argc, argv);

    ClientSession client("localhost", 12345, "/home/kuba/CLionProjects/drop-file/example_assets/cert.pem");

    spdlog::debug("Client started.");
    if (args.action == Action::send) {
        nlohmann::json message_json = InitSessionMessage::createSendMessage(*args.file_to_send_path);
        client.SocketBase::send(message_json.dump());
        spdlog::info("Message sent.");
        std::string received_msg = client.SocketBase::receive();
        spdlog::info("Server response: {}", received_msg);
        spdlog::info("Waiting for client to confirm");
        client.SocketBase::receiveACK();
        std::ifstream file{*args.file_to_send_path};
        client.send(std::move(file));
    } else {
        nlohmann::json message_json = InitSessionMessage::createReceiveMessage(*args.receive_code);
        client.SocketBase::send(message_json.dump());
        spdlog::info("Message sent.");
        auto received = client.SocketBase::receive();
        spdlog::info("Server response: {}", received);

        spdlog::info("Do you want to proceed? [y/n]");
        char confirmation{};
        std::cin >> confirmation;
        if (confirmation != 'y') {
            spdlog::info("Entered {} aborting.", confirmation);
            client.SocketBase::send("abort");
            return 1;
        }
        spdlog::info("Sending confirmation...");
        client.SocketBase::sendACK();
        auto json = nlohmann::json::parse(received);
        std::string filename = json[InitSessionMessage::FILENAME_KEY].get<std::string>();

        std::size_t expected_file_size = json[InitSessionMessage::FILE_SIZE_KEY].get<std::size_t>();
        std::ofstream received_file{filename, std::ios::trunc};
        spdlog::info("Receiving file...");
        client.receive(received_file, expected_file_size);
    }

    spdlog::info("Done!");

    return 0;
}
