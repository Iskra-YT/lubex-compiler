#!/bin/bash
set -e

rm -r build
mkdir build

cd compiler
./build.sh "$@"
cd ..
cp compiler/build/lubex build/lubex

cd runtime
./build.sh
cd ..
cp ./runtime/build/liblubrtx_lib.a ./build/lubrtx-lib.a
