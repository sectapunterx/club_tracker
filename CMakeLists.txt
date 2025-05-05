cmake_minimum_required(VERSION 3.24)
project(club_tracker LANGUAGES CXX)

# --------------------------------------------------
#  gtest
include(FetchContent)
FetchContent_Declare(googletest
        URL https://github.com/google/googletest/archive/refs/heads/main.zip)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# --------------------------------------------------
#  Компилятор
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -pedantic -Wconversion -Wno-sign-conversion)

# --------------------------------------------------
file(GLOB_RECURSE CC_CORE_SOURCES CONFIGURE_DEPENDS
        ${PROJECT_SOURCE_DIR}/src/*.cpp)

list(REMOVE_ITEM CC_CORE_SOURCES ${PROJECT_SOURCE_DIR}/src/main.cpp)

add_library(cc_core STATIC ${CC_CORE_SOURCES})

target_include_directories(cc_core PUBLIC
        ${PROJECT_SOURCE_DIR}/include)
target_include_directories(cc_core PRIVATE
        ${PROJECT_SOURCE_DIR}/src)

# --------------------------------------------------
add_executable(task src/main.cpp)
target_link_libraries(task PRIVATE cc_core)

# --------------------------------------------------
#  Tests
enable_testing()

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/tests/*.cpp)
add_executable(core_tests ${TEST_SOURCES})
target_link_libraries(core_tests PRIVATE cc_core gtest_main)

include(GoogleTest)
gtest_discover_tests(core_tests)
