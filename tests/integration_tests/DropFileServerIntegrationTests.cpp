#include <gtest/gtest.h>

#include "ArgParser.hpp"
#include "DropFileSendClient.hpp"
#include "DropFileReceiveClient.hpp"
#include "DropFileServer.hpp"

#include <filesystem>


using namespace ::testing;

struct DropFileServerIntegrationTests : public Test {
    DropFileServer<> server{DropFileServer<>::DEFAULT_PORT, EXAMPLE_CERT_DIR "/key.pem"};
    const std::filesystem::path TEST_FILE_PATH{std::filesystem::temp_directory_path() / "test_file.txt"};

    const std::string FILE_CONTENT{"Hello world, this is some content!"};

    void SetUp() override {
        std::ofstream file{TEST_FILE_PATH, std::ios::trunc};
        file << FILE_CONTENT;
    }

    void TearDown () override {
        std::filesystem::remove_all(TEST_FILE_PATH);
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

    auto future_receive_code = std::async(std::launch::async, [&]{
        send_client.sendFSEntryMetadata(TEST_FILE_PATH);
        return "receive-code";
    });


    std::stringstream interaction_stream;
    interaction_stream << 'y';
    DropFileReceiveClient recv_client{createClientSocket(), interaction_stream};

    auto receive_result = std::async(std::launch::async, [&]{
        recv_client.receiveFile(future_receive_code.get());
    });

//    send_client.sendFSEntry({});
}