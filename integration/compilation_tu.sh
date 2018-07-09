#! /bin/bash
# Test file to test non-regression of the compilation and of the test units


DIR_SOURCE=$(dirname $_)/..

CMD_BUILD=build.sh

CMD_TF_1=integration/TF_socket_retry/tf_socket_us_nb.sh
CMD_TF_2=integration/TF_socket_retry/tf_network_cli.sh

# No errors or undefined variables allowed
set -eu


echo "Clean build directory and do a CLASSIC build"
$DIR_SOURCE/$CMD_BUILD -cbt

echo "Clean build directory and do a RELEASE build"
$DIR_SOURCE/$CMD_BUILD -Rcbt

echo "Clean build directory and do a STATIC build"
$DIR_SOURCE/$CMD_BUILD -Scbt

echo "Clean build directory and do a TESTLESS build"
$DIR_SOURCE/$CMD_BUILD -Tcb

echo "Clean build directory and do a GCOV build"
$DIR_SOURCE/$CMD_BUILD -Gcbt

echo "Execute functional tests"
$DIR_SOURCE/$CMD_TF_1
$DIR_SOURCE/$CMD_TF_2

echo "Make a LCOV report"
$DIR_SOURCE/$CMD_BUILD -l

