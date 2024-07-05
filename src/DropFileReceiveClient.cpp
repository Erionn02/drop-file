#include "DropFileReceiveClient.hpp"
#include "InitSessionMessage.hpp"
#include "DirectoryCompressor.hpp"
#include "FileHash.hpp"

#include <spdlog/spdlog.h>


DropFileReceiveClient::DropFileReceiveClient(ClientSocket socket, std::istream &interaction_stream) : socket(
        std::move(socket)), interaction_stream(interaction_stream) {
    std::filesystem::remove_all(DROP_FILE_RECEIVER_TMP_DIR);
    std::filesystem::create_directories(DROP_FILE_RECEIVER_TMP_DIR);
}

void DropFileReceiveClient::receiveFile(const std::string &code_words) {
    nlohmann::json message_json = InitSessionMessage::createReceiveMessage(code_words);
    socket.SocketBase::send(message_json.dump());
    spdlog::info("Message sent.");
    auto received = socket.SocketBase::receive();
    spdlog::info("Server response: {}", received);

    waitForUserConfirmation();

    spdlog::info("Sending confirmation...");
    socket.SocketBase::sendACK();
    auto json = nlohmann::json::parse(received);
    std::string filename = json[InitSessionMessage::FILENAME_KEY].get<std::string>();
    bool is_compressed = json[InitSessionMessage::IS_COMPRESSED_KEY].get<bool>();
    std::filesystem::path receive_path_base = is_compressed
                                              ? DROP_FILE_RECEIVER_TMP_DIR
                                              : std::filesystem::current_path();

    std::filesystem::path file_to_receive_path = receive_path_base / filename;

    receiveFileImpl(file_to_receive_path, json[InitSessionMessage::FILE_SIZE_KEY].get<std::size_t>());
    validateFileHash(file_to_receive_path, json[InitSessionMessage::FILE_HASH_KEY].get<std::string>());
    handleCompressedFile(is_compressed, file_to_receive_path);
}

void DropFileReceiveClient::validateFileHash(const std::filesystem::path &compressed_file_path,
                                             const std::string &expected_file_hash) const {
    auto actual_file_hash = calculateFileHash(compressed_file_path);
    if (actual_file_hash != expected_file_hash) {
        throw DropFileBaseException("Received file's hash is not equal to the expected one.");
    }
    std::cout << "File hashes match." << std::endl;
}

void DropFileReceiveClient::handleCompressedFile(bool is_compressed,
                                                 const std::filesystem::path &compressed_file_path) const {
    if (is_compressed) {
        DirectoryCompressor compressor{std::filesystem::current_path() / compressed_file_path.filename()};
        std::cout << "Decompressing..." << std::endl;
        compressor.decompress(compressed_file_path);
        std::cout << "Decompressed..." << std::endl;
        std::filesystem::remove(compressed_file_path);
    }
}

void DropFileReceiveClient::waitForUserConfirmation() {
    spdlog::info("Do you want to proceed? [y/n]");
    char confirmation{};
    interaction_stream >> confirmation;
    if (confirmation != 'y') {
        std::cerr<< fmt::format("Entered '{}', aborting.", confirmation);
        socket.SocketBase::send("abort");
        exit(1);
    }
}

void DropFileReceiveClient::receiveFileImpl(const std::filesystem::path &file_to_receive_path, std::size_t expected_bytes) {
    spdlog::info("Receiving file...");
    std::ofstream received_file{file_to_receive_path, std::ios::trunc};
    std::size_t total_transferred_bytes{0};
    while (total_transferred_bytes < expected_bytes) {
        std::string_view data = socket.SocketBase::receiveToBuffer();
        spdlog::debug("Received {} bytes chunk", data.size());
        std::size_t left_to_transfer = expected_bytes - total_transferred_bytes;
        std::size_t write_size = std::min(left_to_transfer, data.size());
        received_file.write(data.data(), static_cast<std::streamsize>(write_size));
        total_transferred_bytes += write_size;
        socket.SocketBase::sendACK();
    }
    received_file.flush();
}
