cmake_minimum_required(VERSION 3.16)
project(PendulumLab5Var19 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Pendulum_Lab5 WIN32 main.cpp)
target_link_libraries(Pendulum_Lab5 PRIVATE gdi32 user32)
