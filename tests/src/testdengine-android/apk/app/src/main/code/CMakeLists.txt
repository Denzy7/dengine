cmake_minimum_required(VERSION 3.10)

project(DengineTest VERSION 0.1)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
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
get_directory_property(dengine_incl DIRECTORY ${PROJECT_BINARY_DIR}/${DENGINE_BUILD} DEFINITION dengine_incl)
get_directory_property(dengine DIRECTORY ${PROJECT_BINARY_DIR}/${DENGINE_BUILD} DEFINITION dengine)

#build bullet and nsl and resource
set(BULLETSRC ${DENGINE_DIR}/deps/bullet3_src)
if(NOT EXISTS ${BULLETSRC}/CMakeLists.txt)
    message(FATAL_ERROR "we need submodule deps/bullet_src, update it to build. see deps/README.md in repo for help")
endif()

add_subdirectory(${BULLETSRC} "bullet-build")
set(nslbulletcar_res
    ${DENGINE_DIR}/lib/dengine-script/src/nsl-bulletphysics/rover.obj
    ${DENGINE_DIR}/lib/dengine-script/src/nsl-bulletphysics/wheelmap.jpg
    )
set(nslbulletcar_outres)
foreach(res ${nslbulletcar_res})
    get_filename_component(barename ${res} NAME)
    # inres and outres should be absolute paths for fair measure
    set(inresname ${res})
    get_filename_component(barename ${res} NAME)
    set(outresname ${CMAKE_CURRENT_BINARY_DIR}/${barename}-res.c)
    add_custom_command(
        OUTPUT ${outresname}
        COMMAND dengine-rc ${inresname} ${outresname}
        DEPENDS ${inresname}
        VERBATIM
        )
    list(APPEND nslbulletcar_outres ${outresname})
endforeach()

add_library(nsl_bulletcar_android 
    ${DENGINE_DIR}/lib/dengine-script/src/nsl-bulletphysics/car.cpp
    ${DENGINE_DIR}/lib/dengine-script/src/nsl-bulletphysics/denginebulletcommon.cpp
    ${nslbulletcar_outres}
)
target_include_directories(nsl_bulletcar_android PRIVATE 
    ${dengine_incl}
    ${BULLETSRC}/src
    ${ANDROID_NDK}/sources
    )
target_link_libraries(nsl_bulletcar_android ${dengine} LinearMath BulletCollision BulletDynamics)

# now build app's shared lib
add_library(GFX SHARED
    #loader
    main.c

    #tests
    ${DENGINE_DIR}/tests/src/testdengine-lighting/lighting-normalmap.c
    ${DENGINE_DIR}/tests/src/testdengine-lighting/lighting-standard.c
    ${DENGINE_DIR}/tests/src/testdengine-script/script-nslbullet-car.c

    #android glue
	${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
)

target_include_directories(GFX PRIVATE 
    ${dengine_incl}
    ${BULLETSRC}/src
    ${ANDROID_NDK}/sources
    )
target_link_libraries(GFX PRIVATE ${dengine} android log)

