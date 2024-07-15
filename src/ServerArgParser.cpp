#include "ServerArgParser.hpp"

#include <argparse/argparse.hpp>


ServerArgs parseServerArgs(int argc, char **argv) {
    argparse::ArgumentParser program("drop-file-server", "1.0.0");

    program.add_argument("key-cert-dir")
            .nargs(1)
            .required()
            .help("Path to directory that contains certificate (cert.pem) and key (key.pem)");

    program.add_argument("-p", "--port")
            .default_value(ServerArgs::DEFAULT_PORT)
            .scan<'u', unsigned short>()
            .help("Server port");

    program.add_argument("-t", "--timeout")
            .default_value(static_cast<unsigned int>(ServerArgs::DEFAULT_CLIENT_TIMEOUT.count()))
            .scan<'u', unsigned int>()
            .help("Amount of seconds that server keeps the sender's connection alive before returned code is entered on another device.");

    program.add_argument("-w", "--json_words")
            .default_value(static_cast<unsigned int>(ServerArgs::DEFAULT_CLIENT_TIMEOUT.count()))
            .help(R"(Path to the json file of this structure: { "nouns" : [...], "adjectives": [...] })");

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error &err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        throw;
    }

    auto cert_dir = program.get<std::string>("key-cert-dir");
    auto port = program.get<unsigned short>("-p");
    std::chrono::seconds timeout{program.get<unsigned int>("-t")};

    return {.certs_directory = std::move(cert_dir), .port = port, .client_timeout = timeout};
}
