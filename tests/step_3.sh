#!/bin/sh

source ./functions.sh

# Exit tests
test_case 'exit' "exit"
test_case 'exit basic' "exit 0"
test_case 'exit with status' "exit 42"
test_case 'exit after command' "echo 'Before exit'; exit 0"
test_case 'exit with invalid status' "exit abc"

# CD tests, numbers based on SCL
test_case 'unset_home' 'unset HOME; cd'
test_case 'set_home' 'HOME=/tmp; cd; echo $PWD'
test_case 'pwd null' 'unset PWD; cd'
test_case 'absolute_path' 'cd /tmp; echo $PWD'
test_case 'dot_path' 'cd .; echo $PWD'
test_case 'dot_dot_path' 'mkdir -p a/b; cd a/b; cd ..; echo $PWD; cd ..; rm -rf a/'
test_case 'cdpath_empty' 'CDPATH=""; cd testdir; echo $PWD'
test_case 'cdpath_set' 'CDPATH=/tmp:/home; cd testdir; echo $PWD'
test_case 'permission_denied' 'mkdir noperm; chmod 000 noperm; cd noperm; rm -rf noperm'
test_case 'oldpwd_check' 'cd /tmp; cd -; echo $PWD'
test_case 'canonical_dots' 'cd ./././.; echo $PWD'
test_case 'canonical_dotdot' 'cd a/b/../../a; echo $PWD'
test_case 'multiple_slashes' 'mkdir /tmp/dir; cd /tmp////dir; echo $PWD; rm -rf /tmp/dir'
test_case 'trailing_slash' 'cd /tmp/; echo $PWD'
test_case 'nonexistent' 'cd nonexistent'
test_case 'file_not_dir' 'touch file1; cd file1; rm -rf file1'
test_case 'relative_to_absolute' 'mkdir -p relative/path; cd ./relative/path; echo $PWD; cd ../../; rm -rf relative'

# Export tests
test_case 'export basic' "export TEST=value; echo \$TEST"
test_case 'export existing var' "TEST=first; export TEST=second; echo \$TEST"
test_case 'export without value' "export TEST; echo \$TEST"
test_case 'export multiple' "export A=1 B=2 C=3; echo \$A \$B \$C"
test_case 'export invalid name' "export 2invalid=value"
# test_case 'export with spaces' "export TEST='value with spaces'; echo \$TEST"

# Break/Continue tests
# test_case 'break in while' "i=0; while true; do i=$((i+1)); if [ \$i -eq 3 ]; then break; fi; echo \$i; done"
# test_case 'continue in while' "i=0; while [ \$i -lt 5 ]; do i=$((i+1)); if [ \$i -eq 3 ]; then continue; fi; echo \$i; done"
# test_case 'break invalid' "breakexport with spaces"
# test_case 'continue invalid' "continue"
test_case 'continue in for' "for x in hello world; do continue; done"
test_case 'WHILE true break' 'while true; do break; done'
test_case 'UNTIL false break' 'until false; do break; done'

# Dot (source) tests, to update and create some files in step_3/*.sh
test_case 'dot basic' ". ./step_3/test_script.sh"
test_case 'dot with arguments' ". ./step_3/test_script.sh arg1 arg2"
test_case 'dot with arguments' ". /step_3/test_script.sh arg1 arg2"
test_case 'dot nonexistent file' ". ./nonexistent.sh"
test_case 'dot relative path' ". ../tests/step_3/test_script.sh"

test_case 'dot basic' ". ./step_3/test_script_complex.sh"
test_case 'dot with arguments' ". ./step_3/test_script_complex.sh arg1 arg2"
test_case 'dot with arguments' ". /step_3/test_script_complex.sh arg1 arg2"
test_case 'dot relative path' ". ../tests/step_3/test_script_complex.sh"

