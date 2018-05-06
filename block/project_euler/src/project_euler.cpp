//
// This block intends to give a platform to solve project euler problems
//
// https://projecteuler.net/
//

// System library headers
extern "C" {
#include <dlfcn.h> // dlopen, dlsym, dlerror
}

// Project headers
#include "block/project_euler.hpp"
#include "c3qo/manager.hpp"
#include "utils/logger.hpp"

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
    void *self;
    char solver_name[32];
    void (*solver)(char *);

    self = dlopen(NULL, RTLD_LAZY);
    if (self == NULL)
    {
        LOGGER_ERR("Failed to open ourselves to look for symbols: %s", dlerror());
        return;
    }

    sprintf(solver_name, "solve_problem_%d", index);
    solver = (void (*)(char *))dlsym(self, solver_name);
    if (solver == NULL)
    {
        LOGGER_ERR("Failed to find solver function: %s [func=%s]", dlerror(), solver_name);
        return;
    }

    solver(param);
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
