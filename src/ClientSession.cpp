#include "ClientSession.hpp"

#include <spdlog/spdlog.h>

ClientSession::ClientSession(const std::string &host, unsigned short port) : ClientSession(
        std::make_unique<boost::asio::io_context>(),
        boost::asio::ssl::context{
                boost::asio::ssl::context::sslv23}) {
    context.set_default_verify_paths();
    connect(host, port);
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

void ClientSession::connect(const std::string &host, unsigned short port) {
    tcp::resolver resolver(*io_context);
    auto endpoints = resolver.resolve(host, std::to_string(port));
    if (endpoints.empty()) {
        throw SocketException(fmt::format("Did not find {}:{}", host, port));
    }
    socket_.lowest_layer().connect(*endpoints.begin());
    socket_.handshake(boost::asio::ssl::stream_base::client);
}

void ClientSession::send(std::ifstream data_source) {
    std::streamsize bytes_read;
    do {
        bytes_read = data_source.readsome(data_buffer.get(), static_cast<std::streamsize>(MAX_MSG_SIZE));
        if (bytes_read > 0) {
            SocketBase::send(std::string_view{data_buffer.get(), static_cast<std::size_t>(bytes_read)});
            auto msg = SocketBase::receive();
            if (msg != "ok") {
                spdlog::error("Error from server: {}", msg);
                break;
            }
        }
    } while (bytes_read != -1);
}

void ClientSession::receive(std::ofstream &data_sink, std::size_t expected_bytes) {
    std::size_t total_transferred_bytes{0};
    while (total_transferred_bytes < expected_bytes) {
        std::string data = SocketBase::receive();
        std::size_t left_to_transfer = expected_bytes - total_transferred_bytes;
        std::size_t write_size = std::min(left_to_transfer, data.size());
        data_sink.write(data.data(), static_cast<std::streamsize>(write_size));
        total_transferred_bytes += write_size;
        SocketBase::send(std::string_view{"ok"});
    }
}
