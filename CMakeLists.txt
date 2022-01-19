cmake_minimum_required(VERSION 3.10)

project(dengine VERSION 0.1 LANGUAGES C)

if(UNIX)
    find_package(PkgConfig)
    if(PkgConfig_FOUND)
        pkg_check_modules(GTK3 gtk+-3.0)
        if(GTK3_LIBRARIES)
            message("Found GTK3 Libraries")
            include_directories(${GTK3_INCLUDE_DIRS})
            link_directories(${GTK3_LIBRARY_DIRS})
            set(DENGINE_HAS_GTK3 1)
        else()
            message(WARNING "Could not find GTK3. No OS GUI support!")
        endif()
    else()
        message(WARNING "Could not find pkg-config. Disabled=GTK")
    endif()
endif()

if(NOT ANDROID)
    find_package(glfw3)
    if(glfw3_FOUND)
        message("Using System GLFW")
    else()
        message("Compile GLFW source")
    endif()
endif()

include_directories(
        ${CMAKE_SOURCE_DIR}/deps/stb
        ${CMAKE_SOURCE_DIR}/deps/glad4.6/include
        ${CMAKE_SOURCE_DIR}/deps/cglm/include
        ${CMAKE_BINARY_DIR} #dengine_config.h
        ${CMAKE_SOURCE_DIR}/lib/dengine-core/include
        ${CMAKE_SOURCE_DIR}/lib/dengine-core/include/dengine
        ${CMAKE_SOURCE_DIR}/lib/dengine-utils/include
        ${CMAKE_SOURCE_DIR}/lib/dengine-utils/include/dengine-utils
        )

if(NOT ANDROID)
    add_subdirectory(deps/glad4.6)
endif()

configure_file(dengine_config.h.in dengine_config.h)

add_subdirectory(lib/dengine-utils)
add_subdirectory(lib/dengine-core)

add_subdirectory(tests)

