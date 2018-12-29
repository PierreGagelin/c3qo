#!/usr/bin/env bash

#
# Script to build the c3qo project
#

# No error or undefined variables allowed
set -eu

# Get absolute path to this script
DIR_SCRIPT=$(dirname $(readlink -f $0))

source $DIR_SCRIPT/c3qo_lib.sh

# Generate every useful paths from source path
c3qo_generate_path $DIR_SCRIPT

#
# C3QO customization parameters
#
C3QO_COVERAGE="OFF"
C3QO_LOG="OFF"
C3QO_TEST="OFF"

# CMAKE customization
CMAKE_BUILD_TYPE="Debug"
CMAKE_TOOLCHAIN_FILE=$C3QO_DIR_SOURCE/toolchain_linux_x86_64_gcc.cmake

# MAKE customization
MAKE_JOBS=4

# Action to do, specified from command line options
ACTION_BUILD="false"
ACTION_CLEAN="false"
ACTION_INSTALL="false"
ACTION_LCOV="false"
ACTION_PACK="false"
ACTION_TEST="false"

#
# Clean the build directory
#
function action_clean
{
    local FILES_GCNO=
    local FILES_GCDA=

    if [ ! -d $C3QO_DIR_BUILD ]
    then
        echo "Nothing to clean, directory does not exist [dir=$C3QO_DIR_BUILD]"
        return 0
    fi

    # Clean objects, libraries and executables
    make -C $C3QO_DIR_BUILD clean

    # Clean coverage data
    if [ -d $C3QO_DIR_LCOV ]
    then
        rm -rf $C3QO_DIR_LCOV
    fi

    FILES_GCNO=$(find $C3QO_DIR_BUILD -name *.gcno)
    if [ -n "$FILES_GCNO" ]
    then
        rm $FILES_GCNO
    fi

    FILES_GCDA=$(find $C3QO_DIR_BUILD -name *.gcda)
    if [ -n "$FILES_GCDA" ]
    then
        rm $FILES_GCDA
    fi
}

#
# Build c3qo
#
function action_build
{
    mkdir -p $C3QO_DIR_BUILD

    # Could use cmake's -H and -B options but it's undocumented so better not rely on those
    cd $C3QO_DIR_BUILD
    cmake $CMAKE_OPTIONS $C3QO_DIR_SOURCE
    cd -

    make -j $MAKE_JOBS -C $C3QO_DIR_BUILD
}

#
# Install required packages
#
function action_install
{
    local url_protobuf="https://github.com/protobuf-c/protobuf-c/releases/download/v1.3.1/protobuf-c-1.3.1.tar.gz"
    local url_zeromq="https://github.com/zeromq/libzmq/releases/download/v4.3.0/zeromq-4.3.0.tar.gz"

    mkdir -p $C3QO_DIR_TOOLS

    #
    # Protobuf-c installation
    #
    if [ ! -d $C3QO_DIR_TOOLS/protobuf-c-1.3.1 ]
    then
        wget --directory-prefix=$C3QO_DIR_TOOLS $url_protobuf

        tar -C $C3QO_DIR_TOOLS -xzf $C3QO_DIR_TOOLS/protobuf-c-1.3.1.tar.gz

        cd $C3QO_DIR_TOOLS/protobuf-c-1.3.1
        CFLAGS="-O2" CXXFLAGS="-O2" ./configure
        make -j 4
        cd -

        # Don't know why it needs a symlink?
        cd $C3QO_DIR_TOOLS/protobuf-c-1.3.1/protoc-c
        ln -s protoc-gen-c protoc-c
        cd -
    fi

    #
    # ZeroMQ installation
    #
    if [ ! -d $C3QO_DIR_TOOLS/zeromq-4.3.0-build ]
    then
        wget --directory-prefix=$C3QO_DIR_TOOLS $url_zeromq

        tar -C $C3QO_DIR_TOOLS -xzf $C3QO_DIR_TOOLS/zeromq-4.3.0.tar.gz

        mkdir -p $C3QO_DIR_TOOLS/zeromq-4.3.0-build
        cd $C3QO_DIR_TOOLS/zeromq-4.3.0-build
        cmake ../zeromq-4.3.0
        make -j 4
        cd -
    fi
}

#
# Build a package of c3qo
#
function action_package
{
    make -j $MAKE_JOBS -C $C3QO_DIR_BUILD package

    # Remove a previously extracted package
    if [ -d /tmp/c3qo-0.0.7-local ]
    then
        rm -r /tmp/c3qo-0.0.7-local
    fi

    # Extract the package
    tar -C /tmp -xzf $C3QO_DIR_BUILD/c3qo-0.0.7-local.tar.gz
}

#
# Run the unit tests
#
function action_test
{
    make -C $C3QO_DIR_BUILD test
}

