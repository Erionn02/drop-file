#include "InitSessionMessage.hpp"

#include <fmt/format.h>


nlohmann::json InitSessionMessage::createSendMessage(const std::string &parsed_test_file_path) {
    std::filesystem::path file_path{parsed_test_file_path};
    if (!std::filesystem::exists(file_path)) {
        throw InitSessionMessageException(fmt::format("Given path {} does not exist!", file_path.string()));
    }

    nlohmann::json json{};
    json[ACTION_KEY] = "send";
    json[FILENAME_KEY] = file_path.filename();
    json[FILE_SIZE_KEY] = std::filesystem::file_size(file_path);
    json[FILE_HASH_KEY] = std::filesystem::file_size(file_path); // todo calculate actual file hash
    json[IS_ZIPPED_KEY] = std::filesystem::is_directory(parsed_test_file_path);
    return json;
}

nlohmann::json InitSessionMessage::createReceiveMessage(const std::string &code) {
    nlohmann::json json{};
    json[ACTION_KEY] = "receive";
    json[CODE_WORDS_KEY] = code;
    return json;
}

nlohmann::json InitSessionMessage::create(const std::string_view &str) {
    try {
        nlohmann::json json = nlohmann::json::parse(str);
        validate(json);
        return json;
    } catch (const nlohmann::json::exception &e) {
        throw InitSessionMessageException(e.what());
    }
}

void InitSessionMessage::validate(const nlohmann::json &json) {
    validateActionKey(json);
    if (json[ACTION_KEY] == "send") {
        validateKeysExist(json);
        validateKeysTypes(json);
    } else {
        validateSingleKeyExists(json, CODE_WORDS_KEY);
        validateStringKey(json, CODE_WORDS_KEY);
    }
}

void InitSessionMessage::validateActionKey(const nlohmann::json &json) {
    validateSingleKeyExists(json, ACTION_KEY);
    validateStringKey(json, ACTION_KEY);
    if (json[ACTION_KEY] != "send" && json[ACTION_KEY] != "receive") {
        throw InitSessionMessageException(
                fmt::format("InitSessionMessage {} key has unrecognized value: {}.", ACTION_KEY,
                            json[ACTION_KEY].get<std::string>()));
    }
}

void InitSessionMessage::validateStringKey(const nlohmann::json &json, const char *key) {
    if (!json[key].is_string()) {
        throw InitSessionMessageException(fmt::format("InitSessionMessage json key {} should be a string.", key));
    }
}

void InitSessionMessage::validateKeysExist(const nlohmann::json &json) {
    for (auto key: {FILENAME_KEY, FILE_SIZE_KEY, FILE_HASH_KEY, IS_ZIPPED_KEY}) {
        validateSingleKeyExists(json, key);
    }
}

void InitSessionMessage::validateSingleKeyExists(const nlohmann::json &json, const char *key) {
    if (!json.contains(key)) {
        throw InitSessionMessageException(fmt::format("InitSessionMessage json is missing {} key.", key));
    }
}

void InitSessionMessage::validateKeysTypes(const nlohmann::json &json) {
    validateStringKey(json, FILENAME_KEY);

    for (auto &key: {FILE_SIZE_KEY, FILE_HASH_KEY}) {
        if (!json[key].is_number_unsigned()) {
            throw InitSessionMessageException(fmt::format("InitSessionMessage json key {} should be a number.", key));
        }
    }
    if (!json[IS_ZIPPED_KEY].is_boolean()) {
        throw InitSessionMessageException(
                fmt::format("InitSessionMessage json key {} should be a boolean.", IS_ZIPPED_KEY));
    }
}
