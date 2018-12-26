#ifndef NCLI_HPP
#define NCLI_HPP

#include "engine/block.hpp"

struct ncli : public block
{
    explicit ncli(struct manager *mgr);
    virtual ~ncli() override final;

    virtual void start_() override final;
    virtual void stop_() override final;

    virtual void on_fd_(struct file_desc &fd) override final;
    virtual void on_timer_(struct timer &tm) override final;

    // ZeroMQ contexts
    void *zmq_ctx_;
    struct file_desc zmq_sock_;

    // Command and identity of the peer to configure
    char *ncli_args_;
    char *ncli_peer_;

    // Status of the command
    bool received_answer_;
    bool timeout_expired_;
};

#endif // NCLI_HPP
