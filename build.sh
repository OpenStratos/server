#!/bin/bash

aclocal
autoheader
automake --add-missing
autoconf
./configure
make utesting

mkdir data
mkdir data/logs
mkdir data/logs/GPS
mkdir data/logs/camera
mkdir data/logs/main
mkdir data/video

printf "\n\n----- Starting unit tests -----\n\n"
./utesting
