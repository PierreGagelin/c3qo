

// Local header
#include "problem.hpp"

void solve_problem_2(char *param)
{
    unsigned long long sum;
    int current;
    int max;
    int fib1;
    int fib2;

    LOGGER_DEBUG("Solving problem 2 [param=%s]", param);

    // Retrieve argument
    errno = 0;
    max = (int)strtol(param, NULL, 10);
    if (errno != 0)
    {
        LOGGER_ERR("Failed to parse configuration line: first argument has to be an integer");
        return;
    }

    // Initialize fibonacci
    sum = 0;
    fib1 = 1;
    fib2 = 1;
    current = fib1 + fib2;
    while (current <= max)
    {
        if ((current % 2) == 0)
        {
            sum += current;
        }

        fib1 = fib2;
        fib2 = current;

        current = fib1 + fib2;
    }

    LOGGER_INFO("Solved problem 2 [max=%d ; sum=%llu]", max, sum);
}