#include <gtest/gtest.h>

#include "ServerArgParser.hpp"


using namespace ::testing;
using namespace std::chrono_literals;

TEST(ServerArgParserTests, throwsWhenNoArgsAreSupplied) {
    int argc{1};
    char * argv[] = {"program_name"};

    ASSERT_THROW(parseServerArgs(argc, argv), std::runtime_error);
}

TEST(ServerArgParserTests, noThrowWhenOnlyDirectoryIsSuppliedAndDefultValuesAreCorrect) {
    int argc{2};
    char some_dir[] = "/some/directory";
    char * argv_only_first_arg[] = {"program_name", some_dir};
    ServerArgs server_args = parseServerArgs(argc, argv_only_first_arg);
    ASSERT_NO_THROW(server_args = parseServerArgs(argc, argv_only_first_arg));
    ASSERT_EQ(server_args.certs_directory, some_dir);
    ASSERT_EQ(server_args.port, ServerArgs::DEFAULT_PORT);
    ASSERT_EQ(server_args.client_timeout, ServerArgs::DEFAULT_CLIENT_TIMEOUT);
}

TEST(ServerArgParserTests, setsAllCustomValues) {
    int argc{6};
    char some_dir[] = "/some/directory";
    char * argv_only_first_arg[] = {"program_name", some_dir, "-p", "2137", "-t", "420"};
    ServerArgs server_args;
    ASSERT_NO_THROW(server_args = parseServerArgs(argc, argv_only_first_arg));
    ASSERT_EQ(server_args.certs_directory, some_dir);
    ASSERT_EQ(server_args.port, 2137);
    ASSERT_EQ(server_args.client_timeout, 420s);
}

TEST(ServerArgParserTests, setsCorrectPortValue) {
    int argc{4};
    char * argv[] = {"program_name", "/some/directory", "-p","3456"};
    ServerArgs server_args;
    ASSERT_NO_THROW(server_args = parseServerArgs(argc, argv));
    ASSERT_EQ(server_args.port, 3456);
}

TEST(ServerArgParserTests, setsCorrectTimeoutValue) {
    int argc{4};
    char * argv[] = {"program_name", "/some/directory", "-t","123"};
    ServerArgs server_args;
    ASSERT_NO_THROW(server_args = parseServerArgs(argc, argv));
    ASSERT_EQ(server_args.client_timeout, 123s);
}


TEST(ServerArgParserTests, throwsOnTooBigPortNumber) {
    int argc{4};
    char * argv[] = {"program_name", "/some/directory","--port", "345678"};
    ASSERT_THROW(parseServerArgs(argc, argv), std::exception);
}

TEST(ServerArgParserTests, throwsOnNegativePortNumber) {
    int argc{4};
    char * argv[] = {"program_name", "/some/directory", "--port","-8088"};
    ASSERT_THROW(parseServerArgs(argc, argv), std::exception);
}

TEST(ServerArgParserTests, throwsOnNegativeTimeout) {
    int argc{4};
    char * argv[] = {"program_name", "/some/directory", "--timeout","-8088"};
    ASSERT_THROW(parseServerArgs(argc, argv), std::exception);
}