#!/usr/bin/env bash

#
# Test file of non-regression :
# - compilation
# - test units (TU)
# - test functions (TF)
# - coverage
#

# No errors or undefined variables allowed
set -eu

# Get absolute path to this script
DIR_SCRIPT=$(dirname $(readlink -f $0))

source $DIR_SCRIPT/../c3qo_lib.sh

# Generate every useful paths from source path
c3qo_generate_path $DIR_SCRIPT/..

#
# Compilation
#
echo -e "$COLOR_BLUE\nCLASSIC build\n$COLOR_NO\n"
$C3QO_DIR_SOURCE/$C3QO_CMD_BUILD -Tbt

echo -e "$COLOR_BLUE\nRELEASE build\n$COLOR_NO\n"
$C3QO_DIR_SOURCE/$C3QO_CMD_BUILD -B Release -Tbt

echo -e "$COLOR_BLUE\nTESTLESS build\n$COLOR_NO\n"
$C3QO_DIR_SOURCE/$C3QO_CMD_BUILD -b

echo -e "$COLOR_BLUE\nMake a package\n$COLOR_NO\n"
$C3QO_DIR_SOURCE/$C3QO_CMD_BUILD -p

echo -e "$COLOR_BLUE\nGCOV build\n$COLOR_NO\n"
$C3QO_DIR_SOURCE/$C3QO_CMD_BUILD -GTbp

#
# Test Units
#
echo -e "$COLOR_BLUE\nExecute unit tests\n$COLOR_NO\n"
$C3QO_DIR_SOURCE/$C3QO_CMD_BUILD -t

#
# Test Functions
#
echo -e "$COLOR_BLUE\nExecute functional tests\n$COLOR_NO\n"
PYTHONDONTWRITEBYTECODE=1 robot --output NONE --report NONE --log NONE $C3QO_DIR_INT/tf/

#
# Coverage
#
echo -e "$COLOR_BLUE\nMake a LCOV report\n$COLOR_NO\n"
$C3QO_DIR_SOURCE/$C3QO_CMD_BUILD -l
