cmake_minimum_required(VERSION 3.10)
project(MARS_CI)

set(CMAKE_CXX_STANDARD 14)
add_executable(MARS_CI main.cpp InputLoader.h Annealing.cpp Annealing.h)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(MARS_CI Threads::Threads)
