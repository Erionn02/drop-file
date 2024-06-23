#include "ClientSession.hpp"

#include <spdlog/spdlog.h>

ClientSession::ClientSession(const std::string &host, unsigned short port) : ClientSession() {
    context.set_default_verify_paths();
    connect(host, port);
}

ClientSession::ClientSession(const std::string &host, unsigned short port,
                             const std::string &path_to_cert_authority_file) : ClientSession() {
    context.load_verify_file(path_to_cert_authority_file);
    connect(host, port);
}

ClientSession::ClientSession() : SocketBase({io_context, context}) {
    socket_.set_verify_mode(boost::asio::ssl::verify_peer);
    socket_.set_verify_callback([this](bool preverified, boost::asio::ssl::verify_context &ctx) {
        return verify_certificate(preverified, ctx);
    });
}

ClientSession::~ClientSession() {
    io_context.stop();
}

void ClientSession::start() {
    context_thread = std::jthread{[this] {
        io_context.run();
    }};
}

bool ClientSession::verify_certificate(bool preverified, boost::asio::ssl::verify_context &ctx) {
    char subject_name[256];
    X509 *cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
    X509_NAME_oneline(X509_get_subject_name(cert), subject_name, sizeof(subject_name));
    spdlog::debug("Preverified {}, verifying {}", preverified, subject_name);
    return preverified;
}

void ClientSession::connect(const std::string &host, unsigned short port) {
    tcp::resolver resolver(io_context);
    auto endpoints = resolver.resolve(host, std::to_string(port));
    if (endpoints.empty()) {
        throw SocketException(fmt::format("Did not find {}:{}", host, port));
    }
    assert(!endpoints.empty());
    socket_.lowest_layer().connect(*endpoints.begin());
    socket_.handshake(boost::asio::ssl::stream_base::client);
}
