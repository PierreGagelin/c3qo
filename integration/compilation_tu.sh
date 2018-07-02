#! /bin/bash
# Test file to test non-regression of the compilation and of the test units


DIR_BUILD=$(dirname $_)/..

BUILD_COMMAND=build.sh


# No errors or undefined variables allowed
set -eu


echo "Clean build directory and do a CLASSIC build"
$DIR_BUILD/$BUILD_COMMAND -Ecbt

echo "Clean build directory and do a RELEASE build"
$DIR_BUILD/$BUILD_COMMAND -ERcbt

echo "Clean build directory and do a STATIC build"
$DIR_BUILD/$BUILD_COMMAND -EScbt

echo "Clean build directory and do a GCOV build"
$DIR_BUILD/$BUILD_COMMAND -EGcbtl

