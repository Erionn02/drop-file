#include <gtest/gtest.h>

#include "ClientArgParser.hpp"


using namespace ::testing;

TEST(ClientArgParserTests, throwsWhenNoArgsAreSupplied) {
    int argc{1};
    char * argv[] = {"program_name"};

    ASSERT_THROW(parseClientArgs(argc, argv), std::runtime_error);
}

TEST(ClientArgParserTests, throwsWhenIncorrectPositionalArgValue) {
    int argc{2};
    char * argv_only_first_arg[] = {"program_name", "not-send_or_receive_string"};
    ASSERT_THROW(parseClientArgs(argc, argv_only_first_arg), std::runtime_error);

    char * argv[] = {"program_name", "not-send_or_receive_string", "second_arg"};
    ASSERT_THROW(parseClientArgs(3, argv), std::runtime_error);

}

TEST(ClientArgParserTests, throwsWhenOnlyFirstArgValueCorrect) {
    int argc{2};
    char * argv_send[] = {"program_name", "send"};
    char * argv_recv[] = {"program_name", "receive"};

    ASSERT_THROW(parseClientArgs(argc, argv_send), std::runtime_error);
    ASSERT_THROW(parseClientArgs(argc, argv_recv), std::runtime_error);
}

TEST(ClientArgParserTests, noThrowWhenCorrectSendArgs) {
    int argc{3};
    char * argv_send[] = {"program_name", "send", "send_value"};
    ClientArgs send_args;

    ASSERT_NO_THROW(send_args = parseClientArgs(argc, argv_send));
    ASSERT_EQ(send_args.action, Action::send);
    ASSERT_FALSE(send_args.receive_code.has_value());
    ASSERT_EQ(*send_args.file_to_send_path, "send_value");
}

TEST(ClientArgParserTests, noThrowWhenCorrectRecvArgs) {
    int argc{3};
    char * argv_recv[] = {"program_name", "receive", "recv_value"};
    ClientArgs recv_args;

    ASSERT_NO_THROW(recv_args = parseClientArgs(argc, argv_recv));
    ASSERT_EQ(recv_args.action, Action::receive);
    ASSERT_EQ(*recv_args.receive_code, "recv_value");
    ASSERT_FALSE(recv_args.file_to_send_path.has_value());
}

TEST(ClientArgParserTests, setsCorrectDefaultValuesOnSend) {
    int argc{3};
    char * argv_send[] = {"program_name", "send", "wefwefwe"};
    ClientArgs args = parseClientArgs(argc, argv_send);
    ASSERT_EQ(args.port, 12345);
    ASSERT_EQ(args.server_domain_name, "localhost");
    ASSERT_FALSE(args.path_to_server_cert_file.has_value());
}

TEST(ClientArgParserTests, setsCorrectDefaultValuesOnReceive) {
    int argc{3};
    char * argv_send[] = {"program_name", "receive", "wefwefwe"};
    ClientArgs args = parseClientArgs(argc, argv_send);
    ASSERT_EQ(args.port, 12345);
    ASSERT_EQ(args.server_domain_name, "localhost");
    ASSERT_FALSE(args.path_to_server_cert_file.has_value());
}

TEST(ClientArgParserTests, setsPathToServerCertFile) {
    int argc{5};
    char * argv_send[] = {"program_name", "--path_to_server_cert_file", "some_path", "receive", "wefwefwe"};
    ClientArgs args = parseClientArgs(argc, argv_send);
    ASSERT_TRUE(args.path_to_server_cert_file.has_value());
    ASSERT_EQ(*args.path_to_server_cert_file, "some_path");
}

TEST(ClientArgParserTests, trimsSlashesOutOfPaths) {
    int argc{3};
    char * argv_send[] = {"program_name", "send", "/some/path////"};
    ClientArgs send_args = parseClientArgs(argc, argv_send);

    ASSERT_EQ(*send_args.file_to_send_path, "/some/path");
}

TEST(ClientArgParserTests, doesNotTrimSlashes) {
    int argc{3};
    char * argv_send[] = {"program_name", "send", "/some/path"};
    ClientArgs send_args = parseClientArgs(argc, argv_send);

    ASSERT_EQ(*send_args.file_to_send_path, "/some/path");
}

TEST(ClientArgParserTests, doesNotTrimFirstSlashCharacter) {
    int argc{3};
    char * argv_send[] = {"program_name", "send", "/////"};
    ClientArgs send_args = parseClientArgs(argc, argv_send);

    ASSERT_EQ(*send_args.file_to_send_path, "/");
}

TEST(ClientArgParserTests, doesNotTrimAnything) {
    int argc{3};
    char * argv_send[] = {"program_name", "send", "aaa"};
    ClientArgs send_args = parseClientArgs(argc, argv_send);

    ASSERT_EQ(*send_args.file_to_send_path, "aaa");
}

TEST(ClientArgParserTests, doesNotBreakWhenThirdArgIsSomehowEmptyStr) {
    int argc{3};
    char * argv_send[] = {"program_name", "send", ""};
    ASSERT_EQ(*parseClientArgs(argc, argv_send).file_to_send_path, "");
}