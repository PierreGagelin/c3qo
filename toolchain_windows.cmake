#
# Toolchain file for cross-compilation
# - Windows
# - 64 bits
# - mingw
#

set(CMAKE_SYSTEM_NAME Windows)

set(prefix x86_64-w64-mingw32)
set(CMAKE_C_COMPILER ${prefix}-gcc)
set(CMAKE_CXX_COMPILER ${prefix}-g++)
set(CMAKE_RC_COMPILER ${prefix}-windres)
