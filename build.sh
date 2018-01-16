#!/bin/bash
# Script to build the c3qo project


BUILD_DIR="build/"

LLVM="false"

ACTION_BUILD="false"
ACTION_CLEAN="false"


function build
{
        mkdir -p "$BUILD_DIR"
        cd "$BUILD_DIR"

        if [ $LLVM = "true" ]
        then
                export CC=/usr/bin/clang
                export CXX=/usr/bin/clang++
        fi

        cmake ../
        make -j 4

        cd -
}


function clean
{
        if [ -d "$BUILD_DIR" ]
        then
                rm -r "$BUILD_DIR"
        fi
}


while getopts "bcl" opt
do
        case "${opt}" in
                b)
                        ACTION_BUILD="true"
                        ;;
                c)
                        ACTION_CLEAN="true"
                        ;;
                l)
                        LLVM="true"
                        ;;
                *)
                        echo "Invalid option" >&2
                        ;;
        esac
done


if [ $ACTION_CLEAN = "true" ]
then
        clean
fi


if [ $ACTION_BUILD = "true" ]
then
        build
fi


