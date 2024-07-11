#include "DropFileServer.hpp"
#include "ServerArgs.hpp"
#include "ServerArgParser.hpp"

#include <spdlog/spdlog.h>


int main(int argc, char *argv[]) {
    spdlog::set_level(spdlog::level::debug);
    ServerArgs args = parseServerArgs(argc, argv);
    spdlog::info("Creating sessions manager...");
    auto sessions_manager = std::make_shared<SessionsManager>(args.client_timeout,
                                                              SessionsManager::DEFAULT_CHECK_INTERVAL);
    spdlog::info("Starting server at port: {}", args.port);
    DropFileServer server{args.port, args.certs_directory, std::move(sessions_manager)};
    server.run();
}