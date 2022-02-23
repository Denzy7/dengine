cmake_minimum_required(VERSION 3.10)

project(dengine VERSION 0.1 LANGUAGES C)

if(NOT ANDROID)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        #link desktop libs
            pkg_check_modules(GTK3 gtk+-3.0)
            if(GTK3_LIBRARIES)
                message(">>> Found GTK3 Libraries")
                include_directories(${GTK3_INCLUDE_DIRS})
                link_directories(${GTK3_LIBRARY_DIRS})
                set(DENGINE_HAS_GTK3 1)
            else()
                message(WARNING ">>> Could not find GTK3. No OS GUI support!")
            endif()
    else()
        message(WARNING ">>> Could not find pkg-config. Will compile From sources")
    endif()

    find_package(glfw3 QUIET)
    if(glfw3_FOUND)
        message(">>> Using system GLFW3")
    else()
        message(">>> Compile GLFW3 sources")
        add_subdirectory(deps/glfw "glfw3-build")
        include_directories(deps/glfw/include)
    endif()
endif()

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/stb
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/cglm/include
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/glad/include
        ${PROJECT_BINARY_DIR} #dengine_config.h
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/dengine-core/include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/dengine-core/include/dengine
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/dengine-utils/include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/dengine-utils/include/dengine-utils
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/dengine-gui/include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/dengine-gui/include/dengine-gui
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/dengine-model/include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/dengine-model/include/dengine-model
        )

configure_file(dengine_config.h.in dengine_config.h)

add_subdirectory(deps/glad "glad-build")
add_subdirectory(lib/dengine-utils "dengine-utils-build")
add_subdirectory(lib/dengine-core "dengine-core-build")
add_subdirectory(lib/dengine-gui "dengine-gui-build")
add_subdirectory(lib/dengine-model "dengine-model-build")

add_subdirectory(tests "tests-build")


