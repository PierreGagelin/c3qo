

#ifndef C3QO_JSON_H
#define C3QO_JSON_H

#include "utils/include.hpp"

enum json_type
{
    JSON_UNDEFINED,
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_STRING,
    JSON_PRIMITIVE
};
const char *json_type_to_string(enum json_type type);

struct json_entry
{
    enum json_type type; // JSON type
    size_t size;         // Count of childs
    std::string value;   // JSON value
};

bool json_parse(const char *json, std::vector<struct json_entry> &token);

#endif // C3QO_JSON_H