#
# Gather coverage report
#
function action_lcov
{
    local file_b="$C3QO_DIR_LCOV/coverage.build"
    local file_r="$C3QO_DIR_LCOV/coverage.run"
    local file_t="$C3QO_DIR_LCOV/coverage.total"
    local file_b_count=
    local file_r_count=

    # Verify that the build directory exists
    if [ ! -d $C3QO_DIR_BUILD ]
    then
        echo "FAILED: project is not built"
        exit 1
    fi

    # Verify that there are files for the analysis
    file_b_count=$(find $C3QO_DIR_BUILD -name "*.gcno" | wc -l)
    file_r_count=$(find $C3QO_DIR_BUILD -name "*.gcda" | wc -l)
    if [ $file_b_count -eq 0 ]
    then
        echo "FAILED: no .gcno files available. Did you compile with GCOV?"
        exit 1
    fi
    if [ $file_r_count -eq 0 ]
    then
        echo "FAILED: no .gcda files available. Did you run the tests?"
        exit 1
    fi

    # Prepare a directory for LCOV output
    if [ -d $C3QO_DIR_LCOV ]
    then
        rm -r $C3QO_DIR_LCOV
    fi
    mkdir $C3QO_DIR_LCOV

    # Analysis of .gcno (build) and .gcda (run) files
    lcov --directory $C3QO_DIR_BUILD --capture --initial --output-file $file_b
    lcov --directory $C3QO_DIR_BUILD --capture           --output-file $file_r

    # Catenate the output of both
    lcov --directory $C3QO_DIR_BUILD --add-tracefile $file_b --add-tracefile $file_r --output-file $file_t

    # Extract only folders after current directory
    lcov --extract $file_t "$C3QO_DIR_SOURCE/*" --output-file $file_t

    # Generate an index.html file into $C3QO_DIR_LCOV with results
    genhtml --output-directory $C3QO_DIR_LCOV $file_t

    # Clean
    rm $(find $C3QO_DIR_BUILD -name "*.gcda")

    echo "HTML report available at: $C3QO_DIR_LCOV/index.html"
}

#
# Retrieve command line options
#
while getopts "bchilptAB:C:GJ:LT" opt
do
    case "${opt}" in
        b)
            ACTION_BUILD="true"
            ;;
        c)
            ACTION_CLEAN="true"
            ;;
        h)
            echo "lol, t'as cru un peu, non ?"
            ;;
        i)
            ACTION_INSTALL="true"
            ;;
        l)
            ACTION_LCOV="true"
            ;;
        p)
            ACTION_PACK="true"
            ;;
        t)
            ACTION_TEST="true"
            ;;
        A)
            C3QO_LOG="ON"
            C3QO_TEST="ON"
            ;;
        B)
            CMAKE_BUILD_TYPE=$OPTARG
            ;;
        C)
            CMAKE_TOOLCHAIN_FILE=$OPTARG
            ;;
        G)
            C3QO_COVERAGE="ON"
            ;;
        J)
            MAKE_JOBS=$OPTARG
            ;;
        L)
            C3QO_LOG="ON"
            ;;
        T)
            C3QO_TEST="ON"
            ;;
        *)
            echo "Invalid option" >&2
            ;;
    esac
done

#
# Prepare cmake options
#
CMAKE_OPTIONS=
CMAKE_OPTIONS="$CMAKE_OPTIONS -DCMAKE_BUILD_TYPE:STRING=$CMAKE_BUILD_TYPE"
CMAKE_OPTIONS="$CMAKE_OPTIONS -DCMAKE_TOOLCHAIN_FILE:STRING=$CMAKE_TOOLCHAIN_FILE"
CMAKE_OPTIONS="$CMAKE_OPTIONS -DC3QO_COVERAGE:BOOL=$C3QO_COVERAGE"
CMAKE_OPTIONS="$CMAKE_OPTIONS -DC3QO_LOG:BOOL=$C3QO_LOG"
CMAKE_OPTIONS="$CMAKE_OPTIONS -DC3QO_PROTOBUF:STRING=$C3QO_DIR_TOOLS/protobuf-c-1.3.1"
CMAKE_OPTIONS="$CMAKE_OPTIONS -DC3QO_ZEROMQ:STRING=$C3QO_DIR_TOOLS/zeromq-4.3.0-build"
CMAKE_OPTIONS="$CMAKE_OPTIONS -DC3QO_TEST:BOOL=$C3QO_TEST"

#
# Execute actions
#
if [ $ACTION_INSTALL = "true" ]
then
    action_install
fi

if [ $ACTION_CLEAN = "true" ]
then
    action_clean
fi

if [ $ACTION_BUILD = "true" ]
then
    action_build
fi

if [ $ACTION_TEST = "true" ]
then
    action_test
fi

if [ $ACTION_PACK = "true" ]
then
    action_package
fi

if [ $ACTION_LCOV = "true" ]
then
    action_lcov
fi
