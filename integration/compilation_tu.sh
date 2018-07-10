#! /bin/bash
# Test file to test non-regression of the compilation and of the test units

# No errors or undefined variables allowed
set -eux


SCRIPT=$0
DIR_CURRENT=$(pwd)
DIR_SCRIPT=$(dirname $DIR_CURRENT/$SCRIPT)
DIR_SOURCE=$DIR_SCRIPT/..
DIR_BUILD=$DIR_SOURCE/../build
DIR_C3QO=$DIR_BUILD/c3qo
DIR_TEST=$DIR_SOURCE/integration/TF_socket_retry

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
$DIR_TEST/tf_socket_us_nb.sh -b $DIR_C3QO -c $DIR_TEST
$DIR_TEST/tf_network_cli.sh -b $DIR_C3QO -c $DIR_TEST

echo "Make a LCOV report"
$DIR_SOURCE/$CMD_BUILD -l

