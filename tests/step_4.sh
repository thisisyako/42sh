#!/bin/sh

source ./functions.sh

#for file in step_4/*; do
#  test_script ./$file 
#done

# case

test_case "Simple match" '  
case "apple" in  
    apple) echo "Matched Apple" ;;  
    banana) echo "Matched Banana" ;;  
    *) echo "No Match" ;;  
esac  
'  

test_case "Default case with * pattern" '  
case "grape" in  
    apple) echo "Matched Apple" ;;  
    banana) echo "Matched Banana" ;;  
    *) echo "No Match" ;;  
esac  
'  

test_case "Multiple patterns for the same case" '  
case "banana" in  
    apple|banana|cherry) echo "Fruit Matched" ;;  
    orange) echo "Matched Orange" ;;  
    *) echo "No Match" ;;  
esac  
'

test_case "Variable expansion" '  
fruit="banana"  
case "$fruit" in  
    apple) echo "Apple" ;;  
    banana) echo "Banana" ;;  
    *) echo "Unknown" ;;  
esac  
'  

test_case "Command substitution in case" '  
case "$(echo apple)" in  
    apple) echo "Apple Found" ;;  
    orange) echo "Orange Found" ;;  
    *) echo "Unknown" ;;  
esac  
'  

test_case "Matching string with spaces" '  
case "hello world" in  
    "hello world") echo "Matched" ;;  
    *) echo "No Match" ;;  
esac  
'  

test_case "Escaped characters in case" '  
case "hello\nworld" in  
    "hello\nworld") echo "Matched" ;;  
    *) echo "No Match" ;;  
esac  
'  

test_case "Nested case statements" '  
value="animal"  
case "$value" in  
    animal)  
        case "dog" in  
            dog) echo "Animal: Dog" ;;  
            cat) echo "Animal: Cat" ;;  
        esac  
        ;;  
    fruit)  
        echo "Its a fruit" ;;  
    *) echo "Unknown" ;;  
esac  
'  

test_case "Case inside a function" '  
check_fruit() {  
    case "$1" in  
        apple|banana) echo "Its a fruit" ;;  
        carrot) echo "Its a vegetable" ;;  
        *) echo "Unknown" ;;  
    esac  
}  
check_fruit "banana"  
'  

test_case "Empty string case match" '  
case "" in  
    "") echo "Empty string matched" ;;  
    *) echo "No Match" ;;  
esac  
'  

test_case "No matching pattern (silent case)" '  
case "xyz" in  
    abc) echo "Matched ABC" ;;  
    def) echo "Matched DEF" ;;  
esac  
'  

test_case "Syntax error - missing esac" '  
case "apple" in  
    apple) echo "Apple Matched"  
'  


# aliases

# Field Splitting

delete_logs
print_result
