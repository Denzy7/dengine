cmake_minimum_required(VERSION 3.10)

project(DengineAndroid VERSION 0.1)

#export on create to prevent trimming by linker
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

set(DENGINE_BUILD "dengine-build")
#relative to this file
set(DENGINE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../..")

add_subdirectory(
${DENGINE_DIR}  ${DENGINE_BUILD}
)

# now build app's shared lib
add_library(GFX SHARED
	main.c
)

target_include_directories(GFX PUBLIC
		${ANDROID_NDK}/sources
		"${PROJECT_BINARY_DIR}/${DENGINE_BUILD}"
		${DENGINE_DIR}/lib/dengine-core/include
		${DENGINE_DIR}/lib/dengine-utils/include
		${DENGINE_DIR}/lib/dengine-gui/include
		${DENGINE_DIR}/deps/cglm/include
		${DENGINE_DIR}/deps/glad/include
		)

target_link_libraries(GFX PUBLIC log EGL glad dengine-core dengine-utils dengine-gui)

