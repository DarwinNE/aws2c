#!/bin/bash

red_code="\033[1;31m"
green_code="\033[32m"
normal_code="\033[0m"

printf "Compiling Iunnuh with no options........... "
cd ../../Iunnuh

make clean >/dev/null 2>/dev/null

if make modern >compile_results.txt 2>compile_errors.txt
then
    printf $green_code"PASS\n"$normal_code
else
    printf $red_code"FAIL\n"$normal_code
fi

printf "Running Iunnuh with walktrough............. "

./iunnuh <soluzione.txt >test_results.txt

# Search for a sentence present in the message shown when the game is complete

if grep -q "Hai risolto l'avventura." test_results.txt
then
    printf $green_code"PASS\n"$normal_code
else
    printf $red_code"FAIL\n"$normal_code
fi


printf "Compiling Iunnuh with -c -d -s -m.......... "

if make modern_alt >compile_results.txt 2>compile_errors.txt
then
    printf $green_code"PASS\n"$normal_code
else
    printf $red_code"FAIL\n"$normal_code
    exit
fi

printf "Running Iunnuh with walktrough............. "

./iunnuh_a <soluzione.txt >test_results.txt

# Search for a sentence present in the message shown when the game is complete

if grep -q "Hai risolto l'avventura." test_results.txt
then
    printf $green_code"PASS\n"$normal_code
else
    printf $red_code"FAIL\n"$normal_code
    exit
fi