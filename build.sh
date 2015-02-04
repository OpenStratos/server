#!/bin/bash

aclocal
autoheader
automake --add-missing
autoconf
./configure
make utesting

printf "\n\n----- Starting unit tests -----\n\n"
./utesting