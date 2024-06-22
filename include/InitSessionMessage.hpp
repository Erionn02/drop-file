#pragma once

#include <nlohmann/json.hpp>

#include <stdexcept>


class InitSessionMessageException: public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};


class InitSessionMessage {
public:
    static nlohmann::json create(const std::string &parsed_test_file_path, const std::string &action);
    static nlohmann::json create(const std::string_view &str);

private:
    static void validate(const nlohmann::json& json);
    static void validateKeysTypes(const nlohmann::json &json);
    static void validateKeysExist(const nlohmann::json &json);
    static void validateActionKey(const nlohmann::json &json);

public:
    // send
    static inline const char* FILENAME_KEY{"filename"};
    static inline const char* FILE_SIZE_KEY{"file_size"};
    static inline const char* FILE_HASH_KEY{"file_hash"};
    static inline const char* IS_ZIPPED_KEY{"is_zipped"};

    // receive
    static inline const char* CODE_WORDS_KEY{"code_words_key"};

    // both
    static inline const char* ACTION_KEY{"action"};

    static void validateSingleKeyExists(const nlohmann::json &json, const char *key);

    static void validateStringKey(const nlohmann::json &json, const char *key);
};


