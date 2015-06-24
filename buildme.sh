#!/bin/sh

mkdir build
(cd build && cmake ..)
make -C build "$@"
