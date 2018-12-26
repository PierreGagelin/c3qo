//
// @brief API to manage socket
//

// Project headers
#include "utils/socket.hpp"
#include "utils/logger.hpp"

void c3qo_zmq_msg_del(std::vector<struct c3qo_zmq_part> &msg)
{
    for (const auto &it : msg)
    {
        delete[] it.data;
    }
    msg.clear();
}

//
// @brief Receive data from a ZMQ socket
//
void socket_zmq_read(void *socket, std::vector<struct c3qo_zmq_part> &msg, int flags)
{
    struct c3qo_zmq_part msg_part;

    while (true)
    {
        zmq_msg_t part;
        int ret;

        // Receive a message
        zmq_msg_init(&part);

        ret = zmq_msg_recv(&part, socket, flags);
        if (ret <= 0)
        {
            LOGGER_ERR("Failed to receive data from ZMQ socket: %s [errno=%d]", strerror(errno), errno);
            break;
        }

        // Copy the message
        // - add a NULL byte in case it's a string
        // - do not count it in the message length
        msg_part.len = zmq_msg_size(&part);
        msg_part.data = new char[msg_part.len + 1];
        memcpy(msg_part.data, zmq_msg_data(&part), msg_part.len);
        msg_part.data[msg_part.len] = '\0';

        msg.push_back(msg_part);

        // Look if there is another part to come
        ret = zmq_msg_more(&part);

        zmq_msg_close(&part);
        if (ret != 1)
        {
            break;
        }
    }
}

//
// @brief Send a ZMQ multi-part message
//
// @return True on success
//
bool socket_zmq_write(void *socket, std::vector<struct c3qo_zmq_part> &msg, int flags)
{
    flags |= ZMQ_SNDMORE;
    for (size_t idx = 0u; idx < msg.size(); ++idx)
    {
        zmq_msg_t message;
        int rc;

        zmq_msg_init_size(&message, msg[idx].len);
        memcpy(zmq_msg_data(&message), msg[idx].data, msg[idx].len);

        if (idx == msg.size() - 1)
        {
            flags &= ~ZMQ_SNDMORE;
        }
        rc = zmq_msg_send(&message, socket, flags);
        if (rc == -1)
        {
            LOGGER_ERR("Failed to write ZMQ message: %s [errno=%d ; len=%zu ; data=%s]",
                       strerror(errno), errno, msg[idx].len, msg[idx].data);
            zmq_msg_close(&message);
            return false;
        }
    }
    return true;
}
