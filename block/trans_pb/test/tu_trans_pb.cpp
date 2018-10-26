//
// @brief Test file for a block
//

#define LOGGER_TAG "[TU.trans_pb]"

// Project headers
#include "block/hello.hpp"
#include "block/trans_pb.hpp"
#include "block/zmq_pair.hpp"
#include "c3qo/tu.hpp"

class tu_trans_pb : public testing::Test
{
    void SetUp();
    void TearDown();

  public:
    struct manager mgr_;
};

void tu_trans_pb::SetUp()
{
    LOGGER_OPEN("tu_trans_pb");
    logger_set_level(LOGGER_LEVEL_DEBUG);
}

void tu_trans_pb::TearDown()
{
    logger_set_level(LOGGER_LEVEL_NONE);
    LOGGER_CLOSE();
}

//
// @brief Test serialization of hello message
//
TEST_F(tu_trans_pb, hello)
{
    struct trans_pb block(&mgr_);
    struct trans_pb_notif notif;
    struct hello_ctx hello;

    block.id_ = 1;

    hello.bk_id = 42;
    notif.type = BLOCK_HELLO;
    notif.context.hello = &hello;

    EXPECT_EQ(block.ctrl_(&notif), 0);
}

//
// @brief Test serialization of zmq_pair message
//
TEST_F(tu_trans_pb, zmq_pair)
{
    struct trans_pb block(&mgr_);
    struct trans_pb_notif notif;
    struct zmq_pair_ctx zmq_pair;

    block.id_ = 1;

    zmq_pair.bk_id = 42;
    notif.type = BLOCK_ZMQ_PAIR;
    notif.context.zmq_pair = &zmq_pair;

    EXPECT_EQ(block.ctrl_(&notif), 0);
}

//
// @brief Test errors
//
TEST_F(tu_trans_pb, error)
{
    struct trans_pb block(&mgr_);
    struct trans_pb_notif notif;

    // Ignore errors as these are nominal
    logger_set_level(LOGGER_LEVEL_EMERG);

    block.id_ = 1;

    // Bad arguments
    EXPECT_EQ(block.ctrl_(nullptr), 0);

    // Bad notif type
    notif.type = static_cast<enum bk_type>(42);
    EXPECT_EQ(block.ctrl_(&notif), 0);
}
