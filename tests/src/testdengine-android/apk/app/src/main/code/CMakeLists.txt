cmake_minimum_required(VERSION 3.10)

project(DengineTest VERSION 0.1)
#export on create to prevent trimming by linker
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

set(DENGINE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../")
set(DengineRC_DIR ${DENGINE_DIR}/build/dengine-rc)
set(DENGINE_BUILD "dengine-build")

find_package(DengineRC)
if(NOT DengineRC_FOUND)
    message(FATAL_ERROR "Could not find DengineRC. Ensure its been built in ${denginerc_loc}")
endif()

add_subdirectory(
    ${DENGINE_DIR} ${DENGINE_BUILD}
)

#extract some important vars
get_directory_property(dengine_incl DIRECTORY ${PROJECT_BINARY_DIR}/${DENGINE_BUILD} INCLUDE_DIRECTORIES)
get_directory_property(dengine DIRECTORY ${PROJECT_BINARY_DIR}/${DENGINE_BUILD} DEFINITION dengine)

#build bullet and nsl and resource
set(BULLETSRC ${DENGINE_DIR}/deps/bullet3_src)
if(NOT EXISTS ${BULLETSRC}/CMakeLists.txt)
    message(FATAL_ERROR "we need submodule deps/bullet_src, update it to build. see deps/README.md in repo for help")
endif()

# now build app's shared lib
add_library(GFX SHARED
    #loader
    main.c

    #tests
    ${DENGINE_DIR}/tests/src/testdengine-lighting/lighting-normalmap.c
    ${DENGINE_DIR}/tests/src/testdengine-lighting/lighting-standard.c
    ${DENGINE_DIR}/tests/src/testdengine-gui/textbench.c
    ${DENGINE_DIR}/tests/src/testdengine-script/script-nslbullet-car.c
    ${DENGINE_DIR}/tests/src/testdengine-scene/scene-ecs.c

    #android glue
	${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
)
target_include_directories(GFX PRIVATE 
    ${dengine_incl}
    ${BULLETSRC}/src
    ${ANDROID_NDK}/sources
    )
target_link_libraries(GFX PRIVATE ${dengine} android log)

set(requested_nsl nsl-test nsl-bulletphysics-car)
foreach(nsl ${requested_nsl})
    set_target_properties(${nsl} PROPERTIES
        PREFIX ""
        SUFFIX ".nsl.so")
endforeach()

