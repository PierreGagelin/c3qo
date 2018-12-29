

// Project headers
#include "engine/manager.hpp"

manager::manager() : is_term_(true) {}
manager::~manager()
{
    timer_clear();
    block_clear();
    block_factory_clear();
}

void manager::start_()
{
    is_term_ = false;
    LOGGER_INFO("Started manager");
}

void manager::stop_()
{
    is_term_ = true;
    LOGGER_INFO("Stopped manager");
}
