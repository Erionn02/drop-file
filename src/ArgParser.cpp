#include "ArgParser.hpp"

#include <argparse/argparse.hpp>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

ClientArgs parseArgs(int argc, char **argv) {
    argparse::ArgumentParser program("drop-file", "1.0.0");

    program.add_argument("action")
            .nargs(1)
            .required()
            .help("Type of operation: send or receive")
            .action([](const std::string& value) {
                if(value != "send" && value != "receive") {
                    throw std::runtime_error(fmt::format("Incorrect value of action argument: {}. Expected send or receive.", value));
                }
                return value;
            });

    program.add_argument("file_or_code")
            .nargs(1)
            .required()
            .help("Either path to file to send or code words to receive file.");


    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        exit(1);
    }

    auto action = program.get<std::string>("action");
    auto file_or_code = program.get<std::string>("file_or_code");

    if (action == "send") {
        return {.action = Action::send,
                .file_to_send_path = file_or_code};
    } else {
        return {.action = Action::receive,
                .receive_code = file_or_code};
    }
}
