#include "client/ClientArgParser.hpp"

#include <argparse/argparse.hpp>
#include <fmt/format.h>

void removeTrailingSlashes(std::string &file_or_code);

ClientArgs parseClientArgs(int argc, char **argv) {
    argparse::ArgumentParser program("drop-file", "1.0.0");

    program.add_argument("action")
            .nargs(1)
            .required()
            .help("Type of operation: send or receive")
            .action([](const std::string &value) {
                if (value != "send" && value != "receive") {
                    throw std::runtime_error(
                            fmt::format("Incorrect value of action argument: {}. Expected send or receive.", value));
                }
                return value;
            });

    program.add_argument("file_or_code")
            .nargs(1)
            .required()
            .help("Either path to file to send or code words to receive file.");

    program.add_argument("-p", "--port")
            .default_value(std::uint16_t{8088})
            .scan<'u', unsigned short>()
            .help("Server port");

    program.add_argument("-d", "--domain_name")
            .default_value(ClientArgs::DEFAULT_SERVER_DOMAIN)
            .help("Server domain name or ip address for self-hosted");

    program.add_argument("-a","--allow_self_signed_cert")
            .default_value(false)
            .implicit_value(true)
            .help("Boolean arg specifying whether client should verify server's cert. "
                  "Set to false to allow self signed certs. Only for self-hosted. Use with caution.");


    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error &err) {
        std::cerr << err.what() << '\n';
        std::cerr << program;
        throw ClientArgParserException(err.what());
    }

    auto action = program.get<std::string>("action");
    auto file_or_code = program.get<std::string>("file_or_code");



    if (action == "send") {
        removeTrailingSlashes(file_or_code);
        return {.action = Action::send,
                .file_to_send_path = file_or_code,
                .port = program.get<unsigned short>("-p"),
                .server_domain_name = program.get<std::string>("-d"),
                .verify_cert = !program.get<bool>("-a")};
    } else {
        return {.action = Action::receive,
                .receive_code = file_or_code,
                .port = program.get<unsigned short>("-p"),
                .server_domain_name = program.get<std::string>("-d"),
                .verify_cert = !program.get<bool>("-a")};
    }
}

void removeTrailingSlashes(std::string &file_or_code) {
    auto pos = file_or_code.find_last_not_of('/');
    if (pos != std::string::npos) {
        file_or_code = file_or_code.substr(0, pos + 1);
    } else {
        file_or_code = file_or_code.substr(0, std::min(file_or_code.size(), 1ul));
    }
}
