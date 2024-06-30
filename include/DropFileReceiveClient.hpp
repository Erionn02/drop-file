#pragma once
#include "ClientSocket.hpp"

#include <string>
#include <iostream>

class DropFileReceiveClient {
public:
    DropFileReceiveClient(ClientSocket socket, std::istream& interaction_stream = std::cin);

    void receiveFile(const std::string& code_words);
private:
    void waitForConfirmation();
    void receiveFileImpl(std::ofstream& data_sink, std::size_t expected_bytes);

    ClientSocket socket;
    std::istream& interaction_stream; // to enable automatic testing with stream that is not a standard input
};


