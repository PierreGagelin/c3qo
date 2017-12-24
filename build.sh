#!/bin/bash
# Script to build the c3qo project

BUILD_DIR="build/"

function build
{
        mkdir -p "$BUILD_DIR"
        cd "$BUILD_DIR"
        cmake ../
        make -j8
        cd -
}


function clean
{
        if [ -d "$BUILD_DIR" ]
        then
                rm  -r "$BUILD_DIR"
        fi
}


while getopts "bc" opt
do
        case "${opt}" in
                b)
                        build
                        ;;
                c)
                        clean
                        ;;
                *)
                        echo "Invalid option" >&2
                        ;;
        esac
done


