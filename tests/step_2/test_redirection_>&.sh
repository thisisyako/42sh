#!/bin/sh
source ./functions.sh
$SHELL -c "echo Error 1>&2" 2> $TEST_FILE

if [ "$(cat $TEST_FILE)" = "Error" ]; then
    delete_logs
    exit 0
else
    delete_logs
    exit 1
fi
