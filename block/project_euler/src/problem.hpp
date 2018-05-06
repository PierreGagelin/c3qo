

// C++ library headers
#include <cerrno>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>

// Project headers
#include "utils/logger.hpp"

// We need those to have a demangled name to be loaded at run-time
extern "C" {
void solve_problem_1(char *param);
void solve_problem_2(char *param);
void solve_problem_51(char *param);
}
