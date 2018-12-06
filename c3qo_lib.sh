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


# Get absolute path of a directory
#   - $1 : directory to get an absolute path from
function get_abs_path()
{
    local dir=
    local ret=

    dir=$1

    # Temporary creation of the directory in case it does not exist
    mkdir -p $dir

    # Get an absolute path to the directory
    ret=$(cd $dir > /dev/null && pwd)
    dir=$ret

    # Remove potentially created (empty) directories
    while [ -z "$(ls -A $dir)" ]
    do
        rmdir $dir
        dir=$(dirname $dir)
    done

    echo $ret
}


# Generate absolute path to some useful directories
#   - $1 : path to the source folder of c3qo
function c3qo_generate_path()
{
    C3QO_DIR_SOURCE=$(get_abs_path $1)
    C3QO_DIR_INT=$(get_abs_path $C3QO_DIR_SOURCE/integration)
    C3QO_DIR_BUILD=$(get_abs_path $C3QO_DIR_SOURCE/../build/c3qo)
    C3QO_DIR_LCOV=$(get_abs_path $C3QO_DIR_BUILD/lcov)
}

# Name of the build command
C3QO_CMD_BUILD=build.sh
