#include "DropFileServer.hpp"

#include <spdlog/spdlog.h>


int main() {
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Starting server at port: {}", DropFileServer<>::DEFAULT_PORT);
    std::filesystem::path cert_key_dir{CERTS_DIR};
    DropFileServer server{DropFileServer<>::DEFAULT_PORT, cert_key_dir};
    server.run();
}