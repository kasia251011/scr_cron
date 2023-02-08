cmake_minimum_required(VERSION 3.17)
project(project_2 C)

set(CMAKE_C_STANDARD 11)
set(THREADS_PREFER_PTHREAD_FLAG ON)

add_executable(project_2 main.c task_manager.c task_manager.h)
add_executable(child child.c)

find_package(Threads REQUIRED)
target_link_libraries(project_2 ${CMAKE_THREAD_LIBS_INIT} rt)