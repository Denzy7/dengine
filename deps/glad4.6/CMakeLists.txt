cmake_minimum_required(VERSION 3.10)

project(glad VERSION 0.1 LANGUAGES C)

include_directories(
include/
)

add_library(glad src/glad.c)

