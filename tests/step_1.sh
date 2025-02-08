#!/bin/sh

source ./functions.sh

# Basic echo tests
test_case 'echo basic print' "echo 'Hello, World!'"
test_case 'echo no newline' "echo -n 'Hello, World!'"
test_case 'echo basic escapes' "echo -e 'Line1\nLine2'"
test_case 'echo tab escape' "echo -e 'Tab\tSeparated'"
test_case 'echo backslash escape' "echo -e 'Backslash:\\'"
test_case 'echo disable escapes' "echo -E 'Line1\nLine2'"
test_case 'echo disable tab escape' "echo -E 'Tab\tSeparated'"
test_case 'echo disable backslash escape' "echo -E 'Backslash:\\'"
test_case 'echo no newline with escapes' "echo -n -e 'Hello\nWorld'"
test_case 'echo no newline, disable escapes' "echo -n -E 'Hello\nWorld'"
test_case 'echo empty' "echo"

# Multiple args echo tests
test_case 'echo multiple arguments' "echo 'This is a test of multiple arguments'"
test_case 'echo no newline, multiple arguments' "echo -n 'This is a test of multiple arguments'"
test_case 'echo escapes with multiple arguments' "echo -e 'This is a test\nof multiple arguments'"

# Escapes echo tests
test_case 'echo mixed escapes' "echo -e 'Line1\\nLine2\tTabbed\tLine3\\nLine4'"
test_case 'echo complex escapes' "echo -e '\\t\\nTesting\\tescapes\\n\\'"
test_case 'echo disable complex escapes' "echo -E '\\t\\nTesting\\tescapes\\n\\'"
test_case 'echo tabulations with escapes' "echo -e '\t\t\tTabulations'"
test_case 'echo newlines with escapes' "echo -e '\n\n\nNewlines'"
test_case 'echo varied escapes' "echo -e '\\n\t\\\tEscape sequences'"
test_case 'echo disable escapes, tabulations' "echo -E '\t\t\tTabulations'"
test_case 'echo disable escapes, newlines' "echo -E '\n\n\nNewlines'"
test_case 'echo disable varied escapes' "echo -E '\\n\t\\\tEscape sequences'"

# Spaces echo tests
test_case 'echo leading/trailing spaces' "echo '    Leading and trailing spaces     '"
test_case 'echo no newline with spaces' "echo -n '    Leading and trailing spaces     '"
test_case 'echo escapes with spaces' "echo -e '    Leading and trailing\nspaces'"

# No arg echo tests
test_case 'echo no arguments, newline suppressed' "echo -n -e"
test_case 'echo disable escapes, no arguments' "echo -E"
test_case 'echo no newline, no arguments' "echo -n"

# Many args echo tests
test_case 'echo many arguments' "echo 'arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9 arg10 arg11 arg12 arg13 arg14 arg15'"
test_case 'echo many arguments with escapes' "echo -e 'arg1\\narg2\\narg3\\narg4\\narg5'"
test_case 'echo no newline, many arguments' "echo -n 'arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9 arg10'"

# Basic if/then tests
test_case 'if true' "if true; then echo 'True'; fi"
test_case 'if false' "if false; then echo 'True'; fi"
test_case 'if true else' "if true; then echo 'True'; else echo 'False'; fi"
test_case 'if false else' "if false; then echo 'True'; else echo 'False'; fi"
test_case 'if true elif true' "if false; then echo 'False'; elif true; then echo 'True'; fi"
test_case 'if true elif false' "if false; then echo 'False'; elif false; then echo 'Also False'; else echo 'True'; fi"
test_case 'if true elif false else' "if false; then echo 'False'; elif false; then echo 'Also False'; else echo 'True'; fi"

# Advanced if/then tests
test_case 'nested ifs' "if true; then if false; then echo 'Nested False'; else echo 'Nested True'; fi; fi"
test_case 'empty if' "if true; then ; fi"
test_case 'if with system command' "if ls; then echo 'LS ran'; else echo 'LS failed'; fi"
test_case 'else with no if' "else echo 'This should fail'; fi"
test_case 'elif with no if' "elif true; then echo 'This should fail'; fi"
test_case 'if with multiple elifs' "if false; then echo 'False'; elif true; then echo 'First elif'; elif false; then echo 'Second elif'; else echo 'Else'; fi"
test_case 'if with multiple commands in block' "if true; then echo 'Hello'; echo 'World'; fi"
test_case 'elif with command substitution' "if false; then echo 'False'; elif true; then echo 'Command Substitution'; else echo 'Else'; fi"
test_case 'if with multiple commands in elif' "if false; then echo 'False'; elif true; then echo 'First'; echo 'Second'; else echo 'Else'; fi"
test_case 'if with false and multiple elifs' "if false; then echo 'False'; elif false; then echo 'Second'; elif true; then echo 'True'; fi"
test_case 'elif multiple commands' "if false; then echo 'Problem'; elif true; then echo 'Accepted'; else false; fi; true;"
test_case 'missing fi' "if true; then echo 'Missing fi'"
test_case 'missing then' "if true echo 'Missing then'; fi"

