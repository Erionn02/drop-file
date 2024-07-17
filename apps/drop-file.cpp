#include "InitSessionMessage.hpp"
#include "ClientSocket.hpp"
#include "DropFileReceiveClient.hpp"
#include "DropFileSendClient.hpp"
#include "ClientArgParser.hpp"

#include <spdlog/spdlog.h>

#include <iostream>



int main(int argc, char *argv[]) {
    spdlog::set_level(spdlog::level::err);
    ClientArgs args = parseClientArgs(argc, argv);

    ClientSocket client_socket(args.server_domain_name, args.port, args.verify_cert);

    try {
        if (args.action == Action::send) {
            DropFileSendClient client{std::move(client_socket)};
            auto [fs_entry, receive_code] = client.sendFSEntryMetadata(*args.file_to_send_path);
            std::cout << "Receive code: " << receive_code << std::endl;
            client.sendFSEntry(std::move(fs_entry));
        } else {
            DropFileReceiveClient client{std::move(client_socket), std::cin};
            client.receiveFile(*args.receive_code);
        }
    } catch (const DropFileBaseException& e) {
        std::cerr << e.what();
        exit(1);
    } catch (const boost::wrapexcept<boost::system::system_error> &e) {
        std::cerr<<"Lost connection. Details: " <<e.what();
        exit(2);
    } catch (const std::exception& e) {
        std::cerr<<e.what();
        exit(3);
    }

    spdlog::info("Done!");

    return 0;
}
