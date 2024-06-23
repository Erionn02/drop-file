#include "DropFileServer.hpp"

#include <spdlog/spdlog.h>


int main() {
    spdlog::info("Starting server at port: {}", DropFileServer::DEFAULT_PORT);
    std::filesystem::path cert_key_dir{"/home/kuba/CLionProjects/drop-file/example_assets"};
    DropFileServer server{DropFileServer::DEFAULT_PORT, cert_key_dir};
    server.run();
}