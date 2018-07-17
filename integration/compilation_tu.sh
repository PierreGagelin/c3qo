#! /bin/bash
# Test file to test non-regression of the compilation and of the test units

# No errors or undefined variables allowed
set -eu

# Get absolute path of a directory
function get_abs_path()
{
    echo $(cd $1 > /dev/null && pwd)
}

# Get absolute path to this script
DIR_SCRIPT=$(cd $(dirname $0) > /dev/null && pwd)

DIR_SOURCE=$(get_abs_path $DIR_SCRIPT/..)
DIR_C3QO=$(get_abs_path $DIR_SOURCE/../build/c3qo)

CMD_BUILD=build.sh

echo "Clean build directory and do a CLASSIC build"
$DIR_SOURCE/$CMD_BUILD -cbt

echo "Clean build directory and do a RELEASE build"
$DIR_SOURCE/$CMD_BUILD -Rcbt

echo "Clean build directory and do a TESTLESS build"
$DIR_SOURCE/$CMD_BUILD -Tcb

echo "Clean build directory and do a GCOV build"
$DIR_SOURCE/$CMD_BUILD -Gcbt

echo "Execute functional tests"
$DIR_SCRIPT/tf_socket_us_nb.sh -b $DIR_C3QO -c $DIR_SCRIPT
$DIR_SCRIPT/tf_network_cli.sh -b $DIR_C3QO -c $DIR_SCRIPT

echo "Make a LCOV report"
$DIR_SOURCE/$CMD_BUILD -l

