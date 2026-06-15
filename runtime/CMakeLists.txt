cmake_minimum_required(VERSION 3.28)
project(lubrtx_lib C CXX)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/includes")

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "${SRC_DIR}/*.c")
list(FILTER SRC_FILES EXCLUDE REGEX ".*src/lbxentry\\.c$")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build")

set(CMAKE_C_OUTPUT_EXTENSION ".o")

add_library(lubrtx_lib STATIC ${SRC_FILES})

target_include_directories(lubrtx_lib PUBLIC "${INCLUDE_DIR}")
target_compile_options(lubrtx_lib PRIVATE -ffreestanding -fno-builtin -Wall -Wextra -ffunction-sections -fdata-sections)

set_target_properties(lubrtx_lib PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build"
)

# ========================
# Tests (GoogleTest)
# ========================
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(runtime_tests tests/runtime_tests.cpp)

target_link_libraries(runtime_tests PRIVATE
    lubrtx_lib
    gtest
    gtest_main
)

target_include_directories(runtime_tests PRIVATE
    "${INCLUDE_DIR}"
)

include(GoogleTest)
gtest_discover_tests(runtime_tests)