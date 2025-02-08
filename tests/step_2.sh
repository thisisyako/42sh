#!/bin/sh

source ./functions.sh

for file in step_2/*; do
  test_script ./$file 
done

# Redirections, '>' | '<' | '>>' | '>&' | '<&' | '>|' | '<>'

# Pipelines |

# Negation
test_case 'if with negated false' 'if ! false; then echo if_true ; else echo if_false; fi'

# “while” and “until” Commands.

# Operators
test_case 'OR operator with first command success' 'echo Hello || echo Fail'
test_case 'OR operator with first command failure' 'false || echo Fail'
test_case 'AND operator with first command success' 'echo Hello && echo Success'
test_case 'AND operator with first command failure' 'false && echo Fail'

# Doubles quotes and escape character.

# Variables
test_case 'Variable assignation' 'a=3; echo $a; a=123; echo $a'
test_case 'Last command exit code' 'echo "Exit code: $?"'
test_case 'Current process ID' 'echo "Process ID: $$" > tmp.txt; rm tmp.txt'
test_case 'Random number generation' 'echo $RANDOM > tmp.txt; rm tmp.txt'
test_case 'Current user ID' 'echo "User ID: $UID"'
test_case 'Old and current working directories' 'echo "Previous directory: $OLDPWD; Current directory: $PWD"'
test_case 'Internal field separator' 'echo $IFS | od -c'
test_case '$0 special variable' 'echo $0' 'MyArg1'
test_case '$1 special variable' 'echo $1' 'MyArg1' 'MyArg2'
test_case '$2 special variable' 'echo $2' 'MyArg1' 'MyArg2' 'MyArg3'
test_case '$# special variable' 'echo $#' 'MyArg1' 'MyArg2' 'MyArg3'
test_case '$@ special variable' 'echo $@' 'MyArg1' 'MyArg2' 'MyArg3'
test_case '$@ special variable double quoted' 'echo "$@"' 'MyArg1' 'MyArg2' 'MyArg3'
test_case '$@ special variable with prefix and suffix' 'echo adaw$@bb test' 'MyArg1' 'MyArg2' 'MyArg3'
test_case '$@ special variable double quoted and prefix suffix' 'echo "adaw$@bb" test' 'MyArg1' 'MyArg2' 'MyArg3'
test_case '$* special variable quoted with prefix and suffix' 'echo "adaw$*bb" test' 'MyArg1' 'MyArg2' 'MyArg3'
test_case '$* special variable no quotes prefix suffix' 'echo adaw$*bb test' 'MyArg1' 'MyArg2' 'MyArg3'
test_case '$* special variable quoted' 'echo "$*"' 'MyArg1' 'MyArg2' 'MyArg3'
test_case '$* special variable no quotes' 'echo $*' 'MyArg1' 'MyArg2' 'MyArg3'
test_case 'bad variable' 'echo $2invalid'

# for
test_case 'simple for echo' 'for x in hello world test caca; do echo $x; done'
test_case 'more complex' 'for x in "$@"; do echo $x; done zsh arg1 arg2 arg3 arg4 arg5 arg6'
test_case 'list fruits' 'for fruit in apple banana orange grape; do echo $fruit; done'
test_case 'list colors' 'for color in red blue green yellow purple; do echo $color; done'
test_case 'list numbers' 'for num in one two three four five; do echo $num; done'
test_case 'list days' 'for day in Monday Tuesday Wednesday Thursday Friday; do echo $day; done'
test_case 'list cities' 'for city in Paris London Tokyo Berlin Rome Madrid; do echo $city; done'
test_case 'list countries' 'for country in France Germany Italy Spain UK; do echo $country; done'

delete_logs
print_result
