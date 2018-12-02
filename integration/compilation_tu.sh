#! /bin/bash
# Test file to test non-regression of the compilation and of the test units

# No errors or undefined variables allowed
set -eu

# Get absolute path to this script
dir_script=$(cd $(dirname $0) > /dev/null && pwd)

source $dir_script/../c3qo_lib.sh

# Generate every useful paths from source path
c3qo_generate_path $dir_script/..

echo "Clean build directory and do a CLASSIC build"
$C3QO_DIR_SOURCE/$C3QO_CMD_BUILD -cbt

echo "Clean build directory and do a RELEASE build"
$C3QO_DIR_SOURCE/$C3QO_CMD_BUILD -Rcbt

echo "Clean build directory and do a TESTLESS build"
$C3QO_DIR_SOURCE/$C3QO_CMD_BUILD -Tcb

echo "Make a package"
$C3QO_DIR_SOURCE/$C3QO_CMD_BUILD -p


echo "Clean build directory and do a GCOV build"
$C3QO_DIR_SOURCE/$C3QO_CMD_BUILD -Gcbt

echo "Execute functional tests"
$C3QO_DIR_INT/tf_socket_us_nb.sh -b $C3QO_DIR_C3QO -c $C3QO_DIR_INT
$C3QO_DIR_INT/tf_network_cli.sh -b $C3QO_DIR_C3QO -c $C3QO_DIR_INT

echo "Make a LCOV report"
$C3QO_DIR_SOURCE/$C3QO_CMD_BUILD -l

