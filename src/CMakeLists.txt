cmake_minimum_required(VERSION 3.10)
project(SATSolver)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(sat_solver
    main.cpp
    Solver.cpp
    Solver.h
)
