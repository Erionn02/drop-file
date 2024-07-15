#include "DropFileSendClient.hpp"
#include "InitSessionMessage.hpp"
#include "ArchiveManager.hpp"
#include "Utils.hpp"

#include <spdlog/spdlog.h>


DropFileSendClient::DropFileSendClient(ClientSocket socket) : socket(std::move(socket)) {
    std::filesystem::remove_all(DROP_FILE_SENDER_TMP_DIR);
    std::filesystem::create_directories(DROP_FILE_SENDER_TMP_DIR);
}

DropFileSendClient::~DropFileSendClient() {
    std::filesystem::remove_all(DROP_FILE_SENDER_TMP_DIR);
}

SendFileAndReceiveCode DropFileSendClient::sendFSEntryMetadata(const std::string &path) {
    auto [fs_entry, is_compressed] = compressIfNecessary(path);
    std::cout << "File to send: " << fs_entry.path << std::endl;
    nlohmann::json message_json = InitSessionMessage::createSendMessage(fs_entry.path, is_compressed);
    std::cout << "Requesting DropFileServer for unique receive code..." << std::endl;
    socket.SocketBase::send(message_json.dump());
    std::string receive_code = getReceiveCodeFromServer();
    std::cout << fmt::format("Enter on another device: './drop-file receive {}'", receive_code) << std::endl;
    return {std::move(fs_entry), std::move(receive_code)};
}

std::string DropFileSendClient::getReceiveCodeFromServer() {
    std::string received_msg = socket.SocketBase::receive();
    try {
        auto json = nlohmann::json::parse(received_msg);
        auto receive_code = json[InitSessionMessage::CODE_WORDS_KEY].get<std::string>();
        return receive_code;
    } catch (const nlohmann::json::exception &e) {
        throw DropFileSendException(
                fmt::format("Could not parse server response ({}) to json:  {}", received_msg, e.what()));
    }
}

std::pair<RAIIFSEntry, bool> DropFileSendClient::compressIfNecessary(const std::string &path) {
    bool should_compress = std::filesystem::is_directory(path);
    RAIIFSEntry dir_entry{path, false};
    if (should_compress) {
        ArchiveManager dir_compressor{dir_entry.path};
        std::filesystem::path new_path = DROP_FILE_SENDER_TMP_DIR / dir_entry.path.filename();
        std::cout << "Compressing to " << new_path << std::endl;
        dir_compressor.createArchive(new_path);
        std::cout << "Compressed!" << std::endl;
        dir_entry = RAIIFSEntry{std::move(new_path), true};
    }
    return {std::move(dir_entry), should_compress};
}

void DropFileSendClient::sendFSEntry(RAIIFSEntry data_source) {
    std::cout << "Waiting for other client to confirm transfer..." << std::endl;
    socket.SocketBase::receiveACK();
    std::cout<< "Other client confirmed transfer, sending " << data_source.path.filename() << std::endl;
    std::ifstream file{data_source.path, std::ios::binary};
    std::size_t file_size = std::filesystem::file_size(data_source.path);
    std::size_t total_bytes_read{0};
    auto progress_bar = createProgressBar("Sending file");
    std::streamsize bytes_read;
    auto [buffer_ptr, buffer_size] = socket.SocketBase::getBuffer();
    do {
        bytes_read = file.readsome(buffer_ptr, static_cast<std::streamsize>(buffer_size));
        if (bytes_read > 0) {
            total_bytes_read += static_cast<std::size_t>(bytes_read);
            socket.SocketBase::send({buffer_ptr, static_cast<std::size_t>(bytes_read)});
            progress_bar.set_progress(100 * total_bytes_read / file_size);
            socket.SocketBase::receiveACK();
        }
    } while (bytes_read > 0);
    progress_bar.set_option(indicators::option::PrefixText{"File sent."});
}