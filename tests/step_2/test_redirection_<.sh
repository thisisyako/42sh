#!/bin/sh
source ./functions.sh
echo "Hello" > $TEST_FILE
$SHELL -c "cat < $TEST_FILE" > $TEST_FILE.out

if [ "$(cat $TEST_FILE.out)" = "Hello" ]; then
    delete_logs
    rm -f $TEST_FILE.out
    exit 0
else
    delete_logs
    rm -f $TEST_FILE.out
    exit 1
fi
