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

using SendFileAndReceiveCode = std::pair<RAIIFSEntry, std::string>;

class DropFileSendClient {
public:
    DropFileSendClient(ClientSocket socket);
    ~DropFileSendClient();

    SendFileAndReceiveCode sendFSEntryMetadata(const std::string &path);
    void sendFSEntry(RAIIFSEntry data_source);
protected:
    std::pair<RAIIFSEntry, bool> compressIfNecessary(const std::string &path);
    std::string getReceiveCodeFromServer();


    ClientSocket socket;
    static inline std::filesystem::path DROP_FILE_SENDER_TMP_DIR{std::filesystem::temp_directory_path() / "drop-file" / "sender"};
};


