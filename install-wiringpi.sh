#!/bin/bash

printf "Installing WiringPi\n"
git clone https://github.com/OpenStratos/WiringPi.git > /dev/null
cd WiringPi
sudo ./build > /dev/null
cd ..
