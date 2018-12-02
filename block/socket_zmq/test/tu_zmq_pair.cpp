//
// @brief Test file for a block
//

#define LOGGER_TAG "[TU.zmq_pair]"

// Project headers
#include "block/zmq_pair.hpp"
#include "engine/tu.hpp"

struct manager mgr_;

void message_create(std::vector<struct c3qo_zmq_part> &msg, const char *topic, const char *payload)
{
    struct c3qo_zmq_part part;

    part.data = strdup(topic);
    ASSERT(part.data != nullptr);
    part.len = strlen(part.data);
    msg.push_back(part);

    part.data = strdup(payload);
    ASSERT(part.data != nullptr);
    part.len = strlen(part.data);
    msg.push_back(part);
}

void message_destroy(std::vector<struct c3qo_zmq_part> &msg)
{
    for (const auto &it : msg)
    {
        free(it.data);
    }
    msg.clear();
}

//
// @brief Verify data transmission between client and server
//
static void tu_zmq_pair_data()
{
    struct zmq_pair client(&mgr_);
    struct zmq_pair server(&mgr_);
    char conf_s[] = "type=server addr=tcp://127.0.0.1:5555";
    char conf_c[] = "type=client addr=tcp://127.0.0.1:5555";
    std::vector<struct c3qo_zmq_part> msg;

    // Initialize two ZMQ pairs
    server.id_ = 1;
    client.id_ = 2;

    // Configure them
    server.conf_(conf_s);
    client.conf_(conf_c);

    // Start them
    server.start_();
    client.start_();

    ASSERT(server.rx_pkt_ == 0lu);
    ASSERT(client.rx_pkt_ == 0lu);

    // Send some data between both pairs
    // Some messages are lost because subscription can take some time
    for (int i = 0; i < 10; i++)
    {
        message_create(msg, "hello", "world");

        client.tx_(&msg);
        server.tx_(&msg);

        mgr_.fd_poll();

        message_destroy(msg);
    }

    // At least one message should be received
    ASSERT(client.rx_pkt_ > 0lu);
    ASSERT(server.rx_pkt_ > 0lu);

    client.stop_();
    server.stop_();
}

// Test error cases
static void tu_zmq_pair_error()
{
    struct zmq_pair block(&mgr_);

    // Errors are expected
    logger_set_level(LOGGER_LEVEL_CRIT);

    block.id_ = 1;

    block.conf_(nullptr);

    // Type error
    block.conf_(const_cast<char *>("hello"));
    block.conf_(const_cast<char *>("type="));
    block.conf_(const_cast<char *>("type= "));
    block.conf_(const_cast<char *>("type=banana "));

    // Address error
    block.conf_(const_cast<char *>("type=client no address"));
    block.conf_(const_cast<char *>("type=client addr="));
    block.conf_(const_cast<char *>("type=client addr= "));

    block.tx_(nullptr);

    // Connect failure
    block.addr_ = "well, obviously it's not an address";
    block.client_ = true;
    block.start_();
    block.client_ = false;
    block.start_();

    // Callback on unknown socket
    struct file_desc fd;
    fd.socket = nullptr;
    block.on_fd_(fd);

    logger_set_level(LOGGER_LEVEL_DEBUG);
}

int main(int, char **)
{
    LOGGER_OPEN("tu_zmq_pair");
    logger_set_level(LOGGER_LEVEL_DEBUG);

    tu_zmq_pair_data();
    tu_zmq_pair_error();

    LOGGER_CLOSE();
    return 0;
}
