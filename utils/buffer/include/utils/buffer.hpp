#ifndef BUFFER_HPP
#define BUFFER_HPP

// Project headers
#include "utils/include.hpp"

//
// @struct buffer_part
//
// @brief Buffer part, consist of a generic data array
//
struct buffer_part
{
    void *data;
    size_t len;
};

//
// @struct buffer
//
// @brief Buffer: a complete message, with several parts
//
struct buffer
{
    std::vector<struct buffer_part> parts_;

    void push_back(const void *data, size_t size);
    void clear();
};

#endif // BUFFER_HPP
