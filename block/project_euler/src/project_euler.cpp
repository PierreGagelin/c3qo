//
// This block intends to give a platform to solve project euler problems
//
// https://projecteuler.net/
//

// Project headers
#include "c3qo/manager.hpp"

#include "problem.hpp"

static void solve_problem(int index, char *param)
{
    switch (index)
    {
    case 1:
        solve_problem_1(param);
        break;
    case 2:
        solve_problem_2(param);
        break;
    case 3:
        solve_problem_3(param);
        break;
    case 51:
        solve_problem_51(param);
        break;
    default:
        LOGGER_ERR("Unknown problem index [index=%d]", index);
        break;
    }
}

//
// @brief Configure the block to execute a specific problem
//
static void project_euler_conf(void *vctx, char *conf)
{
    char *token;
    int index;
    char *arg;

    (void)vctx;

    LOGGER_DEBUG("Configuration for Project Euler received [conf=%s]", conf);

    // Retrieve problem number
    token = strtok(conf, " ");
    if (token == nullptr)
    {
        LOGGER_ERR("Failed to parse configuration line: no problem index found");
        return;
    }
    errno = 0;
    index = static_cast<int>(strtol(token, nullptr, 10));
    if (errno != 0)
    {
        LOGGER_ERR("Failed to parse configuration line: first argument has to be an integer");
        return;
    }

    // Retrieve argument (optional)
    arg = strtok(nullptr, "\0");

    solve_problem(index, arg);
}

//
// @brief Exported structure of the block
//
struct bk_if project_euler_if = {
    .init = nullptr,
    .conf = project_euler_conf,
    .bind = nullptr,
    .start = nullptr,
    .stop = nullptr,

    .get_stats = nullptr,

    .rx = nullptr,
    .tx = nullptr,
    .ctrl = nullptr,
};
