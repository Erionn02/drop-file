#include "ClientSession.hpp"

#include <spdlog/spdlog.h>

ClientSession::ClientSession(boost::asio::io_context &io_context, boost::asio::ssl::context &context,
                             const boost::asio::ip::basic_resolver<tcp, boost::asio::any_io_executor>::results_type &endpoints)
        : SocketBase({io_context, context}) {
    socket_.set_verify_mode(boost::asio::ssl::verify_peer);
    socket_.set_verify_callback([this](bool preverified, boost::asio::ssl::verify_context &ctx){
        return verify_certificate(preverified, ctx);
    });

    connect(endpoints);
}

bool ClientSession::verify_certificate(bool preverified, boost::asio::ssl::verify_context &ctx) {
    char subject_name[256];
    X509 *cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
    X509_NAME_oneline(X509_get_subject_name(cert), subject_name, sizeof(subject_name));
    spdlog::debug("Preverified {}, verifying {}", preverified, subject_name);
    return preverified;
}

void ClientSession::connect(const tcp::resolver::results_type &endpoints) {
    socket_.lowest_layer().connect(*endpoints.begin());
    socket_.handshake(boost::asio::ssl::stream_base::client);
}