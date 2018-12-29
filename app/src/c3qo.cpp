

// Project headers
#include "block/hello.hpp"
#include "block/trans_pb.hpp"
#include "block/hook_zmq.hpp"
#include "engine/manager.hpp"

// C headers
extern "C"
{
#include <signal.h>
}

extern char *optarg; // Comes with getopt

bool end_signal_received = false;

static void signal_handler(int, siginfo_t *, void *)
{
    end_signal_received = true;
}

int main(int argc, char **argv)
{
    const char *options;
    const char *identity;

    options = "hi:";
    identity = "default_identity";
    for (int opt = getopt(argc, argv, options); opt != -1; opt = getopt(argc, argv, options))
    {
        switch (opt)
        {
        case 'h':
            printf("lol, help is for the weaks");
            return 1;

        case 'i':
            identity = optarg;
            break;

        default:
            return 1;
        }
    }

    LOGGER_OPEN(identity);

    // Register block factories
    struct manager mgr;
    struct hello_factory hello;
    struct trans_pb_factory trans_pb;
    struct hook_zmq_factory hook_zmq;

    mgr.block_factory_register("hello", &hello);
    mgr.block_factory_register("trans_pb", &trans_pb);
    mgr.block_factory_register("hook_zmq", &hook_zmq);

    // Add the ZMQ monitoring client
    struct hook_zmq *block;

    mgr.block_add(-1, "hook_zmq");

    block = static_cast<struct hook_zmq *>(mgr.block_get(-1));
    block->type_ = ZMQ_DEALER;
    block->name_ = std::string(identity);
    block->addr_ = std::string("tcp://127.0.0.1:1664");
    block->client_ = true;

    mgr.block_start(-1);

    // Add the protobuf transcoder
    mgr.block_add(-2, "trans_pb");
    mgr.block_start(-2);

    // Bind ZMQ and transcoder
    mgr.block_bind(-1, 0, -2);
    mgr.block_bind(-2, 0, -1);

    // Register a signal handler
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = signal_handler;

    int rc;
    rc = sigaction(SIGINT, &sa, NULL);
    ASSERT(rc != -1);
    rc = sigaction(SIGTERM, &sa, NULL);
    ASSERT(rc != -1);

    LOGGER_INFO("Registered signal handler for SIGINT and SIGTERM");

    // Main loop
    mgr.start_();
    while ((end_signal_received == false) && (mgr.is_term_ == false))
    {
        mgr.fd_poll();
        mgr.timer_check_exp();
    }

    LOGGER_CLOSE();

    return 0;
}
