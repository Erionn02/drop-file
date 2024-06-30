#pragma once

#include "ClientSocket.hpp"
#include "ClientArgs.hpp"

#include <iostream>
#include <fstream>


class DropFileSendClient {
public:
    DropFileSendClient(ClientSocket socket);
    void sendFile(const std::string &path);
private:
    void sendFileImpl(std::ifstream data_source);


    ClientSocket socket;
};


