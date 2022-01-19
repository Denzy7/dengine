cmake_minimum_required(VERSION 3.10)

include_directories(
include/
)

project(glad VERSION 0.1)

add_library(glad src/glad.c)

