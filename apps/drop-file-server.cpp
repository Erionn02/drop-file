#include "server/DropFileServer.hpp"
#include "server/ServerArgs.hpp"
#include "server/ServerArgParser.hpp"

#include <spdlog/spdlog.h>


int main(int argc, char *argv[]) {
    spdlog::set_level(spdlog::level::debug);
    ServerArgs args = parseServerArgs(argc, argv);
    spdlog::info("Creating sessions manager...");
    auto sessions_manager = std::make_shared<SessionsManager>(args.client_timeout,
                                                              SessionsManager::DEFAULT_CHECK_INTERVAL);
    spdlog::info("Starting server at port: {} with {} certs dir.", args.port, args.certs_directory);
    DropFileServer server{args.port, args.certs_directory, std::move(sessions_manager)};
    server.run();
}