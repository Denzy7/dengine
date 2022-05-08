cmake_minimum_required(VERSION 3.10)

project(dengine VERSION 0.5.0 LANGUAGES C)

#this is just a slight nuance with cmake rather than renaming the project
set(DENGINE_VERSION "${dengine_VERSION}")
include(GNUInstallDirs)

set(DENGINE_PKG_LIBS)
set(DENGINE_PKG_DEPS)

if(NOT ANDROID)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        #link desktop libs
        pkg_check_modules(GTK3 gtk+-3.0)
        if(GTK3_LIBRARIES)
            message(">>> Found GTK3 libraries")
            include_directories(${GTK3_INCLUDE_DIRS})
            link_directories(${GTK3_LIBRARY_DIRS})
            set(DENGINE_HAS_GTK3 1)
            string(APPEND DENGINE_PKG_DEPS "gtk+-3.0 ")
        else()
            message(WARNING ">>> Could not find GTK3. No OS GUI support!")
        endif()

        pkg_check_modules(fontconfig fontconfig)
        if(fontconfig_LIBRARIES)
            message(">>> Found fontconfig libraries")
            include_directories(${fontconfig_INCLUDE_DIRS})
            link_directories(${fontconfig_LIBRARY_DIRS})
            set(DENGINE_HAS_FONTCONFIG 1)
            string(APPEND DENGINE_PKG_DEPS "fontconfig ")
        else()
            #most if not all distros have fontconfig
            message(WARNING ">>> Could not find fontconfig. Default font support will not work! Consider installing it")
        endif()

        pkg_check_modules(python3 python3-embed)
        if(python3_LIBRARIES)
            message(">>> Found python3 libraries")
            include_directories(${python3_INCLUDE_DIRS})
            link_directories(${python3_LIBRARY_DIRS})
            set(DENGINE_HAS_PYTHON3 1)
            string(APPEND DENGINE_PKG_DEPS "python3-embed ")
        else()
            #most if not all distros have python3
            message(WARNING ">>> Could not find python3. Python scripting will not work! Consider installing it")
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
        string(APPEND DENGINE_PKG_DEPS "glfw3 ")
    endif()
endif()

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/stb
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/cglm/include
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/glad/include
        ${PROJECT_BINARY_DIR}/dengine_config #dengine_config.h
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/dengine_state #dengine_state.h
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/dengine-core/include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/dengine-utils/include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/dengine-gui/include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/dengine-model/include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/dengine-scene/include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/dengine-script/include
        )

if(NOT python3_LIBRARIES)
    set(PYSRC ${CMAKE_CURRENT_SOURCE_DIR}/deps/cpython)
    set(PYVER "3.9")
    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/cpython/Include
        ${CMAKE_CURRENT_BINARY_DIR}/deps/cpython-portable/config
    )
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/deps/cpython-portable)
    set(python3_LIBRARIES cpython-portable)
endif()


option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

option(DENGINE_FONT_EMBED_OPENSANS_REGULAR "Embed OpenSans Regular into dengine-gui" OFF)
option(DENGINE_FONT_EMBED_OPENSANS_ITALIC "Embed OpenSans Italic into dengine-gui" OFF)
option(DENGINE_FONT_EMBED_OPENSANS_LIGHT "Embed OpenSans Light into dengine-gui" ON)

option(DENGINE_FONT_EMBED_SOURCESANSPRO_REGULAR "Embed Source Sans Pro Regular into dengine-gui" OFF)
option(DENGINE_FONT_EMBED_SOURCESANSPRO_ITALIC "Embed Source Sans Pro Italic into dengine-gui" OFF)
option(DENGINE_FONT_EMBED_SOURCESANSPRO_LIGHT "Embed Source Sans Pro Light into dengine-gui" OFF)

option(DENGINE_FONT_EMBED_ROBOTO_REGULAR "Embed Roboto Regular into dengine-gui" OFF)
option(DENGINE_FONT_EMBED_ROBOTO_ITALIC "Embed Roboto Italic into dengine-gui" OFF)
option(DENGINE_FONT_EMBED_ROBOTO_LIGHT "Embed Roboto Light into dengine-gui" OFF)

option(DENGINE_BUILD_TESTS "Build the test suite and examples" ON)

option(DENGINE_MT "Build for multithreading using mutexes" ON)

include(CheckFunctionExists)
include(CheckIncludeFile)

check_function_exists(clock_gettime DENGINE_HAS_CLOCK_GETTIME)
check_include_file(pthread.h DENGINE_HAS_PTHREAD_H)

configure_file(dengine_config.h.in dengine_config/dengine_config.h)

add_subdirectory(lib/dengine-utils "dengine-utils-build")
add_subdirectory(lib/dengine-core "dengine-core-build")
add_subdirectory(lib/dengine-gui "dengine-gui-build")
add_subdirectory(lib/dengine-model "dengine-model-build")
add_subdirectory(lib/dengine-script "dengine-script-build")
add_subdirectory(lib/dengine-scene "dengine-scene-build")

set(dengine dengine-utils dengine-core dengine-gui dengine-scene dengine-script)

if(DENGINE_BUILD_TESTS)
    add_subdirectory(tests "tests-build")
endif()

#please ignore this
if(DENGINE_BUILD_TESTS_PRIV)
    add_subdirectory(testspriv)
endif()

add_subdirectory(main/dengitor)
if(WIN32)
    add_subdirectory(main/python-dengine-script)
endif()

foreach(i ${dengine})
    string(APPEND DENGINE_PKG_LIBS "-l${i} ")
endforeach()

configure_file(dengine.pc.in dengine.pc @ONLY)

install(FILES ${PROJECT_BINARY_DIR}/dengine_config/dengine_config.h DESTINATION include/dengine-${DENGINE_VERSION}/dengine_config)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/lib/dengine_state/dengine_state.h DESTINATION include/dengine-${DENGINE_VERSION}/lib/dengine_state)
install(DIRECTORY deps/glad/include/glad DESTINATION include/dengine-${DENGINE_VERSION}/deps/glad)
install(DIRECTORY deps/glad/include/KHR DESTINATION include/dengine-${DENGINE_VERSION}/deps/glad)
install(DIRECTORY deps/cglm/include/cglm DESTINATION include/dengine-${DENGINE_VERSION}/deps/cglm)

# pkgconf
install(FILES "${PROJECT_BINARY_DIR}/dengine.pc" DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
