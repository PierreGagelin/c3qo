//
// @brief API to manage buffers
//

// Project headers
#include "utils/buffer.hpp"

//
// Add a part to the buffer
// A null byte is added but is not counted in the length
//
void buffer::push_back(const void *data, size_t size)
{
    struct buffer_part part;

    part.len = size;
    part.data = new char[size + 1];
    memcpy(part.data, data, size);
    static_cast<char *>(part.data)[size] = '\0';

    parts_.push_back(part);
}

void buffer::clear()
{
    for (const auto &it : parts_)
    {
        delete[] static_cast<char *>(it.data);
    }

    parts_.clear();
}