# Multiple commands with ;
test_case 'multiple commands in one line' "echo 'First'; echo 'Second'; echo 'Third';"
test_case 'multiple commands with true and false' "true; echo 'True was successful'; false; echo 'False was unsuccessful';"
test_case 'multiple commands with system command' "echo 'Before'; ls; echo 'After';"
test_case 'commands with no output' "true; false; true; echo 'All commands executed';"
test_case 'multiple echo commands' "echo 'Start'; echo 'Middle'; echo 'End';"
test_case 'multiple echo commands with no output in between' "echo 'First'; echo 'Second'; echo 'Third';"
test_case 'multiple true and false' "true; false; true; false; true; echo 'Commands finished';"
test_case 'empty commands' "true;; false;; echo 'Commands finished';"
test_case 'testing semicolon with no output' "true;;; echo 'End';"
test_case 'mixing true, false, and echo' "true; echo 'True executed'; false; echo 'False executed'; true; echo 'True again';"
test_case 'commands with no spaces between' "true;false;true;echo 'End';"
test_case 'sequential commands without spaces' "echo 'Start';echo 'Middle';echo 'End';"

# Newline handling
test_case 'multiple commands on separate lines' "echo 'First'
echo 'Second'
echo 'Third'"
test_case 'true and false on separate lines' "true
echo 'True was successful'
false
echo 'False was unsuccessful'"
test_case 'multiple echo commands on separate lines' "echo 'Start'
echo 'Middle'
echo 'End'"
test_case 'commands with system call on multiple lines' "echo 'Before'
ls
echo 'After'"
test_case 'true, false, and echo on multiple lines' "true
echo 'True executed'
false
echo 'False executed'
true
echo 'True again'"
test_case 'empty lines between commands' "true

echo 'End'"
test_case 'sequential commands with true and false on multiple lines' "true
false
true
echo 'End'"
test_case 'mixing commands on separate lines' "echo 'First'
true
echo 'Second'
false
echo 'Third'"
test_case 'multiple echo commands with empty lines' "echo 'Start'

echo 'Middle'

echo 'End'"

# true/false builtins
test_case "True builtin" 'true'
test_case "False builtin" 'false'
test_case "True False as ifs" "true && echo 'Hello World'
false && echo 'Not True'"

# Complex combinations
test_case "Complex if" 'if true; then echo one; if false; then echo wrong; else echo two; fi; echo three; fi'
test_case "Complex elif" 'if false; then echo wrong; elif true; then echo right; elif false; then echo wrong; else echo wrong; fi'

# Quote handling
test_case 'single quotes around a word' "echo 'Hello'"
test_case 'single quotes with spaces' "echo 'Hello World'"
test_case 'single quotes with special characters' "echo 'Hello $USER!'"
test_case 'single quotes with multiple words and spaces' "echo 'This is a test'"
test_case 'single quotes with newline' "echo 'Hello\nWorld'"
test_case 'single quotes and backslashes' "echo 'Hello \\ World'"
test_case 'single quotes with a command' "echo 'Current directory: $(pwd)'"
test_case 'empty single-quoted string' "echo ''"
test_case 'single quotes with an embedded single-quoted string' "echo 'This is a ''nested'' string'"
test_case 'unterminated single quote' "echo 'Unfinished"

# Basic lexer tests
test_case 'leading spaces' "     echo 'Leading spaces'"
test_case 'trailing spaces' "echo 'Trailing spaces'     "
test_case 'multiple spaces' "echo    'Multiple    spaces'"

# lexer/parser invalid syntax tests
test_case 'empty command with semicolon' "; echo 'Done'"
test_case 'command after multiple semicolons' "echo 'Before';; echo 'After'"
test_case 'missing fi for if statement' "if true; then echo 'True'"
test_case 'missing then after if' "if true echo 'True'; fi"
test_case 'empty command after else' "if true; then echo 'True'; else; fi"
test_case 'extra semicolon at the end' "echo 'First';; echo 'Second';"

# Basic comment tests
test_case 'single comment after a command' "echo 'Hello' # This is a comment"
test_case 'standalone comment' "# This is a full-line comment"
test_case 'comment after multiple commands' "echo 'First'; echo 'Second' # Comment here"
test_case 'comment with special characters' "# @!#$%^&*()"
test_case 'empty comment' "#"

# if comment tests
test_case 'if statement with inline comment' "if true; then echo 'True'; fi # End of if block"
test_case 'if-else with comments' "if true; then echo 'True'; # Then block\nelse echo 'False'; # Else block\nfi"
test_case 'comment after fi' "if true; then echo 'True'; fi # Closing the if"
test_case 'multiple comments in control flow' "if false; then # Start of then block\necho 'False'; # Inside then block\nelse echo 'True'; # Inside else block\nfi"

# Advanced comment tests
test_case 'comment after a semicolon' "echo 'A'; # Comment after first command\necho 'B'"
test_case 'comment after newline-separated commands' "echo 'First'
# Inline comment
echo 'Second'"
test_case 'multiple commands with comments' "echo 'First'; echo 'Second'; # End of commands"
test_case 'comment with single quotes' "echo 'Hello' # Comment after single quotes"
test_case 'comments with keywords' "if true; then echo 'Inside'; fi # if block end"
test_case 'comment only line with tokens' "# if elif else fi ;"
test_case 'comment after empty command' "; # Empty command before this comment"
test_case 'multiple hash symbols in a comment' "# ########## This is a comment"
test_case 'comment after a word with hash' "echo 'Hash#NotComment'"
test_case 'comment after invalid syntax' "; # This should not execute"
test_case 'comment at the end of script' "echo 'End of script' #"

# Command tests
test_case 'nonexistent command' "nonexistent_command"
test_case 'command in PATH' "/bin/echo 'Command in PATH'"
test_case 'relative path command' "../tests/step_1/simple_command.sh"
test_case 'no args' "mv"
test_case 'ls current directory' "ls"
test_case 'print working directory' "pwd"

# Tricky wrong command
test_case 'tricky wrong command' 'prout
echo "CacaProut"' 

delete_logs

print_result
