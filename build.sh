#!/bin/bash
set -e

if [ ! -d build ]; then
    cmake -B build -G Ninja -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache "$@"
fi

cmake --build build --parallel
ccache -s