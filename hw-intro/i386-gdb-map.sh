#!/bin/bash

# Must be run in this directory so results are consistent (env vars change later stack addresses)
EXPECTED_PWD="/home/workspace/code/personal/hw-intro"
if [ "$PWD" != "$EXPECTED_PWD" ]; then
    echo "Error: Must run in $EXPECTED_PWD"
    exit 1
fi

# Clear all irrelevant environment variables and run GDB
env -i \
  TERM=xterm-256color \
  PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin:/home/workspace/.bin:/home/workspace/.cargo/bin \
  i386-gdb map