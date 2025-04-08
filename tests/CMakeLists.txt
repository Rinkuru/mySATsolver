cmake_minimum_required(VERSION 3.10)
project(MyTests)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(test_file 
    test_all.cpp    
    Solver.cpp
    Solver.h
)
target_link_libraries(test_file GTest::GTest GTest::Main pthread)
