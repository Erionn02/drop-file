#pragma once
#include "ClientSocket.hpp"

#include <string>
#include <iostream>
#include <filesystem>

class DropFileReceiveClient {
public:
    DropFileReceiveClient(ClientSocket socket, std::istream& interaction_stream = std::cin);

    void receiveFile(const std::string& code_words);
private:
    void waitForConfirmation();
    void receiveFileImpl(std::ofstream& data_sink, std::size_t expected_bytes);
    void handleCompressedFile(bool is_compressed, const std::filesystem::path &compressed_file_path) const;
    void validateFileHash(const std::filesystem::path &compressed_file_path, const std::string &expected_file_hash) const;

    ClientSocket socket;
    std::istream& interaction_stream; // to enable automatic testing with stream that is not a standard input
    static inline std::filesystem::path DROP_FILE_RECEIVER_TMP_DIR{std::filesystem::temp_directory_path() / "drop-file" / "receiver"};
};


