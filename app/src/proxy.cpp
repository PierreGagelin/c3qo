

// Project headers
#include "block/hook_zmq.hpp"
#include "engine/manager.hpp"

// C headers
extern "C"
{
#include <signal.h>
}

static void signal_handler(int, siginfo_t *, void *) {}

//
// Starts a proxy
// - frontend is to connect several DEALER
// - backend is to pilot these apps
//
int main(int, char **)
{
    struct manager mgr;

    LOGGER_OPEN("proxy");
    logger_set_level(LOGGER_LEVEL_DEBUG);

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

    struct hook_zmq frontend(&mgr);
    frontend.type_ = ZMQ_ROUTER;
    frontend.client_ = false;
    frontend.addr_ = std::string("tcp://127.0.0.1:1664");
    frontend.start_();

    struct hook_zmq backend(&mgr);
    backend.type_ = ZMQ_PAIR;
    backend.client_ = false;
    backend.addr_ = std::string("tcp://127.0.0.1:1665");
    backend.start_();

    zmq_proxy(frontend.zmq_sock_.socket, backend.zmq_sock_.socket, nullptr);

    LOGGER_INFO("Proxy has stopped: end of program");

    LOGGER_CLOSE();

    return 0;
}
