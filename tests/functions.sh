#!/bin/sh

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color
SHELL=../src/42sh
TEST_FILE="output.out"
TEMP_SCRIPT="temp_script.sh"

TESTS_FAILED=0
TOTAL_TESTS=0

test_case() {
    local name="$1"
    shift
    local command="$1"
    shift
    local params="$*"
    ((TOTAL_TESTS += 3)) # Three tests per case now

    # Method 1: -c option
    bash --posix -c "$command" $params > bash.stdout 2> bash.stderr
    BASH_RC=$?
    $SHELL -c "$command" $params > 42sh.stdout 2> 42sh.stderr
    SH42_RC=$?

    if [ $BASH_RC -ne $SH42_RC ] || ! diff bash.stdout 42sh.stdout > /dev/null; then
        print_failure "-c" "$name" "$command" "$params" $BASH_RC $SH42_RC
    else
        echo -en "${GREEN}.${NC}"
    fi

    # Method 2: stdin (pipe)
    # if there are params we don't test this method
    if [ -z "$params" ]; then
        echo "$command" | bash --posix > bash.stdout 2> bash.stderr
        BASH_RC=$?
        echo "$command" | $SHELL > 42sh.stdout 2> 42sh.stderr
        SH42_RC=$?

        if [ $BASH_RC -ne $SH42_RC ] || ! diff bash.stdout 42sh.stdout > /dev/null; then
            print_failure "stdin" "$name" "$command" "$params" $BASH_RC $SH42_RC
        else
            echo -en "${GREEN}.${NC}"
        fi
    fi

    # Method 3: script execution
    echo "#!/bin/sh" > "$TEMP_SCRIPT"
    echo "$command" >> "$TEMP_SCRIPT"
    chmod +x "$TEMP_SCRIPT"

    bash --posix "$TEMP_SCRIPT" $params > bash.stdout 2> bash.stderr
    BASH_RC=$?
    $SHELL "$TEMP_SCRIPT" $params > 42sh.stdout 2> 42sh.stderr
    SH42_RC=$?

    if [ $BASH_RC -ne $SH42_RC ] || ! diff bash.stdout 42sh.stdout > /dev/null; then
        print_failure "script" "$name" "$command" "$params" $BASH_RC $SH42_RC
    else
        echo -en "${GREEN}.${NC}"
    fi
}

print_failure() {
    local method="$1"
    local name="$2"
    local command="$3"
    local params="$4"
    local bash_rc="$5"
    local sh42_rc="$6"

    echo -e "\n${RED}✗ Test failed (Method: $method)"
    echo -e "${BLUE}Tested:${NC} $name"
    echo -e "${YELLOW}Command:${NC} $command $params"
    echo -e "Return code:"
    echo -e "  bash: $bash_rc"
    echo -e "  42sh: $sh42_rc"
    echo -e "stdout mismatch:${NC}"
    echo -e "${YELLOW}Expected (bash):${NC}"
    cat bash.stdout
    echo -e "${YELLOW}Got (42sh):${NC}"
    cat 42sh.stdout
    ((TESTS_FAILED++))
}

test_script() {
    local name="$1"
    ((TOTAL_TESTS += 1))
    
    ./$name
    RET_CODE=$?

    if [ $RET_CODE -ne 0 ]; then
        echo -e "\n${RED}✗ Failed ${NC} $name ${NC}"
        ((TESTS_FAILED++))
        return
    else
        echo -en "${GREEN}.${NC}"
    fi
}

# Print the result of the tests
print_result() {
    if [ $TESTS_FAILED -eq 0 ]; then
        echo -e "${GREEN}SUCCESS${NC}"
        echo -e "All ${TOTAL_TESTS} tests passed!"
    else
        echo -e "\n${RED}FAILED${NC}"
        echo -e "${RED}${TESTS_FAILED}/${TOTAL_TESTS}${NC} tests failed ${NC}"
    fi
    echo -e "\n\n"
    delete_logs
    exit 0
}

delete_logs() {
    rm -f *.std*
    rm -f $TEST_FILE
    rm -f input.txt
    rm -f $TEMP_SCRIPT
}