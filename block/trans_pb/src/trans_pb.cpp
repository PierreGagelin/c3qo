

#define LOGGER_TAG "[block.trans_pb]"

// Project headers
#include "block/hello.hpp"
#include "block/trans_pb.hpp"
#include "block/zmq_pair.hpp"
#include "c3qo/manager.hpp"

trans_pb::trans_pb(struct manager *mgr) : block(mgr) {}
trans_pb::~trans_pb() {}

#ifdef C3QO_PROTOBUF

// Protobuf sources
#include "block.pb.h"

static void trans_pb_serialize(struct c3qo_zmq_msg &msg_zmq, class pb_msg_block &msg_bk) noexcept
{
    const char topic[] = "BLOCK.MSG";

    // Fill the topic (forget about the '\0' in ZeroMQ messages)
    msg_zmq.topic_len = sizeof(topic) - 1;
    msg_zmq.topic = new char[msg_zmq.topic_len];
    memcpy(msg_zmq.topic, topic, msg_zmq.topic_len);

    // Fill the data
    msg_zmq.data_len = static_cast<size_t>(msg_bk.ByteSize());
    msg_zmq.data = new char[msg_zmq.data_len];

    // Serialize the message
    bool ok = msg_bk.SerializeToArray(msg_zmq.data, msg_zmq.data_len);
    if (ok == false)
    {
        LOGGER_ERR("Failed to serialize hello message: protobuf error");
    }
}

static void trans_pb_serialize(struct c3qo_zmq_msg &msg_zmq, struct hello_ctx *ctx) noexcept
{
    class pb_msg_block msg_bk;
    class pb_msg_hello *hello;

    hello = new class pb_msg_hello;
    hello->set_bk_id(ctx->bk_id);

    msg_bk.set_type(pb_msg_block::MSG_HELLO);
    msg_bk.set_allocated_hello(hello);

    trans_pb_serialize(msg_zmq, msg_bk);
}

static void trans_pb_serialize(struct c3qo_zmq_msg &msg_zmq, struct zmq_pair_ctx *ctx) noexcept
{
    class pb_msg_block msg_bk;
    class pb_msg_zmq_pair *zmq_pair;

    zmq_pair = new class pb_msg_zmq_pair;
    zmq_pair->set_bk_id(ctx->bk_id);

    msg_bk.set_type(pb_msg_block::MSG_ZMQ_PAIR);
    msg_bk.set_allocated_zmq_pair(zmq_pair);

    trans_pb_serialize(msg_zmq, msg_bk);
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

#else

int trans_pb::ctrl_(void *) { return 0; }

#endif // C3QO_PROTOBUF

BLOCK_REGISTER(trans_pb);
