#pragma once

#include "ClientSocket.hpp"
#include "ClientArgs.hpp"
#include "DropFileBaseException.hpp"
#include "RAIIFSEntry.hpp"

#include <indicators/progress_bar.hpp>

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
    SendFileAndReceiveCode sendFSEntryMetadata(const std::string &path);
    void sendFSEntry(RAIIFSEntry data_source);
private:
    std::pair<RAIIFSEntry, bool> compressIfNecessary(const std::string &path);
    indicators::ProgressBar createProgressBar();
    std::string getReceiveCodeFromServer();


    ClientSocket socket;
    static inline std::filesystem::path DROP_FILE_SENDER_TMP_DIR{std::filesystem::temp_directory_path() / "drop-file" / "sender"};
};


