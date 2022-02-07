cmake_minimum_required(VERSION 3.10)

project(DengineAndroid VERSION 0.1)

#export on create to prevent trimming by linker
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -Wall ")

set(DENGINE_BUILD "dengine-build")
#relative to this file
set(DENGINE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../..")

add_subdirectory(
${DENGINE_DIR}  ${DENGINE_BUILD}
)

# now build app's shared lib
add_library(GFX SHARED
	main.c
	${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c #native glue
)

target_include_directories(GFX PUBLIC
		${ANDROID_NDK}/sources/android/native_app_glue
		"${PROJECT_BINARY_DIR}/${DENGINE_BUILD}"
		${DENGINE_DIR}/lib/dengine-core/include
		${DENGINE_DIR}/lib/dengine-utils/include
		${DENGINE_DIR}/deps/cglm/include
		)


target_link_libraries(GFX PUBLIC log EGL GLESv3 android dengine-core dengine-utils)

