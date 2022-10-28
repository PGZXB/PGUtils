#! /bin/bash

# Allways DEBUG Mode (developing)
mkdir -p build
cd build && cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DPGZXB_DEBUG:BOOL=ON && make -j && cd ..
