//
// @brief Test file for a block
//

// Project headers
#include "block/hook_zmq.hpp"
#include "engine/tu.hpp"

struct manager mgr_;

void message_create(std::vector<struct c3qo_zmq_part> &msg, const char *topic, const char *payload)
{
    struct c3qo_zmq_part part;

    part.data = strdup(topic);
    ASSERT(part.data != nullptr);
    part.len = strlen(part.data) + 1;
    msg.push_back(part);

    part.data = strdup(payload);
    ASSERT(part.data != nullptr);
    part.len = strlen(part.data) + 1;
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
// @brief Verify ZMQ_PAIR to ZMQ_PAIR communication
//
static void tu_hook_pair_pair()
{
    struct hook_zmq client(&mgr_);
    struct hook_zmq server(&mgr_);
    const char *address = "tcp://127.0.0.1:5555";
    std::vector<struct c3qo_zmq_part> msg;

    // Initialize two ZMQ pairs
    server.id_ = 1;
    client.id_ = 2;

    // Configure them
    server.type_ = ZMQ_PAIR;
    server.addr_ = std::string(address);
    server.client_ = false;

    client.type_ = ZMQ_PAIR;
    client.addr_ = std::string(address);
    client.client_ = true;

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

        client.data_(&msg);
        server.data_(&msg);

        // FIXME: this is ugly
        usleep(10 * 1000);

        mgr_.fd_poll();

        message_destroy(msg);
    }

    // At least one message should be received
    ASSERT(client.rx_pkt_ > 0lu);
    ASSERT(server.rx_pkt_ > 0lu);

    client.stop_();
    server.stop_();
}

//
// @brief Verify DEALER to ROUTER communication
//
static void tu_hook_dealer_router()
{
    struct hook_zmq client(&mgr_);
    struct hook_zmq server(&mgr_);
    const char *address = "tcp://127.0.0.1:5555";
    const char *dealer_name = "beef_is_good";

    // Initialize the blocks
    server.id_ = 3;
    client.id_ = 4;

    // Configure them
    server.type_ = ZMQ_ROUTER;
    server.addr_ = std::string(address);
    server.client_ = false;

    client.type_ = ZMQ_DEALER;
    client.name_ = std::string(dealer_name);
    client.addr_ = std::string(address);
    client.client_ = true;

    // Start them
    server.start_();
    client.start_();

    ASSERT(server.rx_pkt_ == 0lu);
    ASSERT(client.rx_pkt_ == 0lu);

    // Send some data between both
    // Some messages are lost because subscription can take some time
    for (int i = 0; i < 10; i++)
    {
        std::vector<struct c3qo_zmq_part> msg;

        message_create(msg, "question", "?");
        client.data_(&msg);
        message_destroy(msg);

        message_create(msg, dealer_name, "answer");
        server.data_(&msg);
        message_destroy(msg);

        // FIXME: this is ugly
        usleep(10 * 1000);

        mgr_.fd_poll();
    }

    // At least one message should be received
    ASSERT(client.rx_pkt_ > 0lu);
    ASSERT(server.rx_pkt_ > 0lu);

    client.stop_();
    server.stop_();
}

// Test error cases
static void tu_hook_zmq_error()
{
    struct hook_zmq block(&mgr_);

    // Errors are expected
    logger_set_level(LOGGER_LEVEL_CRIT);

    block.id_ = 1;

    block.data_(nullptr);

    // Connect failure
    block.addr_ = "well, obviously it's not an address";
    block.client_ = true;
    block.start_();
    block.stop_();

    // Bind failure
    block.client_ = false;
    block.start_();
    block.stop_();

    // Callback on unknown socket
    struct file_desc fd;
    fd.socket = nullptr;
    block.on_fd_(fd);

    logger_set_level(LOGGER_LEVEL_DEBUG);
}

int main(int, char **)
{
    LOGGER_OPEN("tu_hook_zmq");
    logger_set_level(LOGGER_LEVEL_DEBUG);

    tu_hook_pair_pair();
    tu_hook_dealer_router();
    tu_hook_zmq_error();

    LOGGER_CLOSE();
    return 0;
}