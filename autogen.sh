#!/bin/sh

aclocal --install -I m4 &&
  autoreconf &&
  automake --add-missing --copy &&
  ./configure "$@"