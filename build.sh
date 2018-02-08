#!/bin/bash
# Script to build the c3qo project


DIR_BUILD="build"
DIR_LCOV="$DIR_BUILD/lcov"

LLVM="false"

ACTION_BUILD="false"
ACTION_CLEAN="false"
ACTION_LCOV="false"


function action_build
{
        mkdir $DIR_BUILD
        cd $DIR_BUILD

        if [ $LLVM = "true" ]
        then
                export CC=/usr/bin/clang
                export CXX=/usr/bin/clang++
        fi

        cmake ../
        make -j 4

        cd -
}


function action_clean
{
        if [ -d $DIR_BUILD ]
        then
                rm -r "$DIR_BUILD"
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

        # Extract only src folders
        lcov --extract $file_t "*/src/*" --output-file $file_t

        # Remove gtest src folder
        lcov --remove $file_t "*/gtest*" --output-file $file_t

        # Generate an index.html file into $DIR_LCOV with results
        genhtml --output-directory $DIR_LCOV $file_t

        open $DIR_LCOV/index.html

        # Clean
        rm  $(find $DIR_BUILD -name "*.gcda")
}


while getopts "bcgl" opt
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


