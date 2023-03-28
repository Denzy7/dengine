cmake_minimum_required(VERSION 3.10)

project(DengineAndroid VERSION 0.1)

#export on create to prevent trimming by linker
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -Wall ")

set(DENGINE_BUILD "dengine-build")
#relative to this file
set(DENGINE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../..")
set(DengineRC_DIR ${DENGINE_DIR}/build/dengine-rc)
add_subdirectory(
${DENGINE_DIR}  ${DENGINE_BUILD}
)

# now build app's shared lib
add_library(GFX SHARED
	main.c
	${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
)

get_directory_property(dengine-incldirs DIRECTORY ${DENGINE_DIR} INCLUDE_DIRECTORIES)

target_include_directories(GFX PUBLIC
                ${dengine-incldirs}
#		${ANDROID_NDK}/sources/android/native_app_glue
		)


target_link_libraries(GFX PUBLIC dengine-core dengine-utils dengine-gui dengine-script)

