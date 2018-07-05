#!/bin/bash

DIR_CURRENT=$(dirname $_)

# No errors or use of undefined variables allowed
set -eux

DIR_C3QO=$DIR_CURRENT/../../build/c3qo

if [ ! -d $DIR_C3QO ]
then
    echo "Failed to run test: c3qo build directory does not exist [dir=$DIR_C3QO]"
    exit 1
fi

# Run the network command line interface
$DIR_C3QO/ncli &

# Run the server
$DIR_C3QO/c3qo -f $DIR_CURRENT/server_zmq.txt &

sleep 1

killall c3qo

