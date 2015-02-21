#!/bin/bash

printf "Installing WiringPi\n"
git clone https://github.com/OpenStratos/WiringPi.git > /dev/null
cd WiringPi
./build > /dev/null
cd ..