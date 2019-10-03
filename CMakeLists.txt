cmake_minimum_required(VERSION 3.10)
project(MARS_2)

set(CMAKE_CXX_STANDARD 14)
add_executable(MARS_2 main.cpp MatriceLoader.cpp MatriceLoader.h)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(MARS_2 Threads::Threads)

