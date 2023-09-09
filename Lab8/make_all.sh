#!/bin/sh

cd ./lib
make clean
make all

cd ../kernel
make clean
make all