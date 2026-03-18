cmake_minimum_required(VERSION 3.28)

project(stdlubex_lib)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
set(CMAKE_EXE_LINKER_FLAGS "-nostdlib -nostartfiles -ffreestanding -masm=intel")

set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "${SRC_DIR}/*.c")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build")

set(CMAKE_C_OUTPUT_EXTENSION ".o")

add_library(stdlubex_lib STATIC ${SRC_FILES})
target_include_directories(stdlubex_lib PUBLIC
    "${CMAKE_SOURCE_DIR}/includes"
    "${CMAKE_SOURCE_DIR}/../runtime/includes"
)

target_include_directories(stdlubex_lib PUBLIC "${INCLUDE_DIR}")
target_compile_options(stdlubex_lib PRIVATE -ffreestanding -fno-builtin -Wall -Wextra -ffunction-sections -fdata-sections)

set_target_properties(stdlubex_lib PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build"
)