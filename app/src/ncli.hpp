#ifndef NCLI_HPP
#define NCLI_HPP

#include "engine/block.hpp"

struct ncli : public block
{
    // ZeroMQ contexts
    void *zmq_ctx_;
    struct file_desc zmq_sock_;

    //
    // Command configuration
    //
    char *ncli_peer_;     // Identity of the peer to configure
    char *ncli_cmd_type_; // Command type
    char *ncli_cmd_args_; // Command arguments
    char *ncli_cmd_ret_;  // Command expected return

    wordexp_t wordexp_;
    bool options_parse(int argc, char **argv);
    void options_clear();

    //
    // Protobuf command configuration
    //
    Command cmd_;

    BlockAdd bk_add_;
    int32_t add_id_;
    char *add_type_;
    bool parse_add(int argc, char **argv);

    BlockStart bk_start_;
    int32_t start_id_;
    bool parse_start(int argc, char **argv);

    BlockStop bk_stop_;
    int32_t stop_id_;
    bool parse_stop(int argc, char **argv);

    BlockDel bk_del_;
    int32_t del_id_;
    bool parse_del(int argc, char **argv);

    BlockBind bk_bind_;
    int32_t bind_id_;
    int32_t bind_port_;
    int32_t bind_dest_;
    bool parse_bind(int argc, char **argv);

    ConfHookZmq conf_hook_zmq_;
    int32_t hook_zmq_id_;
    bool hook_zmq_client_;
    int32_t hook_zmq_type_;
    char *hook_zmq_name_;
    char *hook_zmq_addr_;
    bool parse_hook_zmq(int argc, char **argv);

    bool parse_term(int argc, char **argv);

    //
    // Status of the ZeroMQ exchange with the peer
    //
    bool received_answer_;
    bool timeout_expired_;

    //
    // Implementation of the block interface
    //
    explicit ncli(struct manager *mgr);
    virtual ~ncli() override final;

    virtual void start_() override final;
    virtual void stop_() override final;

    virtual bool data_(void *vdata) override final;

    virtual void on_timer_(struct timer &tm) override final;
};

struct ncli_factory : block_factory
{
    virtual struct block *constructor(struct manager *mgr) override final;
    virtual void destructor(struct block *bk) override final;
};

#endif // NCLI_HPP
