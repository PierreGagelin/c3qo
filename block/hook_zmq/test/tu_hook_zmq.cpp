//
// @brief Test file for a block
//

// Project headers
#include "block/hook_zmq.hpp"
#include "engine/tu.hpp"

struct manager mgr_;

void message_create(struct buffer &buf, const char *topic, const char *payload)
{
    buf.push_back(topic, strlen(topic) + 1);
    buf.push_back(payload, strlen(payload) + 1);
}

void message_destroy(struct buffer &buf)
{
    buf.clear();
}

//
// @brief Verify ZMQ_PAIR to ZMQ_PAIR communication
//
static void tu_hook_pair_pair()
{
    struct hook_zmq client(&mgr_);
    struct hook_zmq server(&mgr_);
    const char *address = "tcp://127.0.0.1:5555";
    struct buffer buf;

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
        message_create(buf, "hello", "world");

        client.data_(&buf);
        server.data_(&buf);

        // FIXME: this is ugly
        usleep(10 * 1000);

        mgr_.fd_poll();

        message_destroy(buf);
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
        struct buffer buf;

        message_create(buf, "question", "?");
        client.data_(&buf);
        message_destroy(buf);

        message_create(buf, dealer_name, "answer");
        server.data_(&buf);
        message_destroy(buf);

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

    // Socket creation failure
    block.type_ = 42;
    block.start_();
    ASSERT(block.zmq_sock_.socket == nullptr);
    block.stop_();
    block.type_ = 0;

    // Socket identity failure
    block.name_ = "more than 255 characters: ";
    for (int i = 0; i < 300; ++i)
    {
        block.name_ += "a";
    }
    block.start_();
    ASSERT(block.zmq_sock_.socket != nullptr);
    block.stop_();
    block.zmq_sock_.socket = nullptr;
    block.name_ = "";

    // Connect failure
    block.addr_ = "well, obviously it's not an address";
    block.client_ = true;
    block.start_();
    ASSERT(block.zmq_sock_.socket != nullptr);
    block.stop_();
    block.zmq_sock_.socket = nullptr;

    // Bind failure
    block.type_ = 0;
    block.addr_ = "well, obviously it's not an address";
    block.client_ = false;
    block.start_();
    ASSERT(block.zmq_sock_.socket != nullptr);
    block.stop_();
    block.zmq_sock_.socket = nullptr;

    // Send and receive failure
    struct buffer buf;
    buf.push_back("dummy", strlen("dummy"));
    ASSERT(block.send_(buf) == false);
    ASSERT(block.recv_(buf) == false);
    buf.clear();

    // Callback on unknown socket
    struct file_desc fd;
    fd.socket = nullptr;
    block.start_();
    ASSERT(block.zmq_sock_.socket != nullptr);
    block.on_fd_(fd);
    block.stop_();

    // Data failure
    block.data_(nullptr);
}

int main(int, char **)
{
    LOGGER_OPEN("tu_hook_zmq");

    tu_hook_pair_pair();
    tu_hook_dealer_router();
    tu_hook_zmq_error();

    LOGGER_CLOSE();
    return 0;
}
