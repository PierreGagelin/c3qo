//
// @brief Test file for a block
//

// Project headers
#include "c3qo/manager.hpp"

#include "gtest/gtest.h"

// Managers shall be linked
extern struct manager *m;

extern struct bk_if trans_pb_if;

class tu_trans_pb : public testing::Test
{
    void SetUp();
    void TearDown();
};

void tu_trans_pb::SetUp()
{
    LOGGER_OPEN("tu_trans_pb");
    logger_set_level(LOGGER_LEVEL_DEBUG);

    // Populate the managers
    m = new struct manager;
}

void tu_trans_pb::TearDown()
{
    // Clear the managers
    delete m;

    logger_set_level(LOGGER_LEVEL_NONE);
    LOGGER_CLOSE();
}

//
// @brief Test serialization of hello message
//
TEST_F(tu_trans_pb, hello)
{
    struct trans_pb_notif notif;
    struct hello_ctx hello;
    void *ctx;

    ctx = trans_pb_if.init(1);
    ASSERT_NE(ctx, nullptr);

    hello.bk_id = 42;
    notif.type = BLOCK_HELLO;
    notif.context.hello = &hello;

    EXPECT_EQ(trans_pb_if.ctrl(ctx, &notif), 0);

    trans_pb_if.stop(ctx);
}

//
// @brief Test serialization of zmq_pair message
//
TEST_F(tu_trans_pb, zmq_pair)
{
    struct trans_pb_notif notif;
    struct zmq_pair_ctx zmq_pair;
    void *ctx;

    ctx = trans_pb_if.init(1);
    ASSERT_NE(ctx, nullptr);

    zmq_pair.bk_id = 42;
    notif.type = BLOCK_ZMQ_PAIR;
    notif.context.zmq_pair = &zmq_pair;

    EXPECT_EQ(trans_pb_if.ctrl(ctx, &notif), 0);

    trans_pb_if.stop(ctx);
}

//
// @brief Test errors
//
TEST_F(tu_trans_pb, error)
{
    struct trans_pb_notif notif;
    void *ctx;

    // Ignore errors as these are nominal
    logger_set_level(LOGGER_LEVEL_EMERG);

    ctx = trans_pb_if.init(1);
    ASSERT_NE(ctx, nullptr);

    // Bad arguments
    EXPECT_EQ(trans_pb_if.ctrl(nullptr, nullptr), 0);
    EXPECT_EQ(trans_pb_if.ctrl(ctx, nullptr), 0);
    EXPECT_EQ(trans_pb_if.ctrl(nullptr, &notif), 0);

    // Bad notif type
    notif.type = static_cast<enum bk_type>(42);
    EXPECT_EQ(trans_pb_if.ctrl(ctx, &notif), 0);

    trans_pb_if.stop(ctx);
}
