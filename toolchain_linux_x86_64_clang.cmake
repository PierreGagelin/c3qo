#
# Toolchain file for cross-compilation
# - Linux
# - 64 bits
# - clang / llvm
#

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_COMPILER /usr/bin/clang)
set(CMAKE_CXX_COMPILER /usr/bin/clang++)
