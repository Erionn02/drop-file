#include "DropFileSendClient.hpp"
#include "InitSessionMessage.hpp"
#include "DirectoryCompressor.hpp"

#include <spdlog/spdlog.h>

DropFileSendClient::DropFileSendClient(ClientSocket socket) : socket(std::move(socket)) {
    std::filesystem::remove_all(DROP_FILE_SENDER_TMP_DIR);
    std::filesystem::create_directories(DROP_FILE_SENDER_TMP_DIR);
}

void DropFileSendClient::sendFSEntry(const std::string &path) {
    auto [fs_entry, is_compressed] = compressIfNecessary(path);
    spdlog::info("File to send: {}", fs_entry.path.string());
    nlohmann::json message_json = InitSessionMessage::createSendMessage(fs_entry.path, is_compressed);
    socket.SocketBase::send(message_json.dump());
    spdlog::info("Message sent.");
    std::string received_msg = socket.SocketBase::receive();
    spdlog::info("Server response: {}", received_msg);
    spdlog::info("Waiting for client to confirm");
    socket.SocketBase::receiveACK();
    std::ifstream file{fs_entry.path, std::ios::binary};
    sendFileImpl(std::move(file));
}

std::pair<RAIIFSEntry, bool> DropFileSendClient::compressIfNecessary(const std::string &path) {
    bool should_compress = std::filesystem::is_directory(path);
    RAIIFSEntry dir_entry{path, false};
    if (should_compress) {
        DirectoryCompressor dir_compressor{dir_entry.path};
        std::filesystem::path new_path = DROP_FILE_SENDER_TMP_DIR / dir_entry.path.filename();
        std::cout<<"Compressing to "<< new_path << std::endl;
        dir_compressor.compress(new_path);
        std::cout<<"Compressed!"<<std::endl;
        dir_entry = RAIIFSEntry{std::move(new_path), true};
    }
    return {std::move(dir_entry), should_compress};
}

void DropFileSendClient::sendFileImpl(std::ifstream data_source) {
    std::streamsize bytes_read;
    auto [buffer_ptr, buffer_size] = socket.SocketBase::getBuffer();
    do {
        bytes_read = data_source.readsome(buffer_ptr, static_cast<std::streamsize>(buffer_size));
        spdlog::debug("Bytes read: {}", bytes_read);
        if (bytes_read > 0) {
            socket.SocketBase::send({buffer_ptr, static_cast<std::size_t>(bytes_read)});
            socket.SocketBase::receiveACK();
        }
    } while (bytes_read > 0);
}
