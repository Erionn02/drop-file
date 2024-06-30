#include <gtest/gtest.h>

#include "InitSessionMessage.hpp"

using namespace ::testing;

struct DropFileServerIntegrationTests : public Test {
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
    json[InitSessionMessage::ACTION_KEY] = "Not 'sendFile' or 'receiveFile'";
    ASSERT_THROW(InitSessionMessage::create(json.dump()), InitSessionMessageException);
}

TEST_F(DropFileServerIntegrationTests, throwsOnActionKeySendButMissingOthers) {
    nlohmann::json json{};
    json[InitSessionMessage::ACTION_KEY] = "sendFile";
    ASSERT_THROW(InitSessionMessage::create(json.dump()), InitSessionMessageException);
}

TEST_F(DropFileServerIntegrationTests, throwsOnSendActionButOtherKeysHaveWrongType) {
    nlohmann::json json{};
    json[InitSessionMessage::ACTION_KEY] = "sendFile";
    for (auto key: {InitSessionMessage::FILENAME_KEY, InitSessionMessage::FILE_SIZE_KEY,
                    InitSessionMessage::FILE_HASH_KEY, InitSessionMessage::IS_ZIPPED_KEY}) {
        json[key] = key;
    }
    ASSERT_THROW(InitSessionMessage::create(json.dump()), InitSessionMessageException);
}

TEST_F(DropFileServerIntegrationTests, doesNotThrowOnJsonWithExpectedStructure) {
    nlohmann::json json{};
    json[InitSessionMessage::ACTION_KEY] = "sendFile";
    json[InitSessionMessage::FILENAME_KEY] = "example_filename.txt";
    json[InitSessionMessage::IS_ZIPPED_KEY] = true;
    json[InitSessionMessage::FILE_SIZE_KEY] = 12345;
    json[InitSessionMessage::FILE_HASH_KEY] = 6789;

    ASSERT_NO_THROW(InitSessionMessage::create(json.dump()));
}

TEST_F(DropFileServerIntegrationTests, ignoresOtherAdditionalUnrecognizedKeys) {
    nlohmann::json json{};
    json[InitSessionMessage::ACTION_KEY] = "sendFile";
    json[InitSessionMessage::FILENAME_KEY] = "example_filename.txt";
    json[InitSessionMessage::IS_ZIPPED_KEY] = true;
    json[InitSessionMessage::FILE_SIZE_KEY] = 12345;
    json[InitSessionMessage::FILE_HASH_KEY] = 6789;
    json["Some completely random key"] = 6789;

    ASSERT_NO_THROW(InitSessionMessage::create(json.dump()));
}

TEST_F(DropFileServerIntegrationTests, throwsOnActionKeyReceiveButMissingCodeWordsKey) {
    nlohmann::json json{};
    json[InitSessionMessage::ACTION_KEY] = "receiveFile";
    ASSERT_THROW(InitSessionMessage::create(json.dump()), InitSessionMessageException);
}

TEST_F(DropFileServerIntegrationTests, throwsOnCodeWordsKeyIncorrectType) {
    nlohmann::json json{};
    json[InitSessionMessage::ACTION_KEY] = "receiveFile";
    json[InitSessionMessage::CODE_WORDS_KEY] = 123456789;
    ASSERT_THROW(InitSessionMessage::create(json.dump()), InitSessionMessageException);
}

TEST_F(DropFileServerIntegrationTests, doesNotThrowOnCorrectReceiveJson) {
    nlohmann::json json{};
    json[InitSessionMessage::ACTION_KEY] = "receiveFile";
    json[InitSessionMessage::CODE_WORDS_KEY] = "super-drop-file-program";
    ASSERT_NO_THROW(InitSessionMessage::create(json.dump()));
}

TEST_F(DropFileServerIntegrationTests, ignoredAdditionalKeys) {
    nlohmann::json json{};
    json[InitSessionMessage::ACTION_KEY] = "receiveFile";
    json[InitSessionMessage::CODE_WORDS_KEY] = "super-drop-file-program";
    json["Additional unrecognized key"] = true;
    ASSERT_NO_THROW(InitSessionMessage::create(json.dump()));
}