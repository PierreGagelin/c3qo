

#define LOGGER_TAG "[block.project_euler]"

// Local header
#include "problem.hpp"

// Add a prime number to the vector
static inline void add_prime(std::vector<uint32_t> &prime_vec)
{
    uint32_t prime;

    prime = prime_vec.back() + 1;

    while (true)
    {
        auto it = prime_vec.begin();
        auto end = prime_vec.end();
        bool is_prime = true;
        uint32_t root;

        root = static_cast<uint32_t>(sqrt((double)prime));

        while ((*it <= root) && (it != end))
        {
            if ((prime % *it) == 0)
            {
                prime++;
                is_prime = false;
                break;
            }

            ++it;
        }

        if (is_prime)
        {
            prime_vec.push_back(prime);
            break;
        }
    }
}

// Add prime number to vector until a new digit appear, after each call the
// vector contains a new range of prime numbers
// e.g.:
//   - 1st call: 2, 3, 5, 7, 11, 13, 17, 19, ..., 97
//   - 2nd call: 2, 3, ..., 997
static inline void add_prime_digit(std::vector<uint32_t> &prime_vec)
{
    int digit;
    uint32_t prime;
    uint32_t power;

    prime = prime_vec.back();
    for (digit = 1; digit != 10; digit++)
    {
        power = static_cast<uint32_t>(pow((double)10, (double)digit));
        if ((prime / power) == 0)
        {
            break;
        }
    }

    if (digit == 10)
    {
        LOGGER_WARNING("Overflow will occur: need to upgrade prime storage to 64 bits [prime=%u]", prime)
    }

    digit++;
    power = static_cast<uint32_t>(pow((double)10, (double)digit));

    while ((prime_vec.back() / power) == 0)
    {
        add_prime(prime_vec);
    }
    prime_vec.pop_back();
}

// Check problem 51 solution from pos element to the last
static inline void check_replacement(std::vector<uint32_t> &prime_vec, size_t first)
{
    size_t last;

    last = prime_vec.size() - 1;

    LOGGER_DEBUG("Check replacement in range [first_pos=%zu ; last_pos=%zu]", first, last)
}

void solve_problem_51(char *param)
{
    std::vector<uint32_t> prime_vec;

    (void)param;

    prime_vec.push_back(2u);
    prime_vec.push_back(3u);
    prime_vec.push_back(5u);
    prime_vec.push_back(7u);
    for (int i = 0; i < 3; i++)
    {
        size_t pos;

        pos = prime_vec.size();

        add_prime_digit(prime_vec);
        LOGGER_DEBUG("Added new digit range [last_prime=%u]", prime_vec.back());

        check_replacement(prime_vec, pos);
    }
}
