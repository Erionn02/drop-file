#include <gtest/gtest.h>

#include "InitSessionMessage.hpp"

#include <fstream>

using namespace ::testing;

struct DropFileServerIntegrationTests : public Test {
    std::filesystem::path path{std::filesystem::temp_directory_path() / "test_asset"};

    void TearDown() override{
        std::filesystem::remove(path);
    }
};

TEST_F(DropFileServerIntegrationTests, throwsOnStringThatIsNotAValidJson) {
    ASSERT_THROW(InitSessionMessage::create("not a json str"), InitSessionMessageException);
}

TEST_F(DropFileServerIntegrationTests, throwsOnJsonWithoutActionKey) {
    nlohmann::json json{};
    ASSERT_THROW(InitSessionMessage::create(json.dump()), InitSessionMessageException);
}

TEST_F(DropFileServerIntegrationTests, throwsOnJsonWithActionKeyNotAString) {
    nlohmann::json json{};
    json[InitSessionMessage::ACTION_KEY] = 12345;
    ASSERT_THROW(InitSessionMessage::create(json.dump()), InitSessionMessageException);
}

TEST_F(DropFileServerIntegrationTests, throwsOnActionKeyAStringButUnexpectedOne) {
    nlohmann::json json{};
    json[InitSessionMessage::ACTION_KEY] = "Not 'send' or 'receive'";
    ASSERT_THROW(InitSessionMessage::create(json.dump()), InitSessionMessageException);
}

TEST_F(DropFileServerIntegrationTests, throwsOnActionKeySendButMissingOthers) {
    nlohmann::json json{};
    json[InitSessionMessage::ACTION_KEY] = "send";
    ASSERT_THROW(InitSessionMessage::create(json.dump()), InitSessionMessageException);
}

TEST_F(DropFileServerIntegrationTests, throwsOnSendActionButOtherKeysHaveWrongType) {
    nlohmann::json json{};
    json[InitSessionMessage::ACTION_KEY] = "send";
    for (auto key: {InitSessionMessage::FILENAME_KEY, InitSessionMessage::FILE_SIZE_KEY,
                    InitSessionMessage::FILE_HASH_KEY, InitSessionMessage::IS_COMPRESSED_KEY}) {
        json[key] = key;
    }
    ASSERT_THROW(InitSessionMessage::create(json.dump()), InitSessionMessageException);
}

TEST_F(DropFileServerIntegrationTests, doesNotThrowOnJsonWithExpectedStructure) {
    nlohmann::json json{};
    json[InitSessionMessage::ACTION_KEY] = "send";
    json[InitSessionMessage::FILENAME_KEY] = "example_filename.txt";
    json[InitSessionMessage::IS_COMPRESSED_KEY] = true;
    json[InitSessionMessage::FILE_SIZE_KEY] = 12345;
    json[InitSessionMessage::FILE_HASH_KEY] = 6789;

    ASSERT_NO_THROW(InitSessionMessage::create(json.dump()));
}

TEST_F(DropFileServerIntegrationTests, ignoresOtherAdditionalUnrecognizedKeys) {
    nlohmann::json json{};
    json[InitSessionMessage::ACTION_KEY] = "send";
    json[InitSessionMessage::FILENAME_KEY] = "example_filename.txt";
    json[InitSessionMessage::IS_COMPRESSED_KEY] = true;
    json[InitSessionMessage::FILE_SIZE_KEY] = 12345;
    json[InitSessionMessage::FILE_HASH_KEY] = 6789;
    json["Some completely random key"] = 6789;

    ASSERT_NO_THROW(InitSessionMessage::create(json.dump()));
}

TEST_F(DropFileServerIntegrationTests, throwsOnActionKeyReceiveButMissingCodeWordsKey) {
    nlohmann::json json{};
    json[InitSessionMessage::ACTION_KEY] = "receive";
    ASSERT_THROW(InitSessionMessage::create(json.dump()), InitSessionMessageException);
}

TEST_F(DropFileServerIntegrationTests, throwsOnCodeWordsKeyIncorrectType) {
    nlohmann::json json{};
    json[InitSessionMessage::ACTION_KEY] = "receive";
    json[InitSessionMessage::CODE_WORDS_KEY] = 123456789;
    ASSERT_THROW(InitSessionMessage::create(json.dump()), InitSessionMessageException);
}

TEST_F(DropFileServerIntegrationTests, doesNotThrowOnCorrectReceiveJson) {
    nlohmann::json json{};
    json[InitSessionMessage::ACTION_KEY] = "receive";
    json[InitSessionMessage::CODE_WORDS_KEY] = "super-drop-file-program";
    ASSERT_NO_THROW(InitSessionMessage::create(json.dump()));
}

TEST_F(DropFileServerIntegrationTests, ignoredAdditionalKeys) {
    nlohmann::json json{};
    json[InitSessionMessage::ACTION_KEY] = "receive";
    json[InitSessionMessage::CODE_WORDS_KEY] = "super-drop-file-program";
    json["Additional unrecognized key"] = true;
    ASSERT_NO_THROW(InitSessionMessage::create(json.dump()));
}

TEST_F(DropFileServerIntegrationTests, createSendMessageThrowsWhenFileDoesNotExist) {
    ASSERT_THROW(InitSessionMessage::createSendMessage(path, false), InitSessionMessageException);
}

TEST_F(DropFileServerIntegrationTests, createSendMessageReturnsCorrectJsonOnFile) {
    std::ofstream f{path};
    bool is_zipped = false;
    auto json = InitSessionMessage::createSendMessage(path, is_zipped);
    ASSERT_EQ(json[InitSessionMessage::ACTION_KEY], "send");
    ASSERT_EQ(json[InitSessionMessage::FILENAME_KEY], path.filename().string());
    ASSERT_EQ(json[InitSessionMessage::FILE_SIZE_KEY], 0);
    ASSERT_EQ(json[InitSessionMessage::FILE_HASH_KEY], 0);
    ASSERT_EQ(json[InitSessionMessage::IS_COMPRESSED_KEY], is_zipped);
}

TEST_F(DropFileServerIntegrationTests, createReceiveMessageReturnsCorrectJsonOnDirectory) {
    std::string code_words{"some-code-words123"};
    auto json = InitSessionMessage::createReceiveMessage(code_words);
    ASSERT_EQ(json[InitSessionMessage::ACTION_KEY], "receive");
    ASSERT_EQ(json[InitSessionMessage::CODE_WORDS_KEY], code_words);
}
