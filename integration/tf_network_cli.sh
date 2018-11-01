#!/bin/bash

# No errors or use of undefined variables allowed
set -eu

# Get absolute path to this script
dir_script=$(cd $(dirname $0) > /dev/null && pwd)

source $dir_script/../c3qo_lib.sh

# Generate every useful paths from source path
c3qo_generate_path $dir_script/..

# Run the server
$C3QO_DIR_C3QO/c3qo -f $C3QO_DIR_INT/server_zmq.txt &

#
# Network CLI with raw configuration entries
#
$C3QO_DIR_C3QO/ncli -T raw -A "dummy -p \"1 10 trans_pb\" "
$C3QO_DIR_C3QO/ncli -T raw -A "dummy -p \"5 10\" "
$C3QO_DIR_C3QO/ncli -T raw -A "dummy -p \"1 11 project_euler\" "
$C3QO_DIR_C3QO/ncli -T raw -A "dummy -p \"5 11\" "

#
# Network CLI with protobuf configuration entries
#
$C3QO_DIR_C3QO/ncli -T proto -A "dummy -i 20 -t add -a hello"
$C3QO_DIR_C3QO/ncli -T proto -A "dummy -i 20 -t conf -a ZIGOUILLATOR3000"
$C3QO_DIR_C3QO/ncli -T proto -A "dummy -i 20 -t bind -a 2:10"
$C3QO_DIR_C3QO/ncli -T proto -A "dummy -i 20 -t start"
$C3QO_DIR_C3QO/ncli -T proto -A "dummy -i 20 -t stop"
$C3QO_DIR_C3QO/ncli -T proto -A "dummy -i 20 -t del"

sleep 1

killall c3qo
