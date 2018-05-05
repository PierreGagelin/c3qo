//
// This block intends to give a platform to solve project euler problems
//
// https://projecteuler.net/
//

// Project headers
#include "block/project_euler.hpp"
#include "c3qo/manager.hpp"
#include "utils/logger.hpp"

// Local block header
#include "problem.hpp"

// Managers shall be linked
extern struct manager *m;

// Project Euler is a stateless block, it does not need to be initialized and so forth
static void *project_euler_init(int bk_id)
{
    (void)bk_id;
    return NULL;
}
static void project_euler_start(void *vctx)
{
    (void)vctx;
}
static void project_euler_stop(void *vctx)
{
    (void)vctx;
}

static void solve_problem(int index, char *param)
{
    switch (index)
    {
    case 1:
    {
        solve_problem_1(param);
    }
    break;

    default:
        LOGGER_ERR("Failed to solve problem: unknown problem number [number=%d]", index);
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
    if (token == NULL)
    {
        LOGGER_ERR("Failed to parse configuration line: no problem index found");
        return;
    }
    errno = 0;
    index = (int)strtol(token, NULL, 10);
    if (errno != 0)
    {
        LOGGER_ERR("Failed to parse configuration line: first argument has to be an integer");
        return;
    }

    // Retrieve argument (optional)
    arg = strtok(NULL, "\0");

    solve_problem(index, arg);
}

//
// @brief Exported structure of the block
//
struct bk_if project_euler_if = {
    .init = project_euler_init,
    .conf = project_euler_conf,
    .bind = NULL,
    .start = project_euler_start,
    .stop = project_euler_stop,

    .get_stats = NULL,

    .rx = NULL,
    .tx = NULL,
    .ctrl = NULL,
};
