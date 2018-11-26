#!/usr/bin/env bash

#
# This file is a helper to run cppcheck on c3qo project
#
# Has to be launched in the root directory with project built under ../build
#

set -eu

INCLUDES=
INCLUDES_OPT=
FILE_OUTPUT_REPORT="/tmp/cppcheck_result.txt"
FILE_OUTPUT_CONFIG="/tmp/cppcheck_config.txt"

# General c3qo includes
INCLUDES=$(find . -name include)
for include in $INCLUDES
do
    INCLUDES_OPT="$INCLUDES_OPT -I $include"
done

#
# We could add these to be fully tested but:
# - it costs a lot (like 95% maybe) of processing time
# - these are external COTS
# - did it once and it did not reveal anything relevant
#
# # Specific build-time includes
# ## Generated protobuf header
# INCLUDES=$(find ../build -name *.pb.h)
# for include in $INCLUDES
# do
#     INCLUDES_OPT="$INCLUDES_OPT -I $(dirname $include)"
# done
#

cppcheck --enable=all $INCLUDES_OPT . 2> $FILE_OUTPUT_REPORT
cppcheck --enable=all $INCLUDES_OPT --check-config . 2> $FILE_OUTPUT_CONFIG
echo ""
echo "cppcheck report available at: $FILE_OUTPUT_REPORT"
echo "cppcheck config available at: $FILE_OUTPUT_CONFIG"
