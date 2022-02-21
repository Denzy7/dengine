cmake_minimum_required(VERSION 3.10)

project(glad VERSION 0.1 LANGUAGES C)

include_directories(
include/
)

add_library(glad src/glad.c)
if(NOT WIN32)
	target_link_libraries(glad dl)
endif()

#remove lib prefix
if(WIN32)
    set_target_properties(glad PROPERTIES PREFIX "")
endif()

