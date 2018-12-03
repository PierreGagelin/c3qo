#! /bin/bash
# Test file to test non-regression of the compilation and of the test units

# No errors or undefined variables allowed
set -eu

# Get absolute path to this script
dir_script=$(cd $(dirname $0) > /dev/null && pwd)

source $dir_script/../c3qo_lib.sh

# Generate every useful paths from source path
c3qo_generate_path $dir_script/..

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

echo -e "$COLOR_BLUE\nExecute unit tests\n$COLOR_NO\n"
$C3QO_DIR_SOURCE/$C3QO_CMD_BUILD -t
$C3QO_DIR_SOURCE/integration/lib/ncli.py

echo -e "$COLOR_BLUE\nExecute functional tests\n$COLOR_NO\n"
PYTHONDONTWRITEBYTECODE=1 robot --output NONE --report NONE --log NONE $C3QO_DIR_INT/tf/

echo -e "$COLOR_BLUE\nMake a LCOV report\n$COLOR_NO\n"
$C3QO_DIR_SOURCE/$C3QO_CMD_BUILD -l
