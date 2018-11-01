#!/bin/bash

# No errors or use of undefined variables allowed
set -eu

# Get absolute path to this script
dir_script=$(cd $(dirname $0) > /dev/null && pwd)

source $dir_script/../c3qo_lib.sh

# Generate every useful paths from source path
c3qo_generate_path $dir_script/..

# Run the client
$C3QO_DIR_C3QO/c3qo -f $C3QO_DIR_INT/client_us_nb.txt &

# Run the server
$C3QO_DIR_C3QO/c3qo -f $C3QO_DIR_INT/server_us_nb.txt &

sleep 1

killall c3qo
