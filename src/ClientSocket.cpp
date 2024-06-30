#include "ClientSocket.hpp"

#include <spdlog/spdlog.h>

ClientSocket::ClientSocket(const std::string &host, unsigned short port) : ClientSocket(
        std::make_unique<boost::asio::io_context>(),
        boost::asio::ssl::context{
                boost::asio::ssl::context::sslv23}) {
    context.set_default_verify_paths();
    connect(host, port);
    start();
}

ClientSocket::ClientSocket(const std::string &host, unsigned short port,
                           const std::string &path_to_cert_authority_file) : ClientSocket(
        std::make_unique<boost::asio::io_context>(),
        boost::asio::ssl::context{
                boost::asio::ssl::context::sslv23}) {
    context.load_verify_file(path_to_cert_authority_file);
    connect(host, port);
}

ClientSocket::ClientSocket(std::unique_ptr<boost::asio::io_context> io_context, boost::asio::ssl::context context)
        : SocketBase({*io_context, context}), io_context(std::move(io_context)), context(std::move(context)) {
    socket_.set_verify_mode(boost::asio::ssl::verify_peer);
    socket_.set_verify_callback([this](bool preverified, boost::asio::ssl::verify_context &ctx) {
        return verify_certificate(preverified, ctx);
    });
}

void ClientSocket::connect(const std::string &host, unsigned short port) {
    tcp::resolver resolver(socket_.get_executor());
    auto endpoints = resolver.resolve(host, std::to_string(port));
    if (endpoints.empty()) {
        throw SocketException(fmt::format("Did not find {}:{}", host, port));
    }
    socket_.lowest_layer().connect(*endpoints.begin());
    socket_.handshake(boost::asio::ssl::stream_base::client);
}


ClientSocket::~ClientSocket() {
    io_context->stop();
}

void ClientSocket::start() {
    context_thread = std::jthread{[this] {
        io_context->run();
    }};
}

bool ClientSocket::verify_certificate(bool preverified, boost::asio::ssl::verify_context &ctx) {
    char subject_name[256];
    X509 *cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
    X509_NAME_oneline(X509_get_subject_name(cert), subject_name, sizeof(subject_name));
    spdlog::debug("Preverified {}, verifying {}", preverified, subject_name);
    return preverified;
}
