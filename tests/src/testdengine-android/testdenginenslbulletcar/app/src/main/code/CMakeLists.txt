cmake_minimum_required(VERSION 3.10)

project(DengineAndroid VERSION 0.1)

#export on create to prevent trimming by linker
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

set(DENGINE_BUILD "dengine-build")
#relative to this file
set(DENGINE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../..")
set(DengineRC_DIR ${DENGINE_DIR}/build/dengine-rc)

add_subdirectory(
${DENGINE_DIR}  ${DENGINE_BUILD}
)

#extract some important vars
get_directory_property(dengine_incl DIRECTORY ${PROJECT_BINARY_DIR}/${DENGINE_BUILD} DEFINITION dengine_incl)
get_directory_property(dengine DIRECTORY ${PROJECT_BINARY_DIR}/${DENGINE_BUILD} DEFINITION dengine)
include_directories(
	${dengine_incl}
	${BULLETSRC}/src)

#build bullet and nsl
add_subdirectory(${BULLETSRC} "bullet-build")
add_library(nsl_bulletcar_android ${DENGINE_DIR}/lib/dengine-script/src/nsl-bulletphysics/car.cpp)
target_link_libraries(nsl_bulletcar_android ${dengine} LinearMath BulletCollision BulletDynamics)

# now build app's shared lib and res
set(DengineRC_DIR ${DENGINE_DIR}/build/dengine-rc)
find_package(DengineRC)
if(NOT DengineRC_FOUND)
    message(FATAL_ERROR "Could not find DengineRC. Ensure its been built in ${denginerc_loc}")
endif()
set(inresname ${DENGINE_DIR}/tests/src/testdengine-script/rover.obj)
get_filename_component(barename ${inresname} NAME)
set(outresname ${CMAKE_CURRENT_BINARY_DIR}/${barename}-res.c)
add_custom_command(
	OUTPUT ${outresname}
	COMMAND dengine-rc ${inresname} ${outresname}
	DEPENDS ${inresname}
	VERBATIM
)
add_library(GFX SHARED
	main.c
	${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
	${outresname}
)
target_link_libraries(GFX PRIVATE ${dengine} android log)

