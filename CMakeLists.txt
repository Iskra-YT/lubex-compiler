cmake_minimum_required(VERSION 3.28)
project(lubex VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(${CMAKE_SOURCE_DIR}/includes)

file(GLOB_RECURSE LIB_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")
list(FILTER LIB_SOURCES EXCLUDE REGEX ".*src/main\\.cpp$")

add_library(lubex_lib ${LIB_SOURCES})
target_include_directories(lubex_lib PUBLIC ${CMAKE_SOURCE_DIR}/includes)

add_executable(lubex src/main.cpp)
target_link_libraries(lubex PRIVATE lubex_lib)

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

file(GLOB_RECURSE TEST_SOURCES "${CMAKE_SOURCE_DIR}/tests/*.cpp")

add_executable(lubex_tests ${TEST_SOURCES})

target_link_libraries(lubex_tests
    PRIVATE
    lubex_lib
    gtest
    gtest_main
)

target_include_directories(lubex_tests PRIVATE ${CMAKE_SOURCE_DIR}/includes)

include(GoogleTest)
gtest_discover_tests(lubex_tests)