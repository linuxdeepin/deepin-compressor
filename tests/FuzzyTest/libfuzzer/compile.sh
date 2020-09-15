#!/bin/bash

mkdir -p build
cd build
cmake .. -DCMAKE_C_COMPILER=/opt/llvm/bin/clang -DCMAKE_CXX_COMPILER=/opt/llvm/bin/clang++
make