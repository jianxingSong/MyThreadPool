cmake_minimum_required(VERSION 3.21)
project(learnThreadPool)

set(CMAKE_CXX_STANDARD 14)

add_executable(learnThreadPool main.cpp)
target_link_libraries(learnThreadPool -pthread)
