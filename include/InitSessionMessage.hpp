#pragma once

#include "DropFileBaseException.hpp"

#include <nlohmann/json.hpp>


class InitSessionMessageException: public DropFileBaseException {
public:
    using DropFileBaseException::DropFileBaseException;
};


class InitSessionMessage {
public:
    static nlohmann::json createSendMessage(const std::filesystem::path &file_path, bool is_compressed);
    static nlohmann::json createReceiveMessage(const std::string &code);
    static nlohmann::json create(const std::string_view &str);

private:
    static void validate(const nlohmann::json& json);
    static void validateKeysTypes(const nlohmann::json &json);
    static void validateKeysExist(const nlohmann::json &json);
    static void validateActionKey(const nlohmann::json &json);
    static void validateSingleKeyExists(const nlohmann::json &json, const char *key);
    static void validateStringKey(const nlohmann::json &json, const char *key);
public:
    // send
    static inline const char* FILENAME_KEY{"filename"};
    static inline const char* FILE_SIZE_KEY{"file_size"};
    static inline const char* FILE_HASH_KEY{"file_hash"};
    static inline const char* IS_COMPRESSED_KEY{"is_compressed"};

    // receive
    static inline const char* CODE_WORDS_KEY{"code_words_key"};

    // both
    static inline const char* ACTION_KEY{"action"};
};


