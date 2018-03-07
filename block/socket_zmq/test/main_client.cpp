
// C++ library headers
#include <cstdlib> // NULL

// Project headers
#include "block/client_zmq_rr.hpp"
#include "utils/logger.hpp"

extern struct bk_if client_zmq_rr_if;

int main(int argc, char **argv)
{
    LOGGER_OPEN("tu_socket_zmq_rr");
    logger_set_level(LOGGER_LEVEL_DEBUG);

    void *ctx;

    ctx = client_zmq_rr_if.init(1);
    client_zmq_rr_if.start(ctx);
   

    return 0;
}