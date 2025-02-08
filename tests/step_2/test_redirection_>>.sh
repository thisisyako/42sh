#!/bin/sh
source ./functions.sh

echo "Line 1" > $TEST_FILE
$SHELL -c "echo Line 2 >> $TEST_FILE"
REDIRECT_STATUS=$?
LINES=$(wc -l < "$TEST_FILE")
if [ $REDIRECT_STATUS -eq 0 ] && [ -f "$TEST_FILE" ] && [ "$LINES" -eq 2 ]; then
    delete_logs
    exit 0
else
    echo "Append redirection test failed"
    delete_logs
    exit 1
fi
