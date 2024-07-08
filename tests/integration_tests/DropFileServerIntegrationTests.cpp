#include <gtest/gtest.h>

#include "TestHelpers.hpp"
#include "ArgParser.hpp"
#include "DropFileSendClient.hpp"
#include "DropFileReceiveClient.hpp"
#include "DropFileServer.hpp"


#include <filesystem>


using namespace ::testing;

struct DropFileServerIntegrationTests : public Test {
    const unsigned short TEST_PORT{61342};
    DropFileServer<> server{TEST_PORT, EXAMPLE_CERT_DIR };
    const std::filesystem::path TEST_FILE_PATH{std::filesystem::temp_directory_path() / "test_fs_entry"};
    std::stringstream interaction_stream;

    const std::string FILE_CONTENT{"Hello world, this is some content!"};
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
        interaction_stream.clear();
    }

    std::filesystem::path getExpectedPath() {
        return std::filesystem::current_path() / TEST_FILE_PATH.filename();
    }

    ClientSocket createClientSocket() {
        return {"localhost", TEST_PORT, EXAMPLE_CERT_DIR "/cert.pem"};
    }

    DropFileReceiveClient createRecvClient(char character) {
        interaction_stream << character;
        return {createClientSocket(), interaction_stream};
    }
};

TEST_F(DropFileServerIntegrationTests, doesNotSendFileWhenUserDoesNotConfirm) {
    DropFileSendClient send_client{createClientSocket()};

    createTestFile();

    DropFileReceiveClient recv_client{createRecvClient('n')};

    auto [fs_entry, receive_code] = send_client.sendFSEntryMetadata(TEST_FILE_PATH);

    ASSERT_DEATH(recv_client.receiveFile(receive_code), "Entered 'n', aborting.");
}

TEST_F(DropFileServerIntegrationTests, canSendAndReceiveFile) {
    DropFileSendClient send_client{createClientSocket()};

    createTestFile();

    DropFileReceiveClient recv_client{createRecvClient('y')};

    auto [fs_entry, receive_code] = send_client.sendFSEntryMetadata(TEST_FILE_PATH);

    auto receive_result = std::async(std::launch::async, [&]{
        recv_client.receiveFile(receive_code);
    });

    send_client.sendFSEntry(std::move(fs_entry));

    receive_result.get();
    ASSERT_TRUE(std::filesystem::exists(getExpectedPath()));
    ASSERT_EQ(getFileContent(TEST_FILE_PATH), FILE_CONTENT);
    ASSERT_EQ(getFileContent(getExpectedPath()), FILE_CONTENT);
}

TEST_F(DropFileServerIntegrationTests, canSendAndReceiveDirectory) {
    DropFileSendClient send_client{createClientSocket()};

    createTestDirectory();

    DropFileReceiveClient recv_client{createRecvClient('y')};

    auto [fs_entry, receive_code] = send_client.sendFSEntryMetadata(TEST_FILE_PATH);

    auto send_result = std::async(std::launch::async, [&]{
        send_client.sendFSEntry(std::move(fs_entry));
    });
    recv_client.receiveFile(receive_code);


    send_result.get();
    ASSERT_TRUE(std::filesystem::exists(getExpectedPath()));
    ASSERT_TRUE(std::filesystem::is_directory(getExpectedPath()));
    assertDirectoriesEqual(getExpectedPath(), TEST_FILE_PATH);
}

TEST_F(DropFileServerIntegrationTests, throwsWhenGivenPathAlreadyExists) {
    DropFileSendClient send_client{createClientSocket()};

    createTestDirectory();

    DropFileReceiveClient recv_client{createRecvClient('y')};

    auto [fs_entry, receive_code] = send_client.sendFSEntryMetadata(TEST_FILE_PATH);

    std::filesystem::create_directories(getExpectedPath());
    ASSERT_THROW(recv_client.receiveFile(receive_code), DropFileReceiveException);
}

TEST_F(DropFileServerIntegrationTests, throwsWhenReceivesWithNonExistentCode) {
    DropFileReceiveClient recv_client{createRecvClient('y')};

    ASSERT_THROW(recv_client.receiveFile("some-non-existent-recv-code"), DropFileReceiveException);
}
