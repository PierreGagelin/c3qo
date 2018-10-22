

#define LOGGER_TAG "[block.trans_pb]"

// Project headers
#include "block/hello.hpp"
#include "block/trans_pb.hpp"
#include "block/zmq_pair.hpp"
#include "c3qo/manager.hpp"

trans_pb::trans_pb(struct manager *mgr) : block(mgr) {}
trans_pb::~trans_pb() {}

// Protobuf sources
#include "block.pb-c.h"

static void trans_pb_serialize(struct c3qo_zmq_msg &msg_zmq, PbMsgBlock *msg_bk)
{
    const char topic[] = "BLOCK.MSG";

    // Fill the topic (with the '\0')
    msg_zmq.topic_len = sizeof(topic);
    msg_zmq.topic = new char[msg_zmq.topic_len];
    memcpy(msg_zmq.topic, topic, msg_zmq.topic_len);

    // Fill the data
    msg_zmq.data_len = pb_msg_block__get_packed_size(msg_bk);
    msg_zmq.data = new char[msg_zmq.data_len];
    size_t size = pb_msg_block__pack(msg_bk, reinterpret_cast<uint8_t *>(msg_zmq.data));
    if (size != msg_zmq.data_len)
    {
        LOGGER_ERR("Failed to serialize block message: protobuf-c error");
    }
}

static void trans_pb_serialize(struct c3qo_zmq_msg &msg_zmq, struct hello_ctx *ctx)
{
    PbMsgBlock msg_bk;
    PbMsgHello hello;

    pb_msg_block__init(&msg_bk);
    pb_msg_hello__init(&hello);

    hello.bk_id = static_cast<int32_t>(ctx->bk_id);

    msg_bk.type = PB_MSG_BLOCK__MSG_TYPE__MSG_HELLO;
    msg_bk.msg_block_case = PB_MSG_BLOCK__MSG_BLOCK_HELLO;
    msg_bk.hello = &hello;

    trans_pb_serialize(msg_zmq, &msg_bk);
}

static void trans_pb_serialize(struct c3qo_zmq_msg &msg_zmq, struct zmq_pair_ctx *ctx)
{
    PbMsgBlock msg_bk;
    PbMsgZmqPair zmq_pair;

    pb_msg_block__init(&msg_bk);
    pb_msg_zmq_pair__init(&zmq_pair);

    zmq_pair.bk_id = static_cast<int32_t>(ctx->bk_id);

    msg_bk.type = PB_MSG_BLOCK__MSG_TYPE__MSG_HELLO;
    msg_bk.msg_block_case = PB_MSG_BLOCK__MSG_BLOCK_HELLO;
    msg_bk.zmq_pair = &zmq_pair;

    trans_pb_serialize(msg_zmq, &msg_bk);
}

int trans_pb::ctrl_(void *vnotif)
{
    struct trans_pb_notif *notif;
    struct c3qo_zmq_msg msg_zmq;

    if (vnotif == nullptr)
    {
        LOGGER_ERR("trans_pb control failed: nullptr notif");
        return 0;
    }
    notif = static_cast<struct trans_pb_notif *>(vnotif);

    switch (notif->type)
    {
    case BLOCK_HELLO:
        trans_pb_serialize(msg_zmq, notif->context.hello);
        break;
    case BLOCK_ZMQ_PAIR:
        trans_pb_serialize(msg_zmq, notif->context.zmq_pair);
        break;
    default:
        LOGGER_ERR("trans_pb control failed: Unknown notification type [type=%d]", notif->type);
        return 0;
    }

    // Sending message to the ZMQ socket
    process_tx_(1, &msg_zmq);

    if (msg_zmq.topic != nullptr)
    {
        delete[] msg_zmq.topic;
    }
    if (msg_zmq.data != nullptr)
    {
        delete[] msg_zmq.data;
    }

    return 0;
}

BLOCK_REGISTER(trans_pb);
