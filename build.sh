#!/bin/bash

aclocal
autoheader
automake --add-missing
autoconf
./configure
make utesting
./utesting