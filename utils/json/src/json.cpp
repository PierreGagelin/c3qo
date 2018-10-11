

#define LOGGER_TAG "[lib.json]"

#include "utils/json.hpp"
#include "utils/logger.hpp"

#include "jsmn.h"

//
// @brief Helper function to show JSMN type
//
const char *json_type_to_string(enum json_type type)
{
    switch (type)
    {
    case JSON_UNDEFINED:
        return "JSON_UNDEFINED";
    case JSON_OBJECT:
        return "JSON_OBJECT";
    case JSON_ARRAY:
        return "JSON_ARRAY";
    case JSON_STRING:
        return "JSON_STRING";
    case JSON_PRIMITIVE:
        return "JSON_PRIMITIVE";
    default:
        return "UNKNOWN";
    }
}

//
// @brief Convert JSMN type to JSON type
//        basically the same, just ensure default value
//
static enum json_type jsmn_type_to_json_type(jsmntype_t type)
{
    switch (type)
    {
    case JSMN_OBJECT:
        return JSON_OBJECT;
    case JSMN_ARRAY:
        return JSON_ARRAY;
    case JSMN_STRING:
        return JSON_STRING;
    case JSMN_PRIMITIVE:
        return JSON_PRIMITIVE;
    default:
        return JSON_UNDEFINED;
    }
}

//
// @brief Parse a JSON string and fills output tokens
//
// Wrapper around JSMN library to format and verify its output
//
bool json_parse(const char *json, std::vector<struct json_entry> &output)
{
    jsmn_parser parser;
    jsmntok_t *token;
    unsigned int token_count;
    int ret;

    if (json == nullptr)
    {
        LOGGER_ERR("Failed to parse JSON string: nullptr string");
        return false;
    }

    // Count number of JSON tokens
    jsmn_init(&parser);
    ret = jsmn_parse(&parser, json, strlen(json), NULL, 0u);
    if (ret <= 0)
    {
        LOGGER_ERR("Failed to parse JSON string: no token found [json=%s ; token_count=%d]", json, ret);
        return false;
    }
    LOGGER_DEBUG("JSON string inspected [token_count=%d]", ret);
    token_count = static_cast<unsigned int>(ret);

    // Parse JSON string
    token = new jsmntok_t[token_count];
    jsmn_init(&parser);
    ret = jsmn_parse(&parser, json, strlen(json), token, token_count);
    if (static_cast<unsigned int>(ret) != token_count)
    {
        LOGGER_ERR("Failed to parse JSON string: unexpected token count [expected=%u ; actual=%d]", token_count, ret);
        goto err;
    }
    LOGGER_DEBUG("JSON string parsed [token_count=%d]", ret);

    // Format JSON tokens
    output.reserve(token_count);
    for (unsigned int i = 0; i < token_count; ++i)
    {
        struct json_entry entry;

        // Check and get type
        entry.type = jsmn_type_to_json_type(token[i].type);
        if (entry.type == JSON_UNDEFINED)
        {
            LOGGER_ERR("Failed to parse JSON token: unknown type [type=%d]", token[i].type);
            goto err;
        }

        // Check and get size
        if (token[i].size < 0)
        {
            LOGGER_ERR("Failed to parse JSON token: negative child count [count=%d]", token[i].size);
            goto err;
        }
        entry.size = static_cast<size_t>(token[i].size);

        // Check and get value
        if ((token[i].start < 0) || (token[i].end < 0) || (token[i].start > token[i].end))
        {
            LOGGER_ERR("Failed to parse JSON token: corrupted offset value [offset_start=%d ; offset_end=%d",
                       token[i].start, token[i].end);
            goto err;
        }
        entry.value = std::string(json, static_cast<unsigned int>(token[i].start), static_cast<unsigned int>(token[i].end - token[i].start));

        LOGGER_DEBUG("JSON token parsed [index=%d ; type=%s ; size=%d ; value=%s]",
                     i, json_type_to_string(entry.type), token[i].size, entry.value.c_str());

        output.push_back(std::move(entry));
    }

    delete[] token;
    return true;
err:
    delete[] token;
    output.clear();
    return false;
}
