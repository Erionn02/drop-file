#include "DropFileReceiveClient.hpp"
#include "InitSessionMessage.hpp"
#include "DirectoryCompressor.hpp"
#include "Utils.hpp"

#include <spdlog/spdlog.h>


DropFileReceiveClient::DropFileReceiveClient(ClientSocket socket, std::istream &interaction_stream) : socket(
        std::move(socket)), interaction_stream(interaction_stream) {
    std::filesystem::remove_all(DROP_FILE_RECEIVER_TMP_DIR);
    std::filesystem::create_directories(DROP_FILE_RECEIVER_TMP_DIR);
}

DropFileReceiveClient::~DropFileReceiveClient() {
    std::filesystem::remove_all(DROP_FILE_RECEIVER_TMP_DIR);
}

void DropFileReceiveClient::receiveFile(const std::string &code_words) {
    nlohmann::json message_json = InitSessionMessage::createReceiveMessage(code_words);
    std::cout << "Requesting server for file metadata..." << std::endl;
    socket.SocketBase::send(message_json.dump());
    nlohmann::json server_response = getServerResponse();

    getUserConfirmation();


    std::string filename = server_response[InitSessionMessage::FILENAME_KEY].get<std::string>();
    bool is_compressed = server_response[InitSessionMessage::IS_COMPRESSED_KEY].get<bool>();
    std::filesystem::path receive_path_base = is_compressed
                                              ? DROP_FILE_RECEIVER_TMP_DIR
                                              : std::filesystem::current_path();

    std::filesystem::path file_to_receive_path = receive_path_base / filename;

    receiveFileImpl(file_to_receive_path, server_response[InitSessionMessage::FILE_SIZE_KEY].get<std::size_t>());
    validateFileHash(file_to_receive_path, server_response[InitSessionMessage::FILE_HASH_KEY].get<std::string>());
    handleCompressedFile(is_compressed, file_to_receive_path);
}

nlohmann::json DropFileReceiveClient::getServerResponse() {
    auto received = socket.SocketBase::receive();
    try {
        auto json = nlohmann::json::parse(received);
        bool is_compressed = json[InitSessionMessage::IS_COMPRESSED_KEY].get<bool>();
        std::string filename = json[InitSessionMessage::FILENAME_KEY].get<std::string>();
        std::size_t file_size = json[InitSessionMessage::FILE_SIZE_KEY].get<std::size_t>();
        std::cout << (is_compressed ? "Directory" : "File") << " to receive: " << filename << std::endl;
        std::cout << (is_compressed ? "Compressed size: " : "Size: ") << bytesToHumanReadable(file_size) << std::endl;
        assertJsonProperties(json);
        return json;
    } catch (const nlohmann::json::exception &e) {
        throw DropFileReceiveException(fmt::format("Error, server response: {}", received));
    }
}

void DropFileReceiveClient::validateFileHash(const std::filesystem::path &compressed_file_path,
                                             const std::string &expected_file_hash) const {
    std::cout << "Comparing file hashes..." << std::endl;
    auto actual_file_hash = calculateFileHash(compressed_file_path);
    if (actual_file_hash != expected_file_hash) {
        throw DropFileReceiveException("Received file's hash is not equal to the expected one.");
    }
    std::cout << "File hashes match." << std::endl;
}

void DropFileReceiveClient::handleCompressedFile(bool is_compressed,
                                                 const std::filesystem::path &compressed_file_path) const {
    if (is_compressed) {
        DirectoryCompressor compressor{std::filesystem::current_path() / compressed_file_path.filename()};
        compressor.decompress(compressed_file_path);
        std::filesystem::remove(compressed_file_path);
    }
}

void DropFileReceiveClient::getUserConfirmation() {
    std::cout << "Do you want to proceed? [y/n]" << std::endl;
    char confirmation{};
    interaction_stream >> confirmation;
    if (confirmation != 'y') {
        std::cerr << fmt::format("Entered '{}', aborting.", confirmation) << std::endl;
        socket.SocketBase::send("abort");
        exit(1);
    }
    std::cout << "Sending confirmation..." << std::endl;
    socket.SocketBase::sendACK();
}

void
DropFileReceiveClient::receiveFileImpl(const std::filesystem::path &file_to_receive_path, std::size_t expected_bytes) {
    if (std::filesystem::exists(file_to_receive_path)) {
        throw DropFileReceiveException(fmt::format("Path {} already exists!", file_to_receive_path.string()));
    }
    std::ofstream received_file{file_to_receive_path, std::ios::trunc};
    std::size_t total_transferred_bytes{0};
    auto progress_bar = createProgressBar("Receiving file");
    while (total_transferred_bytes < expected_bytes) {
        std::string_view data = socket.SocketBase::receiveToBuffer();
        std::size_t left_to_transfer = expected_bytes - total_transferred_bytes;
        std::size_t write_size = std::min(left_to_transfer, data.size());
        received_file.write(data.data(), static_cast<std::streamsize>(write_size));
        total_transferred_bytes += write_size;
        socket.SocketBase::sendACK();
        progress_bar.set_progress(100 * total_transferred_bytes / expected_bytes);
    }
    received_file.flush();
    progress_bar.set_option(indicators::option::PrefixText{"File received."});
}

void DropFileReceiveClient::assertJsonProperties(const nlohmann::json &json) {
    std::string filename = json[InitSessionMessage::FILENAME_KEY].get<std::string>();
    if (std::filesystem::exists(filename)) {
        throw DropFileReceiveException(fmt::format("{} already exist!", filename));
    }
    bool is_compressed = json[InitSessionMessage::IS_COMPRESSED_KEY].get<bool>();

    std::filesystem::path base_dir = is_compressed ? std::filesystem::temp_directory_path()
                                                   : std::filesystem::current_path();
    std::filesystem::space_info directory_space_info = std::filesystem::space(base_dir);
    std::size_t file_size = json[InitSessionMessage::FILE_SIZE_KEY].get<std::size_t>();
    if (directory_space_info.free < file_size) {
        throw DropFileReceiveException("Not enough disk space to receive this file.");
    }
}
