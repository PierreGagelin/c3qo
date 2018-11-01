#! /bin/bash
# Test file to test non-regression of the compilation and of the test units

# No errors or undefined variables allowed
set -eu

# Get absolute path to this script
dir_script=$(cd $(dirname $0) > /dev/null && pwd)

source $dir_script/../c3qo_lib.sh

# Generate every useful paths from source path
c3qo_generate_path $dir_script/..

# echo -e "$COLOR_BLUE\n    CLASSIC build $COLOR_NO\n"
# $C3QO_DIR_SOURCE/$C3QO_CMD_BUILD -Tbt

# echo -e "$COLOR_BLUE\n    RELEASE build $COLOR_NO\n"
# $C3QO_DIR_SOURCE/$C3QO_CMD_BUILD -B Release -Tbt

# echo -e "$COLOR_BLUE\n    TESTLESS build $COLOR_NO\n"
# $C3QO_DIR_SOURCE/$C3QO_CMD_BUILD -b

# echo -e "$COLOR_BLUE\n    Make a package $COLOR_NO\n"
# $C3QO_DIR_SOURCE/$C3QO_CMD_BUILD -p

echo -e "$COLOR_BLUE\n    GCOV build $COLOR_NO\n"
$C3QO_DIR_SOURCE/$C3QO_CMD_BUILD -GTbt

echo -e "$COLOR_BLUE\n    Execute functional tests $COLOR_NO\n"
echo -e "\nExecute functional tests"
$C3QO_DIR_INT/tf_socket_us_nb.sh
$C3QO_DIR_INT/tf_network_cli.sh
$C3QO_DIR_INT/tf_c3qo_start.sh

echo -e "$COLOR_BLUE\n    Make a LCOV report $COLOR_NO\n"
$C3QO_DIR_SOURCE/$C3QO_CMD_BUILD -l
