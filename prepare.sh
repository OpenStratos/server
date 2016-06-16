#!/bin/bash

aclocal
autoheader
automake --add-missing
autoconf
./configure

mkdir data
mkdir data/logs
mkdir data/logs/GPS
mkdir data/logs/camera
mkdir data/logs/main
mkdir data/video
