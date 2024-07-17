#pragma once

#include "ClientArgs.hpp"
#include "DropFileBaseException.hpp"

class ClientArgParserException: public DropFileBaseException {
public:
    using DropFileBaseException::DropFileBaseException;
};

ClientArgs parseClientArgs(int argc, char* argv[]);
