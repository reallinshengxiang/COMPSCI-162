#!/bin/bash

(make clean && make) >> /dev/null

echo "Testing built-in command"
SHELL_PWD=`echo "pwd" | ./shell`
if [ "$SHELL_PWD" != "$PWD" ]; then
    echo "Build-in commands failed"
    printf "%s\n" \
    "Expected: ${PWD}" \
    "Received: ${SHELL_PWD}"
    exit 1
fi
echo "Built-in commands passed"


echo "Testing basic command"
SHELL_LS=`echo "/bin/ls" | ./shell`
REAL_LS=`ls`
if [ "$SHELL_LS" != "$REAL_LS" ]; then
    echo "Basic command failed"
    printf "%s\n" \
    "Expected: ${REAL_LS}" \
    "Received: ${SHELL_LS}"
    exit 1
fi
echo "Basic command passed"

