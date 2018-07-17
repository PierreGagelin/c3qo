#!/bin/bash
# ad-hoc library for c3qo


# No errors or undefined variables allowed
set -eu


# Get absolute path of a directory
#   - $1 : directory to get an absolute path from
function get_abs_path()
{
    local dir=$1

    # Temporary creation of the directory in case it does not exist
    mkdir -p $dir

    # Get an absolute path to the directory
    local ret=$(cd $dir > /dev/null && pwd)

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
    C3QO_DIR_BUILD=$(get_abs_path $C3QO_DIR_SOURCE/../build)
    C3QO_DIR_LCOV=$(get_abs_path $C3QO_DIR_BUILD/lcov)
    C3QO_DIR_C3QO=$(get_abs_path $C3QO_DIR_BUILD/c3qo)
}

# Name of the build command
C3QO_CMD_BUILD=build.sh
