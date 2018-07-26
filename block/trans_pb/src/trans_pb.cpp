
#include "block/trans_pb.hpp"

#ifdef C3QO_PROTOBUF

#include <cerrno>

// Project headers
#include "block/hello.hpp"
#include "block/zmq_pair.hpp"
#include "c3qo/manager.hpp"
#include "utils/logger.hpp"
#include "utils/socket.hpp"

// Protobuf sources
#include "block.pb.h"

// Managers shall be linked
extern struct manager *m;

static void *trans_pb_init(int bk_id)
{
    struct trans_pb_ctx *ctx;

    ctx = new(std::nothrow) struct trans_pb_ctx;
    if (ctx == nullptr)
    {
        LOGGER_ERR("Failed to initialize block: could not reserve memory for the context [bk_id=%d]", bk_id);
        return ctx;
    }

    ctx->bk_id = bk_id;

    return ctx;
}

static void trans_pb_stop(void *vctx)
{
    struct trans_pb_ctx *ctx;

    if (vctx == nullptr)
    {
        LOGGER_ERR("Failed to stop block: NULL context");
        return;
    }
    ctx = (struct trans_pb_ctx *)vctx;

    delete ctx;
}

static void trans_pb_serialize(struct c3qo_zmq_msg &msg_zmq, class pb_msg_block &msg_bk) noexcept
{
    const char *topic = "BLOCK.MSG";
    bool ok;

    // Fill the topic
    msg_zmq.topic_len = strlen(topic);
    msg_zmq.topic = new(std::nothrow) char[msg_zmq.topic_len];
    if (msg_zmq.topic == nullptr)
    {
        LOGGER_ERR("Failed to serialize message: %s [errno=%d]", strerror(errno), errno);
        return;
    }
    memcpy(msg_zmq.topic, topic, msg_zmq.topic_len);

    // Fill the data
    msg_zmq.data_len = msg_bk.ByteSizeLong();
    msg_zmq.data = new(std::nothrow) char[msg_zmq.data_len];
    if (msg_zmq.data == nullptr)
    {
        LOGGER_ERR("Failed to serialize message: %s [errno=%d]", strerror(errno), errno);
        delete msg_zmq.topic;
        return;
    }
    ok = msg_bk.SerializeToArray(msg_zmq.data, msg_zmq.data_len);
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

static int trans_pb_ctrl(void *vctx, void *vnotif)
{
    struct trans_pb_notif *notif;
    struct trans_pb_ctx *ctx;
    struct c3qo_zmq_msg msg_zmq;

    if ((vctx == nullptr) || (vnotif == nullptr))
    {
        LOGGER_ERR("trans_pb control failed: NULL argument [ctx=%p ; notif=%p]", vctx, vnotif);
        return 0;
    }
    notif = static_cast<struct trans_pb_notif *>(vnotif);
    ctx = static_cast<struct trans_pb_ctx *>(vctx);

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

    return 0;
}

//
// @brief Exported structure of the block
//
struct bk_if trans_pb_if = {
    .init = trans_pb_init,
    .conf = nullptr,
    .bind = nullptr,
    .start = nullptr,
    .stop = trans_pb_stop,

    .get_stats = nullptr,

    .rx = nullptr,
    .tx = nullptr,
    .ctrl = trans_pb_ctrl,
};

#else

struct bk_if trans_pb_if = {
    .init = nullptr,
    .conf = nullptr,
    .bind = nullptr,
    .start = nullptr,
    .stop = nullptr,

    .get_stats = nullptr,

    .rx = nullptr,
    .tx = nullptr,
    .ctrl = nullptr,
};

#endif // C3QO_PROTOBUF
