#!/bin/bash

mkdir -p bin/scripts
gcc -Wall -Wno-unknown-pragmas src/server.c -o bin/server -lm -llua -ldl
cp src/*.lua bin/scripts/
