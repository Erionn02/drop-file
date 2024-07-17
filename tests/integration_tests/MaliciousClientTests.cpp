#include <gtest/gtest.h>

#include "TestHelpers.hpp"
#include "client/ClientArgParser.hpp"
#include "client/DropFileSendClient.hpp"
#include "client/DropFileReceiveClient.hpp"
#include "server/DropFileServer.hpp"
#include "InitSessionMessage.hpp"

#include <filesystem>


class MaliciousSendClient : public DropFileSendClient {
public:
    MaliciousSendClient(ClientSocket socket) : DropFileSendClient(std::move(socket)) {}

    void sendFSEntryThatWillNotMatchHash(RAIIFSEntry data_source) {
        socket.SocketBase::receiveACK();
        std::size_t file_size = std::filesystem::file_size(data_source.path);


        for (std::size_t total_bytes_sent{0}, bytes_sent{0}; total_bytes_sent < file_size; total_bytes_sent += bytes_sent) {
            std::size_t left_bytes_to_send = file_size - total_bytes_sent;
            bytes_sent = std::min(SocketBase::BUFFER_SIZE, left_bytes_to_send);
            socket.SocketBase::send(generateRandomString(bytes_sent));
            socket.SocketBase::receiveACK();
        }
    }

    void sendActualFSEntryAndAdditionalData(RAIIFSEntry data_source) {
        std::size_t file_size = std::filesystem::file_size(data_source.path);
        sendFSEntry(std::move(data_source));


        for (std::size_t total_bytes_sent{0}, bytes_sent{0}; total_bytes_sent < file_size; total_bytes_sent += bytes_sent) {
            std::size_t left_bytes_to_send = file_size - total_bytes_sent;
            bytes_sent = std::min(SocketBase::BUFFER_SIZE, left_bytes_to_send);
            socket.SocketBase::send(generateRandomString(bytes_sent));
            socket.SocketBase::receiveACK();
        }
    }
};


using namespace ::testing;

struct MaliciousClientTests : public Test {
    const unsigned short TEST_PORT{55342};
    DropFileServer<> server{TEST_PORT, EXAMPLE_CERT_DIR, std::make_shared<SessionsManager>(SessionsManager::DEFAULT_CLIENT_TIMEOUT, std::chrono::seconds(1))};
    const std::filesystem::path TEST_FILE_PATH{std::filesystem::temp_directory_path() / "test_fs_entry"};

    const std::string FILE_CONTENT{"Hello world, this is some content!"};
    std::stringstream interaction_stream;
    std::jthread server_thread;

    void SetUp() override {
        spdlog::set_level(spdlog::level::debug);
        std::filesystem::remove_all(getExpectedPath());
        server_thread = std::jthread{[&]{
            server.run();
        }};
    }

    void createTestFile() const {
        std::ofstream file{TEST_FILE_PATH, std::ios::trunc | std::ios::binary};
        file.write(FILE_CONTENT.data(), static_cast<std::streamsize>(FILE_CONTENT.size()));
    }

    void createTestDirectory() const {
        std::filesystem::create_directories(TEST_FILE_PATH);
        {
            std::ofstream file{TEST_FILE_PATH / "test_file.txt"};
            file << "Some content";
        }
        {
            std::ofstream file{TEST_FILE_PATH / "some_binary_thrash_file", std::ios::binary};
            auto content = generateRandomString(100000);
            file.write(content.data(), static_cast<long>(content.size()));
            file.flush();
        }
        fs::create_directories(TEST_FILE_PATH / "nested_dir");
        {
            std::ofstream file{TEST_FILE_PATH / "nested_dir" / "test_file.txt"};
            file << "Some other content";
        }
        fs::create_directories(TEST_FILE_PATH / "another_nested_dir");
    }

    void TearDown () override {
        std::filesystem::remove_all(TEST_FILE_PATH);
        std::filesystem::remove_all(getExpectedPath());
        server.stop();
        server_thread.join();
    }

    std::filesystem::path getExpectedPath() {
        return std::filesystem::current_path() / TEST_FILE_PATH.filename();
    }

    DropFileReceiveClient createRecvClient(char character) {
        interaction_stream << character;
        return {createClientSocket(), interaction_stream};
    }

    ClientSocket createClientSocket() {
        return {"localhost", TEST_PORT, false};
    }

};

TEST_F(MaliciousClientTests, serverDisconnectsSomebodyWhoSendsFirstMessageThatIsNotJson) {
    auto test_client = createClientSocket();

    std::size_t message_length{250};
    std::string not_a_json_msg(message_length, 'a');
    test_client.SocketBase::send(not_a_json_msg);
    ASSERT_EQ(test_client.SocketBase::receive(), "Could not parse to json.");
    ASSERT_THROW(test_client.SocketBase::receive(), boost::exception); // disconnected
}

TEST_F(MaliciousClientTests, serverDisconnectsSomebodyWhoSendsFirstMessageThatIsAJsonButNotAProperOne) {
    auto test_client = createClientSocket();

    nlohmann::json json;
    std::string action_value = "not-a-recv-or-send";
    json[InitSessionMessage::ACTION_KEY] = action_value;
    test_client.SocketBase::send(json.dump());
    ASSERT_EQ(test_client.SocketBase::receive(), fmt::format("InitSessionMessage {} key has unrecognized value: {}.", InitSessionMessage::ACTION_KEY, action_value));
    ASSERT_THROW(test_client.SocketBase::receive(), boost::exception); // disconnected
}

TEST_F(MaliciousClientTests, serverDoesNotAllowAbnormallyBigFirstMessages) {
    auto test_client = createClientSocket();

    std::size_t message_length{100000};
    std::string maliciously_long_file_hash(message_length, 'a');
    createTestFile();
    auto json_msg = InitSessionMessage::createSendMessage(TEST_FILE_PATH, false);
    json_msg[InitSessionMessage::FILE_HASH_KEY] = maliciously_long_file_hash;
    auto msg = json_msg.dump();
    test_client.send(msg);
    try {
        auto response = test_client.receive();
        ASSERT_TRUE(response.contains(fmt::format("Tried to send {} bytes, which is more than allowed", msg.size())));
    } catch (const boost::wrapexcept<boost::system::system_error> &e) {
        // or disconnects right away
    }
}

TEST_F(MaliciousClientTests, receiveClientChecksHashAndDiscardsIfItDoesNotMatch) {
    MaliciousSendClient send_client{createClientSocket()};

    createTestDirectory();

    DropFileReceiveClient recv_client{createRecvClient('y')};

    auto [fs_entry, receive_code] = send_client.sendFSEntryMetadata(TEST_FILE_PATH);

    auto send_result = std::async(std::launch::async, [&]{
        send_client.sendFSEntryThatWillNotMatchHash(std::move(fs_entry));
    });
    ASSERT_THROW(recv_client.receiveFile(receive_code), DropFileReceiveException);
    send_result.get();
}

TEST_F(MaliciousClientTests, serverControlsAmountOfBytesThatAreSupposedToBeSent) {
    MaliciousSendClient send_client{createClientSocket()};

    createTestDirectory();

    DropFileReceiveClient recv_client{createRecvClient('y')};

    auto [fs_entry, receive_code] = send_client.sendFSEntryMetadata(TEST_FILE_PATH);

    auto send_result = std::async(std::launch::async, [&]{
        send_client.sendActualFSEntryAndAdditionalData(std::move(fs_entry));
    });
    ASSERT_NO_THROW(recv_client.receiveFile(receive_code)); // only receives declared bytes, no more,
    ASSERT_THROW(send_result.get(), boost::exception); // disconnected
}
