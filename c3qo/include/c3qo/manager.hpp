#ifndef C3QO_MANAGER_HPP
#define C3QO_MANAGER_HPP

#include "c3qo/manager_bk.hpp"
#include "c3qo/manager_fd.hpp"
#include "c3qo/manager_tm.hpp"

struct manager
{
    class manager_bk bk;
    class manager_fd fd;
    class manager_tm tm;
};

extern struct manager *m;

#endif // C3QO_MANAGER_HPP
