#!/bin/sh
source ./functions.sh
$SHELL -c "echo Hello >| $TEST_FILE"

REDIRECT_STATUS=$?
if [ $REDIRECT_STATUS -eq 0 ] && [ -f "$TEST_FILE" ] 2>/dev/null && [ "$(cat $TEST_FILE 2>/dev/null)" = "Hello" ]; then
    delete_logs
    exit 0
else
    delete_logs
    echo "File was not created."
    exit 1
fi
