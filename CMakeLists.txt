cmake_minimum_required(VERSION 3.15)
project(para)
include_directories(${YOUR_DIRECTORY})
find_package(Threads REQUIRED)
set(CMAKE_C_STANDARD 11)
set(SOURCES main.c lib/bitmap.c lib/bitmap.h)
add_executable(test ${SOURCES})
find_package (Threads)

add_executable(ThreadImage ${SOURCES})
target_link_libraries(test Threads::Threads)
target_link_libraries (ThreadImage ${CMAKE_THREAD_LIBS_INIT})