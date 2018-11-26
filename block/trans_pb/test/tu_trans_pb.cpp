//
// @brief Test file for a block
//

#define LOGGER_TAG "[TU.trans_pb]"

// Project headers
#include "block/hello.hpp"
#include "block/trans_pb.hpp"
#include "block/zmq_pair.hpp"
#include "c3qo/tu.hpp"

struct manager mgr_;

//
// @brief Test serialization of hello message
//
static void tu_trans_pb_hello()
{
    struct trans_pb block(&mgr_);
    struct trans_pb_notif notif;
    struct hello_ctx hello;

    block.id_ = 1;

    hello.bk_id = 42;
    notif.type = BLOCK_HELLO;
    notif.context.hello = &hello;

    ASSERT(block.ctrl_(&notif) == 0);
}

//
// @brief Test serialization of zmq_pair message
//
static void tu_trans_pb_zmq_pair()
{
    struct trans_pb block(&mgr_);
    struct trans_pb_notif notif;
    struct zmq_pair_ctx zmq_pair;

    block.id_ = 1;

    zmq_pair.bk_id = 42;
    notif.type = BLOCK_ZMQ_PAIR;
    notif.context.zmq_pair = &zmq_pair;

    ASSERT(block.ctrl_(&notif) == 0);
}

//
// @brief Test errors
//
static void tu_trans_pb_error()
{
    struct trans_pb block(&mgr_);
    struct trans_pb_notif notif;

    // Ignore errors as these are nominal
    logger_set_level(LOGGER_LEVEL_EMERG);

    block.id_ = 1;

    // Bad arguments
    ASSERT(block.ctrl_(nullptr) == 0);

    // Bad notif type
    notif.type = static_cast<enum bk_type>(42);
    ASSERT(block.ctrl_(&notif) == 0);
}

int main(int, char **)
{
    LOGGER_OPEN("tu_trans_pb");
    logger_set_level(LOGGER_LEVEL_DEBUG);

    tu_trans_pb_error();
    tu_trans_pb_hello();
    tu_trans_pb_zmq_pair();

    LOGGER_CLOSE();
    return 0;
}
