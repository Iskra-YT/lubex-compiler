@echo off
IF EXIST build (
    rmdir /s /q build
)
mkdir build
cd build
cmake .. %*
cmake --build .
cd ..
