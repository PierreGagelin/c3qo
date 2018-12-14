#!/usr/bin/env bash

#
# ad-hoc library for c3qo
#

# No errors or undefined variables allowed
set -eu

# Colors
COLOR_RED="\033[0;31m"
COLOR_GREEN="\033[0;32m"
COLOR_BLUE="\033[0;34m"
COLOR_NO="\033[0m"

#
# Generate absolute path to some useful directories
#   - $1 : path to the source folder of c3qo
#
function c3qo_generate_path()
{
    C3QO_DIR_SOURCE=$(readlink -f $1)
    C3QO_DIR_INT=$(readlink -f $C3QO_DIR_SOURCE/integration)
    C3QO_DIR_BUILD=$(readlink -f $C3QO_DIR_SOURCE/../build/c3qo)
    C3QO_DIR_LCOV=$(readlink -f $C3QO_DIR_BUILD/lcov)
}

# Name of the build command
C3QO_CMD_BUILD=build.sh
