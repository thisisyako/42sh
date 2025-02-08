#!/bin/sh
source ./functions.sh

PIPE_TEST_FILE="pipe_test_output.txt"
PIPE_TEST_CMD="echo Hello | grep Hello > $PIPE_TEST_FILE"

$SHELL -c "$PIPE_TEST_CMD"

PIPE_STATUS=$?
if [ $PIPE_STATUS -eq 0 ] && [ -f "$PIPE_TEST_FILE" ] 2>/dev/null && [ "$(cat $PIPE_TEST_FILE 2>/dev/null)" = "Hello" ]; then
    delete_logs
    rm -f $PIPE_TEST_FILE
    exit 0
else
    delete_logs
    echo "Pipeline functionality failed."
    rm -f $PIPE_TEST_FILE
    exit 1
fi
