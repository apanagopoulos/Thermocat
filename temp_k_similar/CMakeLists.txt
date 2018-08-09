cmake_minimum_required(VERSION 3.7)
project(temp_k_similar)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        main.c
        temp_k_similar.h
        configure.h)

add_executable(temp_k_similar ${SOURCE_FILES})