#include "InitSessionMessage.hpp"
#include "ClientSocket.hpp"
#include "DropFileReceiveClient.hpp"
#include "DropFileSendClient.hpp"
#include "ArgParser.hpp"

#include <spdlog/spdlog.h>

#include <iostream>



int main(int argc, char *argv[]) {
    spdlog::set_level(spdlog::level::debug);
    ClientArgs args = parseArgs(argc, argv);

    ClientSocket client_socket("localhost", 12345, "/home/kuba/CLionProjects/drop-file/example_assets/cert.pem");

    if (args.action == Action::send) {
        DropFileSendClient client{std::move(client_socket)};
        client.sendFile(*args.file_to_send_path);
    } else {
        DropFileReceiveClient client{std::move(client_socket), std::cin};
        client.receiveFile(*args.receive_code);
    }

    spdlog::info("Done!");

    return 0;
}
