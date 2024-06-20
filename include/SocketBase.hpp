#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

using boost::asio::ip::tcp;


class SocketBase {
public:
    SocketBase(boost::asio::ssl::stream<tcp::socket> socket_);
    void send(const std::string& data);
    std::string receive();
protected:
    boost::asio::ssl::stream<tcp::socket> socket_;
    using MSG_HEADER_t = std::size_t;
    static constexpr MSG_HEADER_t MAX_MSG_SIZE{1024 * 1024 * 3}; // 3 MiB
};


