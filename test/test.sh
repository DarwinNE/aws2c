#!/bin/bash

printf "\nAWS2C test suite\n"
printf "Davide Bucci 2020\n\n"

cd test_dp || printf "Can not run Dawn Patrol tests.\n"
./test_dp.sh
cd ..

cd test_iunnuh  || printf "Can not run Iunnuh tests.\n"
./test_iunnuh.sh
cd ..

cd test_iunnuh2  || printf "Can not run Iunnuh2 tests.\n"
./test_iunnuh2.sh
cd ..
