cmake_minimum_required(VERSION 4.3)
project(IS_Labs)

set(CMAKE_CXX_STANDARD 20)

add_executable(lab_1
        lab_1/main.cpp
        lab_1/simple_test.cpp
)

add_executable(lab_1_bonus
        lab_1_bonus/findDecision.cpp
        lab_1_bonus/findDecision.h
        lab_1_bonus/main.cpp
)

add_executable(lab_2
        lab_2/CrazyPyatnashki.cpp
        lab_2/CrazyPyatnashki.h
        lab_2/main.cpp
)