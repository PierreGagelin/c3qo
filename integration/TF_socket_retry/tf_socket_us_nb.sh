#!/bin/bash

# No errors or use of undefined variables allowed
set -eux

DIR_BIN=""
DIR_CONFIG=""

while getopts "b:c:" opt
do
    case "${opt}" in
        b)
            DIR_BIN=$OPTARG
            ;;
        c)
            DIR_CONFIG=$OPTARG
            ;;
        *)
            echo "Invalid option" >&2
            exit 1
            ;;
    esac
done

if [ ! -d $DIR_BIN ]
then
    echo "Failed to run test: c3qo build directory does not exist [dir=$DIR_BIN]"
    exit 1
fi

# Run the client
$DIR_BIN/c3qo -f $DIR_CONFIG/client_us_nb.txt &

# Run the server
$DIR_BIN/c3qo -f $DIR_CONFIG/server_us_nb.txt &

sleep 1

killall c3qo
