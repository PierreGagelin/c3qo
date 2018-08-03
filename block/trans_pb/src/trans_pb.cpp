

// Project headers
#include "c3qo/manager.hpp"

// Managers shall be linked
extern struct manager *m;

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
    msg_zmq.data_len = msg_bk.ByteSizeLong();
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

void bk_trans_pb::init_()
{
    struct trans_pb_ctx *ctx;

    ctx = new struct trans_pb_ctx;

    ctx->bk_id = id_;

    ctx_ = ctx;
}

void bk_trans_pb::stop_()
{
    struct trans_pb_ctx *ctx;

    if (ctx_ == nullptr)
    {
        LOGGER_ERR("Failed to stop block: nullptr context");
        return;
    }
    ctx = static_cast<struct trans_pb_ctx *>(ctx_);

    delete ctx;
    ctx_ = nullptr;
}

int bk_trans_pb::ctrl_(void *vnotif)
{
    struct trans_pb_notif *notif;
    struct trans_pb_ctx *ctx;
    struct c3qo_zmq_msg msg_zmq;

    if ((ctx_ == nullptr) || (vnotif == nullptr))
    {
        LOGGER_ERR("trans_pb control failed: nullptr argument [ctx=%p ; notif=%p]", ctx_, vnotif);
        return 0;
    }
    notif = static_cast<struct trans_pb_notif *>(vnotif);
    ctx = static_cast<struct trans_pb_ctx *>(ctx_);

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
    m->bk.process_tx(ctx->bk_id, 1, &msg_zmq);

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

#endif // C3QO_PROTOBUF
