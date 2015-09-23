################################################################################
##
## =============================================================================
##   HSA Runtime Conformance Release License
## =============================================================================
## The University of Illinois/NCSA
## Open Source License (NCSA)
##
## Copyright (c) 2014, Advanced Micro Devices, Inc.
## All rights reserved.
##
## Developed by:
##
##                 AMD Research and AMD HSA Software Development
##
##                 Advanced Micro Devices, Inc.
##
##                 www.amd.com
##
## Permission is hereby granted, free of charge, to any person obtaining a copy
## of this software and associated documentation files (the "Software"), to
## deal with the Software without restriction, including without limitation
## the rights to use, copy, modify, merge, publish, distribute, sublicense,
## and/or sell copies of the Software, and to permit persons to whom the
## Software is furnished to do so, subject to the following conditions:
##
##  - Redistributions of source code must retain the above copyright notice,
##    this list of conditions and the following disclaimers.
##  - Redistributions in binary form must reproduce the above copyright
##    notice, this list of conditions and the following disclaimers in
##    the documentation and/or other materials provided with the distribution.
##  - Neither the names of <Name of Development Group, Name of Institution>,
##    nor the names of its contributors may be used to endorse or promote
##    products derived from this Software without specific prior written
##    permission.
##
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
## IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
## FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
## THE CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
## OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
## ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
## DEALINGS WITH THE SOFTWARE.
##
################################################################################

#!/bin/bash

function usage() {
    echo "execute.sh [test file]"
}

#Set the attribute variables
TEST_SET_FILE=$1

#Check the variables for correct values
if [ ! -e ${TEST_SET_FILE} ]; then
    echo "The test set file doesn't not exist"
    usage
    exit -1
fi

## Print the header
START_TIME=`date`
MACHINE_NAME=`uname --nodename`
PROCESSOR_TYPE=`uname --processor`
OPERATING_SYSTEM=`uname --operating-system`
KERNEL_NAME=`uname --kernel-name`
KERNEL_VERSION=`uname --kernel-version`

echo "================================================================================"
echo "                      HSA Runtime Conformance Log"
echo "================================================================================"
echo "  Date:               ${START_TIME}"
echo "  Machine:            ${MACHINE_NAME}"
echo "  Processor:          ${PROCESSOR_TYPE}"
echo "  Operating System:   ${OPERATING_SYSTEM}"
echo "  Kernel:             ${KERNEL_NAME} - ${KERNEL_VERSION}"
echo "================================================================================"

## Set important environment variables
export CK_FORK=no
export PATH=$PATH:$PWD

# Test metrics
TOTAL_FAILED=0
TOTAL_PASSED=0
TOTAL_ERROR=0
TOTAL_NA=0
TOTAL_TOTAL=0
GROUP_FAILED=0
GROUP_PASSED=0
GROUP_ERROR=0
GROUP_NA=0
GROUP_TOTAL=0
TEST_GROUP=""
TEST_FAILURES=()

while read -r LINE; do
    IFS=":"; declare -a PARAMS=($LINE)

    # Determine if a new test group is being used.
    # If it is, start a new test group.
    if [[ ${TEST_GROUP} != ${PARAMS[1]} ]]; then
        # Print the results of the previous group.
        if [[ -n ${TEST_GROUP} ]]; then
            echo "  Passed: ${GROUP_PASSED}	Failed:	${GROUP_FAILED}	Error: ${GROUP_ERROR}	Total: ${GROUP_TOTAL}"
            echo ""

            # Update the total metrics.
            TOTAL_FAILED=$[TOTAL_FAILED + GROUP_FAILED]
            TOTAL_PASSED=$[TOTAL_PASSED + GROUP_PASSED]
            TOTAL_ERROR=$[TOTAL_ERROR + GROUP_ERROR]
            TOTAL_NA=$[TOTAL_NA + GROUP_NA]
            TOTAL_TOTAL=$[TOTAL_TOTAL + GROUP_TOTAL]

            # Reset the group metrics.
            GROUP_FAILED=0
            GROUP_PASSED=0
            GROUP_ERROR=0
            GROUP_NA=0
            GROUP_TOTAL=0
        fi

        # Set the test group to the new group.
        TEST_GROUP=${PARAMS[1]}

        # Print header for the new group.
        GROUP_START_TIME=`date`
        echo "${TEST_GROUP} - ${GROUP_START_TIME}"
    fi 

    export CK_RUN_CASE=${PARAMS[0]}
    echo "Running ${PARAM[0]}" >> results.out
    ${PARAMS[1]} > results.out &
    wait
    rc=$?

    if [ $rc -ne 0 ]; then
        GROUP_FAILED=$[GROUP_FAILED + 1]
        TEST_FAILURES+=("${PARAMS[1]} - ${PARAMS[0]}")
    else
        GROUP_PASSED=$[GROUP_PASSED + 1]
    fi

    GROUP_TOTAL=$[GROUP_TOTAL + 1]

done < "$TEST_SET_FILE"

# Update the total metric for the final group
TOTAL_FAILED=$[TOTAL_FAILED + GROUP_FAILED]
TOTAL_PASSED=$[TOTAL_PASSED + GROUP_PASSED]
TOTAL_ERROR=$[TOTAL_ERROR + GROUP_ERROR]
TOTAL_NA=$[TOTAL_NA + GROUP_NA]
TOTAL_TOTAL=$[TOTAL_TOTAL + GROUP_TOTAL]

## Print the final groups results
echo "  Passed: ${GROUP_PASSED}	Failed:	${GROUP_FAILED}	Error: ${GROUP_ERROR}	Total: ${GROUP_TOTAL}"
echo ""

## Print total results
echo "================================================================================"
echo "Testrun"
echo "  Passed: ${TOTAL_PASSED}	Failed: ${TOTAL_FAILED}	Error: ${TOTAL_ERROR}	Total: ${TOTAL_TOTAL}"
echo "================================================================================"
echo ""

## Print out any failures and exit

if [ ${#TEST_FAILURES[@]} -ne 0 ]; then
    echo "Failed tests:"

    for TEST in "${TEST_FAILURES[@]}"; do
        echo "  ${TEST}"
    done

    exit 1
else
    exit 0
fi
