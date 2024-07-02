#include <gtest/gtest.h>

#include "SocketBase.hpp"
#include "ClientSocket.hpp"
#include "DropFileServer.hpp"
#include "Utils.hpp"

using namespace ::testing;

class DummyTestSessionManager;

class SocketBaseWrapper : public SocketBase {
public:
    SocketBaseWrapper(tcp::socket socket, boost::asio::ssl::context &context,
                      std::weak_ptr<DummyTestSessionManager> test_session_manager) : SocketBase(
            {std::move(socket), context}), test_session_manager(std::move(test_session_manager)) {}

    void start();

    std::weak_ptr<DummyTestSessionManager> test_session_manager;
};

class DummyTestSessionManager {
public:
    DummyTestSessionManager(std::function<void(std::shared_ptr<SocketBase>)> set_test_socket) : set_test_socket(
            std::move(set_test_socket)) {}

    void setTestSocket(std::shared_ptr<SocketBase> socket) {
        set_test_socket(std::move(socket));
    }

    std::function<void(std::shared_ptr<SocketBase>)> set_test_socket;
};

void SocketBaseWrapper::start() {
    socket_.handshake(boost::asio::ssl::stream_base::server);
    if (auto manager = test_session_manager.lock()) {
        manager->setTestSocket(shared_from_this());
    }
}

struct ClientSocketTest : public Test {
    const unsigned short TEST_PORT{3421};
    std::shared_ptr<DummyTestSessionManager> test_session_manager{
            std::make_shared<DummyTestSessionManager>([this](auto socket) {
                setPeerSocket(std::move(socket));
            })};
    DropFileServer<SocketBaseWrapper, DummyTestSessionManager> test_server{DropFileServer<>::DEFAULT_PORT,
                                                                           EXAMPLE_CERT_DIR,
                                                                           test_session_manager};
    SocketBase::MessageHandler async_message_handler;
    std::jthread test_server_thread;
    std::atomic_flag is_peer_socket_set;
    std::shared_ptr<SocketBase> peer_socket{};

    void SetUp() override {
        spdlog::set_level(spdlog::level::debug);
        test_server_thread = std::jthread{[this] {
            test_server.run();
        }};
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    void TearDown() override {
        test_server.stop();
    }

    void setPeerSocket(std::shared_ptr<SocketBase> other_peer_socker) {
        if (!is_peer_socket_set.test_and_set()) {
            peer_socket = std::move(other_peer_socker);
        }
    }

    void waitForPeerSocket() {
        while (!is_peer_socket_set.test()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    std::future<std::string> asyncReceiveMessages(std::promise<std::string>& response_promise, std::size_t how_many) {
        async_message_handler = [&, how_many](std::string_view content){
            static std::string final_received_message{};
            static std::size_t received_messages{0};
            if (++received_messages < how_many) {
                final_received_message += content;
                peer_socket->asyncReadMessage(SocketBase::BUFFER_SIZE, async_message_handler);
            }
        };
        peer_socket->asyncReadMessage(SocketBase::BUFFER_SIZE, async_message_handler);
        return response_promise.get_future();
    }
};


TEST_F(ClientSocketTest, twoSocketsCanTalkToEachOther) {
    ClientSocket client_socket{"localhost", 12345, EXAMPLE_CERT_DIR"/cert.pem"};
    waitForPeerSocket();
    std::string_view test_message{"Hello, world!"};
    client_socket.send(test_message);

    auto received_message = peer_socket->receiveToBuffer();

    ASSERT_EQ(test_message, received_message);
}

TEST_F(ClientSocketTest, canReturnMessageCopy) {
    ClientSocket client_socket{"localhost", 12345, EXAMPLE_CERT_DIR"/cert.pem"};
    waitForPeerSocket();
    std::string_view test_message{"Hello, world!"};
    client_socket.send(test_message);

    auto received_message = peer_socket->receive();

    ASSERT_EQ(test_message, received_message);
}

TEST_F(ClientSocketTest, canSendAndReceiveACK) {
    ClientSocket client_socket{"localhost", 12345, EXAMPLE_CERT_DIR"/cert.pem"};
    waitForPeerSocket();

    client_socket.sendACK();
    ASSERT_NO_THROW(peer_socket->receiveACK());
}

TEST_F(ClientSocketTest, canSendInPartsMessageBiggerThanBufferSize) {
    ClientSocket client_socket{"localhost", 12345, EXAMPLE_CERT_DIR"/cert.pem"};
    waitForPeerSocket();

    std::size_t compounded_messages{3};
    std::size_t message_size = compounded_messages * SocketBase::BUFFER_SIZE;
    std::string message = generateRandomString(message_size);

    std::promise<std::string> result_promise;
    auto future_result = asyncReceiveMessages(result_promise, compounded_messages);
    client_socket.send(message);


    ASSERT_EQ(future_result.get(), message);
}

/*
    boost::asio::io_context io_context{};
    boost::asio::ssl::context ssl_context{boost::asio::ssl::context::sslv23};
    boost::asio::ssl::stream<tcp::socket> socket{io_context, ssl_context};
    tcp::resolver resolver(socket.get_executor());
    auto endpoints = resolver.resolve(host, std::to_string(port));
    if (endpoints.empty()) {
        throw SocketException(fmt::format("Did not find {}:{}", host, port));
    }
    socket.lowest_layer().connect(*endpoints.begin());
    socket.handshake(boost::asio::ssl::stream_base::client);
    SocketBase test_socket{std::move(socket)};
*/
