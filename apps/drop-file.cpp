#include "InitSessionMessage.hpp"
#include "client/ClientSocket.hpp"
#include "client/DropFileReceiveClient.hpp"
#include "client/DropFileSendClient.hpp"
#include "client/ClientArgParser.hpp"

#include <spdlog/spdlog.h>

#include <iostream>


ClientSocket createClientSocket(const ClientArgs& args) {
    return {args.server_domain_name, args.port, args.verify_cert};
}

void runDropFileClient(const ClientArgs &args) {
    if (args.action == Action::send) {
        DropFileSendClient client{createClientSocket(args)};
        auto [fs_entry, receive_code] = client.sendFSEntryMetadata(*args.file_to_send_path);
        std::cout << "Receive code: " << receive_code << std::endl;
        client.sendFSEntry(std::move(fs_entry));
    } else {
        DropFileReceiveClient client{createClientSocket(args), std::cin};
        client.receiveFile(*args.receive_code);
    }
}

void addAdditionalInfo(int code_value) {
    int code_for_certificate_verify_failed = 167772294;
    if (code_value == code_for_certificate_verify_failed) {
        std::cout << "Try adding '-a' flag, to allow self-signed certs." << std::endl;
    }
}

int main(int argc, char *argv[]) {
    spdlog::set_level(spdlog::level::warn);

    try {
        ClientArgs args = parseClientArgs(argc, argv);
        runDropFileClient(args);
    } catch (const ClientArgParserException& e) {
        exit(1);
    } catch (const DropFileBaseException& e) {
        std::cerr << e.what() << std::endl;
        exit(2);
    } catch (const boost::wrapexcept<boost::system::system_error> &e) {
        std::cerr<<e.code().message()<< std::endl;
        addAdditionalInfo(e.code().value());
        exit(3);
    } catch (const std::exception& e) {
        std::cerr<<e.what()<< std::endl;
        exit(4);
    }
}