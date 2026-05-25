#!/bin/bash
set -e

DEBUG_FLAG=""
if [[ "$1" == "-DDEBUG" ]]; then
    DEBUG_FLAG="-DDEBUG_BUILD=ON -DENABLE_SANITIZERS=ON"
    shift 
fi

if [ ! -d build ]; then
    cmake -B build -G Ninja -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache "$DEBUG_FLAG" "$@"
fi

cmake --build build --parallel
ccache -s