#include "DropFileServer.hpp"

#include <spdlog/spdlog.h>


int main() {
    spdlog::info("Starting server at port: {}", DropFileServer::DEFAULT_PORT);
    DropFileServer server{DropFileServer::DEFAULT_PORT};
    server.run();
}