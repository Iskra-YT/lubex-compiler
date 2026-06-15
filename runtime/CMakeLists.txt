cmake_minimum_required(VERSION 3.28)
project(lubrtx C CXX)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/includes")

# All sources including lbxentry.c
file(GLOB_RECURSE ALL_SRC_FILES CONFIGURE_DEPENDS "${SRC_DIR}/*.c")

# Core sources excluding lbxentry.c (for tests)
set(CORE_SRC_FILES ${ALL_SRC_FILES})
list(FILTER CORE_SRC_FILES EXCLUDE REGEX ".*src/lbxentry\\.c$")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build")

set(CMAKE_C_OUTPUT_EXTENSION ".o")

# Production library: includes lbxentry.c (_start, __R_mainHeap)
add_library(lubrtx STATIC ${ALL_SRC_FILES})

target_include_directories(lubrtx PUBLIC "${INCLUDE_DIR}")
target_compile_options(lubrtx PRIVATE -ffreestanding -fno-builtin -Wall -Wextra -ffunction-sections -fdata-sections)

set_target_properties(lubrtx PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build"
)

# Core library: without lbxentry.c (for tests)
add_library(lubrtx_core STATIC ${CORE_SRC_FILES})

target_include_directories(lubrtx_core PUBLIC "${INCLUDE_DIR}")
target_compile_options(lubrtx_core PRIVATE -ffreestanding -fno-builtin -Wall -Wextra -ffunction-sections -fdata-sections)

set_target_properties(lubrtx_core PROPERTIES
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
    lubrtx_core
    gtest
    gtest_main
)

target_include_directories(runtime_tests PRIVATE
    "${INCLUDE_DIR}"
)

include(GoogleTest)
gtest_discover_tests(runtime_tests)