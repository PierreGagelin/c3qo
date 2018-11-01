#!/bin/bash

#
# Test initialization
#

# No errors or use of undefined variables allowed
set -eu

# Get absolute path to this script
dir_script=$(cd $(dirname $0) > /dev/null && pwd)

source $dir_script/../c3qo_lib.sh

# Generate every useful paths from source path
c3qo_generate_path $dir_script/..

#
# Check c3qo CLI interface
#

# Option: help: should return
$C3QO_DIR_C3QO/c3qo -h

# Option: log level
$C3QO_DIR_C3QO/c3qo -l 3 &
$C3QO_DIR_C3QO/c3qo -l 300000000000000000000000000000000000 &

# Option: unknown
$C3QO_DIR_C3QO/c3qo -z &

killall c3qo
