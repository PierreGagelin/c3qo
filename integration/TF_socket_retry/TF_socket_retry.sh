#!/bin/bash

DIR_DEV=$HOME/dev
DIR_C3QO=$DIR_DEV/build/c3qo
DIR_DOCKER=$DIR_DEV/integration/docker
DIR_TF=$DIR_DEV/integration/TF_socket_retry

IMAGE="node"
CONTAINER="node1"

EXE_C3QO=$DIR_C3QO/c3qo

CONF_CLIENT=$DIR_TF/client.txt
CONF_SERVER=$DIR_TF/server.txt

if [ ! -d $DIR_C3QO ]
then
    echo "[ERR] Failed to initialize TF: no build directory at $DIR_C3QO"
fi

# Update the image
docker build --tag=$IMAGE --file=$DIR_DOCKER/Dockerfile $DIR_DOCKER

# Restart the container
docker stop --time=0 $CONTAINER
docker rm $CONTAINER
docker run --privileged --interactive=true --tty=true --detach=true --name=$CONTAINER $IMAGE

# Copy files
docker cp $EXE_C3QO    $CONTAINER:/usr/local/bin/
docker cp $CONF_CLIENT $CONTAINER:/usr/local/bin/
docker cp $CONF_SERVER $CONTAINER:/usr/local/bin/
