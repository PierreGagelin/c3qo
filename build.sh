#!/bin/bash
# Script to build the c3qo project
# Works with relative path

DIR_SOURCE=$(dirname $_)
DIR_BUILD="$DIR_SOURCE/build"
DIR_LCOV="$DIR_BUILD/lcov"

LLVM="false"
EXPORT_SYMBOLS="OFF"

ACTION_BUILD="false"
ACTION_CLEAN="false"
ACTION_LCOV="false"


function action_build
{
    mkdir -p $DIR_BUILD

    # Could use cmake's -H and -B options but it's undocumented so better not rely on those
    cd $DIR_BUILD
    cmake -DNO_AS_NEEDED:BOOL=$EXPORT_SYMBOLS ../
    cd -

    make -C $DIR_BUILD -j 4
}


function action_clean
{
    local FILES_GCNO=
    local FILES_GCDA=

    if [ ! -d $DIR_BUILD ]
    then
        echo "Nothing to clean, directory does not exist [dir=$DIR_BUILD]"
        return 0
    fi

    # Clean objects, libraries and executables
    make -C $DIR_BUILD clean

    # Clean coverage data
    if [ -d $DIR_LCOV ]
    then
        rm -rf lcov/
    fi

    FILES_GCNO=$(find $DIR_BUILD -name *.gcno)
    if [ -n "$FILES_GCNO" ]
    then
        rm $FILES_GCNO
    fi

    FILES_GCDA=$(find $DIR_BUILD -name *.gcda)
    if [ -n "$FILES_GCDA" ]
    then
        rm $FILES_GCDA
    fi
}


function action_lcov
{
    local file_b="$DIR_LCOV/coverage.build"
    local file_r="$DIR_LCOV/coverage.run"
    local file_t="$DIR_LCOV/coverage.total"
    local file_b_count=
    local file_r_count=

    # Verify that the build directory exists
    if [ ! -d $DIR_BUILD ]
    then
        echo "FAILED: project is not built"
        exit 1
    fi

    # Verify that there are files for the analysis
    file_b_count=$(find $DIR_BUILD -name "*.gcno" | wc -l)
    file_r_count=$(find $DIR_BUILD -name "*.gcda" | wc -l)
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
    if [ -d $DIR_LCOV ]
    then
        rm -r $DIR_LCOV
    fi
    mkdir $DIR_LCOV

    # Analysis of .gcno (build) and .gcda (run) files
    lcov --directory $DIR_BUILD --capture --initial --output-file $file_b
    lcov --directory $DIR_BUILD --capture           --output-file $file_r

    # Catenate the output of both
    lcov --directory $DIR_BUILD --add-tracefile $file_b --add-tracefile $file_r --output-file $file_t

    # Extract only folders after current directory
    lcov --extract $file_t "`pwd`/*" --output-file $file_t

    # Generate an index.html file into $DIR_LCOV with results
    genhtml --output-directory $DIR_LCOV $file_t

    open $DIR_LCOV/index.html

    # Clean
    rm $(find $DIR_BUILD -name "*.gcda")
}


while getopts "bchlEL" opt
do
    case "${opt}" in
        b)
            ACTION_BUILD="true"
            ;;
        c)
            ACTION_CLEAN="true"
            ;;
        l)
            ACTION_LCOV="true"
            ;;
        h)
            echo "lol, t'as cru un peu, non ?"
            ;;
        E)
            EXPORT_SYMBOLS="ON"
            ;;
        L)
            LLVM="true"
            ;;
        *)
            echo "Invalid option" >&2
            ;;
    esac
done


if [ $ACTION_CLEAN = "true" ]
then
    action_clean
fi


if [ $ACTION_BUILD = "true" ]
then
    action_build
fi


if [ $ACTION_LCOV = "true" ]
then
    action_lcov
fi


