cmake_minimum_required(VERSION 3.10)

project(dengine VERSION 0.3.5 LANGUAGES C)

#this is just a slight nuance with cmake rather than renaming the project
set(DENGINE_VERSION "${dengine_VERSION}")

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
        else()
            message(WARNING ">>> Could not find GTK3. No OS GUI support!")
        endif()

        pkg_check_modules(fontconfig fontconfig)
        if(fontconfig_LIBRARIES)
            message(">>> Found fontconfig libraries")
            include_directories(${fontconfig_INCLUDE_DIRS})
            link_directories(${fontconfig_LIBRARY_DIRS})
            set(DENGINE_HAS_FONTCONFIG 1)
        else()
            #most if not all distros have fontconfig
            message(WARNING ">>> Could not find fontconfig. Default font support will not work! Consider installing it")
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
        ${PROJECT_BINARY_DIR}/dengine_config #dengine_config.h
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/dengine-core/include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/dengine-utils/include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/dengine-gui/include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/dengine-model/include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/dengine-scene/include
        #${CMAKE_CURRENT_SOURCE_DIR}/lib/dengine-script/include
        )

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

configure_file(dengine_config.h.in dengine_config/dengine_config.h)

add_subdirectory(deps/glad "glad-build")
add_subdirectory(lib/dengine-utils "dengine-utils-build")
add_subdirectory(lib/dengine-core "dengine-core-build")
add_subdirectory(lib/dengine-gui "dengine-gui-build")
add_subdirectory(lib/dengine-model "dengine-model-build")
add_subdirectory(lib/dengine-scene "dengine-scene-build")
#add_subdirectory(lib/dengine-script "dengine-script-build")

set(dengine dengine-utils dengine-core dengine-gui dengine-scene)

add_subdirectory(tests "tests-build")

#please ignore this
if(DENGINE_BUILD_TESTS_PRIV)
    add_subdirectory(testspriv)
endif()


