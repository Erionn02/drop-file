#include "ClientSession.hpp"

#include <spdlog/spdlog.h>

ClientSession::ClientSession(const std::string &host, unsigned short port) : ClientSession(
        std::make_unique<boost::asio::io_context>(),
        boost::asio::ssl::context{
                boost::asio::ssl::context::sslv23}) {
    context.set_default_verify_paths();
    connect(host, port);
    start();
}

ClientSession::ClientSession(const std::string &host, unsigned short port,
                             const std::string &path_to_cert_authority_file) : ClientSession(
        std::make_unique<boost::asio::io_context>(),
        boost::asio::ssl::context{
                boost::asio::ssl::context::sslv23}) {
    context.load_verify_file(path_to_cert_authority_file);
    connect(host, port);
}

ClientSession::ClientSession(std::unique_ptr<boost::asio::io_context> io_context, boost::asio::ssl::context context)
        : SocketBase({*io_context, context}), io_context(std::move(io_context)), context(std::move(context)) {
    socket_.set_verify_mode(boost::asio::ssl::verify_peer);
    socket_.set_verify_callback([this](bool preverified, boost::asio::ssl::verify_context &ctx) {
        return verify_certificate(preverified, ctx);
    });
}


ClientSession::~ClientSession() {
    io_context->stop();
}

void ClientSession::start() {
    context_thread = std::jthread{[this] {
        io_context->run();
    }};
}

bool ClientSession::verify_certificate(bool preverified, boost::asio::ssl::verify_context &ctx) {
    char subject_name[256];
    X509 *cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
    X509_NAME_oneline(X509_get_subject_name(cert), subject_name, sizeof(subject_name));
    spdlog::debug("Preverified {}, verifying {}", preverified, subject_name);
    return preverified;
}

void ClientSession::send(std::ifstream data_source) {
    std::streamsize bytes_read;
    do {
        bytes_read = data_source.readsome(data_buffer.get(), static_cast<std::streamsize>(BUFFER_SIZE));
        spdlog::debug("Bytes read: {}", bytes_read);
        if (bytes_read > 0) {
            SocketBase::send({data_buffer.get(), static_cast<std::size_t>(bytes_read)});
            SocketBase::receiveACK();
        }
    } while (bytes_read > 0);
}

void ClientSession::receive(std::ofstream &data_sink, std::size_t expected_bytes) {
    std::size_t total_transferred_bytes{0};
    while (total_transferred_bytes < expected_bytes) {
        std::string_view data = SocketBase::receiveToBuffer();
        spdlog::debug("Received {} bytes chunk", data.size());
        std::size_t left_to_transfer = expected_bytes - total_transferred_bytes;
        std::size_t write_size = std::min(left_to_transfer, data.size());
        data_sink.write(data.data(), static_cast<std::streamsize>(write_size));
        total_transferred_bytes += write_size;
        SocketBase::sendACK();
    }
}
