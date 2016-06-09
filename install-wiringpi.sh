#!/bin/bash

printf "Installing WiringPi\n"
git clone git://git.drogon.net/wiringPi > /dev/null
cd wiringPi
sudo ./build > /dev/null
cd ..
