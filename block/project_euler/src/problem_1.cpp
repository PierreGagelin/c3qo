
// Project headers
#include "problem.hpp"

void solve_problem_1(char *param)
{
    unsigned long sum;
    unsigned long max;

    LOGGER_DEBUG("Solving problem 1 [param=%s]", param);

    // Retrieve argument
    errno = 0;
    max = strtoul(param, NULL, 10);
    if (errno != 0)
    {
        LOGGER_ERR("Failed to parse configuration line: first argument has to be an integer");
        return;
    }

    sum = 0;
    for (unsigned long i = 1; i < max; i++)
    {
        if (((i % 5) == 0) || ((i % 3) == 0))
        {
            sum += i;
        }
    }

    LOGGER_INFO("Solved problem 1 [max=%lu ; sum=%lu]", max, sum);
}
