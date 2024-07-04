#include <gtest/gtest.h>

#include "ArgParser.hpp"
#include "DropFileSendClient.hpp"
#include "DropFileReceiveClient.hpp"
#include "DropFileServer.hpp"

#include <filesystem>


using namespace ::testing;

struct DropFileServerIntegrationTests : public Test {
    DropFileServer<> server{DropFileServer<>::DEFAULT_PORT, EXAMPLE_CERT_DIR };
    const std::filesystem::path TEST_FILE_PATH{std::filesystem::temp_directory_path() / "test_file.txt"};

    const std::string FILE_CONTENT{sizeof("Hello world, this is some content!")};
    std::jthread server_thread;

    void SetUp() override {
        std::filesystem::remove_all(getExpectedPath());
        std::ofstream file{TEST_FILE_PATH, std::ios::trunc | std::ios::binary};
        file.write(FILE_CONTENT.data(), static_cast<std::streamsize>(FILE_CONTENT.size()));
        server_thread = std::jthread{[&]{
            server.run();
        }};
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

    ClientSocket createClientSocket() {
        return {"localhost", DropFileServer<>::DEFAULT_PORT, EXAMPLE_CERT_DIR"/cert.pem"};
    }
};

TEST_F(DropFileServerIntegrationTests, canSendAndReceiveFile) {
    DropFileSendClient send_client{createClientSocket()};


    std::stringstream interaction_stream;
    interaction_stream << 'y';
    DropFileReceiveClient recv_client{createClientSocket(), interaction_stream};
    auto [fs_entry, receive_code] = send_client.sendFSEntryMetadata(TEST_FILE_PATH);

    auto receive_result = std::async(std::launch::async, [&]{
        recv_client.receiveFile(receive_code);
    });
    spdlog::info(receive_code);
    send_client.sendFSEntry(std::move(fs_entry));

    receive_result.get();
    ASSERT_TRUE(std::filesystem::exists(getExpectedPath()));
}