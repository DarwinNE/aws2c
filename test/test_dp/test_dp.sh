#!/bin/bash

red_code="\033[1;31m"
green_code="\033[32m"
normal_code="\033[0m"

printf "Compiling Dawn Patrol with -d -n options... "
cd ../../DawnPatrol

make clean >/dev/null 2>/dev/null

if make modern >compile_results.txt 2>compile_errors.txt
then
    printf $green_code"PASS\n"$normal_code
else
    printf $red_code"FAIL\n"$normal_code
fi

printf "Running Dawn Patrol with walktrough........ "

./patrol <solution.txt >test_results.txt

# Search for a sentence present in the message shown when the game is complete

if grep -q "mates in time" test_results.txt
then
    printf $green_code"PASS\n"$normal_code
else
    printf $red_code"FAIL\n"$normal_code
fi


printf "Compiling Dawn Patrol with -d -c -m -n -s.. "

if make modern_alt >compile_results.txt 2>compile_errors.txt
then
    printf $green_code"PASS\n"$normal_code
else
    printf $red_code"FAIL\n"$normal_code
    exit
fi

printf "Running Dawn Patrol with walktrough........ "

./patrol_a <solution.txt >test_results.txt

# Search for a sentence present in the message shown when the game is complete

if grep -q "mates in time" test_results.txt
then
    printf $green_code"PASS\n"$normal_code
else
    printf $red_code"FAIL\n"$normal_code
    exit
fi