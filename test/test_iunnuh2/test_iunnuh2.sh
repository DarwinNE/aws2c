#!/bin/bash

red_code="\033[1;31m"
green_code="\033[32m"
normal_code="\033[0m"

printf "Compiling Iunnuh2 with -d -m options....... "
cd ../../Iunnuh2

make clean >/dev/null 2>/dev/null

if make modern >compile_results.txt 2>compile_errors.txt
then
    printf $green_code"PASS\n"$normal_code
else
    printf $red_code"FAIL\n"$normal_code
    exit
fi

printf "Running Iunnuh2 with walktrough............ "

./iunnuh2_en <soluzione_en.txt >test_results.txt

# Search for a sentence present in the message shown when the game is complete

if grep -q "the curse is removed!" test_results.txt
then
    printf $green_code"PASS\n"$normal_code
else
    printf $red_code"FAIL\n"$normal_code
    exit
fi


printf "Compiling Iunnuh2 with  -c -d -m -s -n -5.. "

if make modern_alt >compile_results.txt 2>compile_errors.txt
then
    printf $green_code"PASS\n"$normal_code
else
    printf $red_code"FAIL\n"$normal_code
fi

printf "Running Iunnuh2 with walktrough............ "

./iunnuh2_en_a <soluzione_en.txt >test_results.txt

# Search for a sentence present in the message shown when the game is complete

if grep -q "the curse is removed!" test_results.txt
then
    printf $green_code"PASS\n"$normal_code
else
    printf $red_code"FAIL\n"$normal_code
fi