#!/bin/bash

rm -rf core
./run-bug
if [ -f "core" ]; then
    gdb -batch -x bt.gdb ../src/man core
fi