# Unset tests
test_case 'unset basic' "TEST=value; unset TEST; echo \$TEST"
test_case 'unset multiple' "A=1 B=2; unset A B; echo \$A \$B"
test_case 'unset nonexistent' "unset NONEXISTENT"
test_case 'unset nonexistent 2' "unset NONEXISTENT NONEXISTENT2"
test_case 'unset with -v' "TEST=value; unset -v TEST; echo \$TEST"
test_case 'unset with -f' "foo() { echo bar; }; unset -f foo; foo"
test_case 'unset invalid option' "unset -z TEST"
test_case 'unset both -f and -v' "unset -f -v TEST"
test_case 'unset multiple nonexistent' "unset NON1 NON2 NON3; echo \$?"
test_case 'unset invalid name' "unset 123invalid"
test_case 'unset special char' "unset 'TEST@VAR'"
test_case 'unset with spaces' "unset 'VAR NAME'"
test_case 'unset function var conflict' "VAR=value; f() { echo hi; }; unset f; echo \$VAR"
test_case 'unset nonexistent function' "unset -f nonexistent_func; echo \$?"
test_case 'unset env var' "export ENV_VAR=value; unset ENV_VAR; echo \$ENV_VAR"
test_case 'unset env var persistence' "export PERSIST=value; unset PERSIST; bash -c 'echo \$PERSIST'"
test_case 'unset function in use' "f() { echo hi; }; unset -f f; f"

# Command blocks tests
test_case 'basic block' "{ echo 'one'; echo 'two'; }"
test_case 'nested blocks' "{ echo 'outer'; { echo 'inner'; }; }"
test_case 'block with if' "{ if true; then echo 'true'; fi; }"
test_case 'block variable scope' "{ local_var=value; echo \$local_var; }"

# Function tests
test_case 'function definition' "test_func() { echo 'In function'; }; test_func"
test_case 'function with arguments' "test_func() { echo \$1 \$2; }; test_func arg1 arg2"
test_case 'nested functions' "outer() { inner() { echo 'inner'; }; inner; }; outer"

test_case 'simple function definition' "test_func() { echo 'Function executed'; }; test_func"
test_case 'function definition with parentheses' "test_func() ( echo 'Function executed with parentheses'; ); test_func"
test_case 'function with arguments' "test_func() { echo \$1 \$2; }; test_func arg1 arg2"
test_case 'function returning fixed string' "test_func() { echo 'Fixed return value'; }; test_func"
test_case 'nested functions' "outer() { inner() { echo 'Inner function executed'; }; inner; }; outer"
test_case 'function redefining a variable' "test_func() { var='Reassigned'; echo \$var; }; var='Initial'; test_func"
test_case 'function using a global variable' "global_var='Global value'; test_func() { echo \"Accessing: \$global_var\"; }; test_func"
test_case 'function calling another function' "func1() { echo 'First function'; }; func2() { func1; echo 'Second function'; }; func2"
test_case 'function with a loop' "test_func() { for i in 1 2 3; do echo \"Value: \$i\"; done; }; test_func"
test_case 'function with multiple commands' "test_func() { echo 'Command 1'; echo 'Command 2'; }; test_func"
test_case 'function with redirection' "test_func() { echo 'Redirecting output' > test_output.txt; }; test_func; cat test_output.txt; rm test_output.txt"
test_case 'function with pipes' "test_func() { echo 'Test' | grep 'Test'; }; test_func"

# Command substitution tests
test_case 'basic substitution' "echo \$(echo 'test')"
test_case 'nested substitution' "echo \$(echo \$(echo 'nested'))"
test_case 'substitution with quotes' "echo \$(echo 'quoted string')"
test_case 'substitution in variable' "var=\$(echo 'value'); echo \$var"
test_case 'substitution with multiple commands' "echo \$(echo 'one'; echo 'two')"
test_case 'basic command substitution' "echo \$(echo 'test')"
test_case 'backticks command substitution' "echo \`echo 'test'\`"
test_case 'substitution with newline stripping' "echo \$(echo -e 'line1\nline2\n')"
test_case 'substitution with embedded newlines' "echo \"\$(echo -e 'line1\nline2')\""
test_case 'substitution inside double quotes' "echo \"\$(echo 'quoted')\""
test_case 'backslash escaping inside backticks' "echo \`echo \\`echo 'escaped'\\`\`"
test_case 'substitution with variable assignment' "var=\$(echo 'value'); echo \$var"
test_case 'substitution with command failure' "echo \$(false)"
test_case 'substitution capturing stderr' "echo \$(ls non_existent 2>&1)"

delete_logs
print_result
