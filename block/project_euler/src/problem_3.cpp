

// Local header
#include "problem.hpp"

unsigned long largest_prime_factor(unsigned long n)
{
    unsigned long i = 2;
    unsigned long largest = 2;

    while (i * i <= n)
    {
        while (n % i == 0)
        {
            n = n / i;
            largest = i;
        }

        i += (i >= 3) ? 2 : 1;
    }

    return (n > 1) ? n : largest;
}



void solve_problem_3(char *param)
{
    unsigned long max;

    LOGGER_DEBUG("Solving problem 3 [param=%s]", param);

    // Retrieve argument
    errno = 0;
    max = strtoul(param, NULL, 10);
    if (errno != 0)
    {
        LOGGER_ERR("Failed to parse configuration line: first argument has to be an integer");
        return;
    }

    unsigned long answer = largest_prime_factor(max);

    LOGGER_INFO("Solved problem 1 [max=%lu ; sum=%lu]", max, answer);
}
