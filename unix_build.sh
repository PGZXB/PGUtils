#! /bin/bash

# Allways DEBUG Mode (developing)
mkdir -p build
cd build && cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=Debug && make -j && cd ..