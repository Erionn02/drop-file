#pragma once

#include "ClientSocket.hpp"
#include "ClientArgs.hpp"
#include "DropFileBaseException.hpp"
#include "RAIIFSEntry.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>

class DropFileSendException: public DropFileBaseException {
public:
    using DropFileBaseException::DropFileBaseException;
};

class DropFileSendClient {
public:
    DropFileSendClient(ClientSocket socket);
    void sendFSEntry(const std::string &path);
private:
    void sendFileImpl(std::ifstream data_source);
    std::pair<RAIIFSEntry, bool> compressIfNecessary(const std::string &path);


    ClientSocket socket;
    static inline std::filesystem::path DROP_FILE_SENDER_TMP_DIR{std::filesystem::temp_directory_path() / "drop-file" / "sender"};
};


