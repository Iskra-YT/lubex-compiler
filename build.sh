#!/bin/bash
set -e

rm -rf build
mkdir build

cd compiler
./build.sh "$@"
cd ..
cp compiler/build/lubex build/lubex

cd runtime
./build.sh
cd ..
cp ./runtime/build/liblubrtx.a ./build/lubrtx-lib.a

cd standard
./build.sh
cd ..
cp ./standard/build/libstdlubex_lib.a ./build/stdlubex-lib.a
