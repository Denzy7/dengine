cmake_minimum_required(VERSION 3.23)

project(dengine VERSION 0.8.2)

#this is just a slight nuance with cmake rather than renaming the project
set(DENGINE_VERSION "${dengine_VERSION}")
include(GNUInstallDirs)

set(DENGINE_PKG_LIBS)
set(DENGINE_PKG_DEPS)

# * shadow3d is very expensive! (especially on mobile. you might
# * see up to a 40% perfomance decrease there!) 
# *
# * USE RESPONSIBLY!
# *
# * you have to manually calculate shadow 3d for each light
# * as done below with the compile defines. glsl is kinda
# * messed up when using sampler array. you'll still have
# * to index it manually so...
option(DENGINE_LIGHTING_SHADOW3D "Compile with 3D shadows support. 3D shadows are very expensive" ON)
set(DENGINE_LIGHTING_MAX_POINT "4" CACHE STRING "Compile shaders to support a maximum of these point lights")
set(DENGINE_LIGHTING_MAX_SPOT "4" CACHE STRING "Compile shaders to support a maximum of these spot lights")

option(DENGINE_FONT_EMBED_OPENSANS_REGULAR "Embed OpenSans Regular into dengine-gui" OFF)
option(DENGINE_FONT_EMBED_OPENSANS_ITALIC "Embed OpenSans Italic into dengine-gui" OFF)

option(DENGINE_FONT_EMBED_SOURCESANSPRO_REGULAR "Embed Source Sans Pro Regular into dengine-gui" OFF)
option(DENGINE_FONT_EMBED_SOURCESANSPRO_ITALIC "Embed Source Sans Pro Italic into dengine-gui" OFF)
option(DENGINE_FONT_EMBED_SOURCESANSPRO_LIGHT "Embed Source Sans Pro Light into dengine-gui" OFF)

option(DENGINE_FONT_EMBED_ROBOTO_REGULAR "Embed Roboto Regular into dengine-gui" OFF)
option(DENGINE_FONT_EMBED_ROBOTO_ITALIC "Embed Roboto Italic into dengine-gui" OFF)
option(DENGINE_FONT_EMBED_ROBOTO_LIGHT "Embed Roboto Light into dengine-gui" OFF)

option(DENGINE_BUILD_TESTS "Build the test suite and examples" ON)

option(DENGINE_MT "Build for multithreading using mutexes" ON)

option(DENGINE_SCRIPTING_PYTHON "Enable support for experimental Python scripting. Disable if you get random segfault! Requires relevant submodules to be cloned. See deps/README.md" ON)
option(DENGINE_USE_SYSTEM_PYTHON "Use system python or build custom. Custom build may cause compatibility issue. Check if compatible with deps/cpython_portable. Of course no need to compile python if already on system" ON)
set(DENGINE_WIN CACHE STRING "Force Window API. Possible Values: X11, WAYLAND")
set(DENGINE_CONTEXT CACHE STRING "Force Context API. Possible Values: GLX, EGL")
option(DENGINE_DEBUG "Enable general debug facilities. Use only in Debug builds as it has alot of overhead" OFF)
option(DENGINE_DEBUG_GL "Enable OpenGL debug facilities. GL Debugging is decoupled from general debugging and can be enabled independently. Use only in Debug builds as it has alot of overhead" OFF)

find_library(DENGINE_HAS_LIBZ z)
find_library(DENGINE_HAS_EGL EGL)
find_package(PkgConfig QUIET)

if(PkgConfig_FOUND AND NOT ANDROID_NDK)
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

    if(DENGINE_SCRIPTING_PYTHON AND DENGINE_USE_SYSTEM_PYTHON)
        pkg_check_modules(python3 python3-embed)
        if(python3_LIBRARIES)
            message(">>> Found python3 libraries")
            include_directories(${python3_INCLUDE_DIRS})
            link_directories(${python3_LIBRARY_DIRS})
            set(DENGINE_HAS_PYTHON3 1)
            # we actively include Python.h in script.h
            string(APPEND DENGINE_PKG_DEPS "python3-embed ")
        else()
            #most if not all distros have python3
            message(WARNING ">>> Could not find python3. Python scripting will not work! Consider installing it")
        endif()
    endif()

    pkg_check_modules(bullet bullet)
    if(bullet_LIBRARIES)
        include_directories(${bullet_INCLUDE_DIRS})
        link_directories(${bullet_LIBRARY_DIRS})
        set(DENGINE_HAS_BULLET 1)
    endif()

    pkg_check_modules(x11 x11)
    if(x11_LIBRARIES)
        include_directories(${x11_INCLUDE_DIRS})
        link_directories(${x11_LIBRARY_DIRS})
        set(DENGINE_HAS_X11 1)
    endif()

    pkg_check_modules(glx glx)
    if(glx_LIBRARIES)
        include_directories(${glx_INCLUDE_DIRS})
        link_directories(${glx_LIBRARY_DIRS})
        set(DENGINE_HAS_GLX 1)
    endif()

    pkg_get_variable(wayland_scanner wayland-scanner wayland_scanner)
    pkg_get_variable(wayland_protocols_pkgdatadir wayland-protocols pkgdatadir)

    pkg_check_modules(wayland_client wayland-client)
    if(wayland_client_LIBRARIES)
        include_directories(${wayland_client_INCLUDE_DIRS})
        link_directories(${wayland_client_LIBRARY_DIRS})
        set(DENGINE_HAS_WAYLAND_CLIENT 1)
    endif()

    pkg_check_modules(wayland_egl wayland-egl)
    if(wayland_egl_LIBRARIES)
        include_directories(${wayland_egl_INCLUDE_DIRS})
        link_directories(${wayland_egl_LIBRARY_DIRS})
        set(DENGINE_HAS_WAYLAND_EGL 1)
    endif()

    pkg_check_modules(xkbcommon xkbcommon)
    if(xkbcommon_LIBRARIES)
        include_directories(${xkbcommon_INCLUDE_DIRS})
        link_directories(${xkbcommon_LIBRARY_DIRS})
        set(DENGINE_HAS_XKBCOMMON 1)
    endif()

    pkg_check_modules(libpulse_simple libpulse-simple)
    if(libpulse_simple_LIBRARIES)
        include_directories(${libpulse_simple_INCLUDE_DIRS})
        link_directories(${libpulse_simple_LIBRARY_DIRS})
        set(DENGINE_HAS_LIBPULSE_SIMPLE 1)
    endif()
else()
    message(WARNING ">>> Could not find pkg-config. Will compile From sources")

    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/deps/bullet3_src/CMakeLists.txt)
        ADD_DEFINITIONS( -DBT_THREADSAFE=1 )
        add_subdirectory(deps/bullet3_src "bullet_build")
        #TODO: get from builddir 
        set(bullet_LIBRARIES LinearMath BulletDynamics BulletCollision BulletSoftBody)
        set(bullet_INCLUDE_DIRS deps/bullet3_src/src)
        set(DENGINE_HAS_BULLET 1)
        include_directories(${bullet_INCLUDE_DIRS})
    else()
        message(WARNING "submodule deps/bullet_src not init. run `git submodule update --init --depth 1 deps/bullet3_src`")
    endif()
    if(DENGINE_SCRIPTING_PYTHON)
        set(DENGINE_USE_SYSTEM_PYTHON 0)
    endif()

    
endif()

if(DENGINE_SCRIPTING_PYTHON AND NOT DENGINE_USE_SYSTEM_PYTHON)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/deps/cpython-portable/CMakeLists.txt)
        set(PYSRC ${CMAKE_CURRENT_SOURCE_DIR}/deps/cpython)
        set(PYVER "3.9")
        set(DENGINE_HAS_PYTHON3 1)
        include_directories(
            ${CMAKE_CURRENT_SOURCE_DIR}/deps/cpython/Include
            ${CMAKE_CURRENT_BINARY_DIR}/cpython-portable-build/config
        )
        message(WARNING "A custom Python will be compiled from source. Ensure version ${PYVER} is compatible with your scripts and their dependencies")
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/deps/cpython-portable "cpython-portable-build")
        set(python3_LIBRARIES cpython-portable)
    else()
        message(FATAL_ERROR "cpython-portable not found but python scripting requested. either undefine DENGINE_SCRIPTING_PYTHON, enable DENGINE_USE_SYSTEM_PYTHON or run `git submodule update --init --depth 1 deps/cpython deps/cpython-portable`")
    endif()
endif()

# base includes to build 
set(dengine_incl
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/stb
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/cglm/include
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/glad/include
    ${PROJECT_BINARY_DIR}/dengine_config #dengine_config.h
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/dengine-core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/dengine-utils/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/dengine-gui/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/dengine-model/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/dengine-scene/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/dengine-script/include
    ${CMAKE_CURRENT_SOURCE_DIR}/glue/include
    )
#native app glue
if(ANDROID_NDK)
    list(APPEND dengine_incl ${ANDROID_NDK}/sources)
endif()
include_directories(${dengine_incl})

include(CheckSymbolExists)
include(CheckIncludeFile)

check_symbol_exists(clock_gettime "time.h" DENGINE_HAS_CLOCK_GETTIME)
check_include_file(pthread.h DENGINE_HAS_PTHREAD_H)
check_include_file(bits/pthreadtypes.h DENGINE_HAS_PTHREADTYPES_H)
check_include_file(bits/pthread_types.h DENGINE_HAS_PTHREAD_TYPES_H)
check_symbol_exists(pthread_cancel "pthread.h" DENGINE_HAS_PTHREAD_CANCEL)
set(CMAKE_REQUIRED_FLAGS "-D_GNU_SOURCE")
check_symbol_exists(pthread_setname_np "pthread.h" DENGINE_HAS_PTHREAD_SETNAME_NP)
set(CMAKE_REQUIRED_FLAGS "")

configure_file(dengine_config.h.in dengine_config/dengine_config.h)

if (NOT MSVC)
    add_compile_options(-Wall)
endif()

set(dengine 
    dengine-utils
    dengine-core
    dengine-gui
    dengine-script
    dengine-model
    dengine-scene

    #using ${dengine} is using glue implicitly
    dengine_glue_lib
    )

set(denginerc_loc "${dengine_SOURCE_DIR}/build/dengine-rc")
set(DengineRC_DIR ${denginerc_loc})
find_package(DengineRC)
if(NOT DengineRC_FOUND)
    message(FATAL_ERROR "Could not find DengineRC. Ensure its been built in ${denginerc_loc}")
endif()
include(cmake/DengineRCHelper.cmake)
include(cmake/DengineAddLibHelper.cmake)
if(WIN32)
    set(tests_install_dir lib/dengine-${DENGINE_VERSION})
else()
    set(tests_install_dir lib/dengine-${DENGINE_VERSION}/tests/)
endif()

if(MINGW AND CMAKE_CROSSCOMPILING)
    add_subdirectory(deps/ntldd "ntldd")
endif()
add_subdirectory(lib/dengine-utils "dengine-utils-build")
add_subdirectory(lib/dengine-core "dengine-core-build")
add_subdirectory(lib/dengine-gui "dengine-gui-build")
add_subdirectory(lib/dengine-model "dengine-model-build")
add_subdirectory(lib/dengine-script "dengine-script-build")
add_subdirectory(lib/dengine-scene "dengine-scene-build")
add_subdirectory(glue "glue-build")
add_subdirectory(nsl "nsl-build")

if(DENGINE_BUILD_TESTS)
    add_subdirectory(tests "tests-build")
endif()

#please ignore this
if(DENGINE_BUILD_TESTS_PRIV)
    add_subdirectory(testspriv "testspriv-build")
endif()

if(GTK3_LIBRARIES)
    add_subdirectory(main/dengitor "dengitor-build")
endif()

foreach(i ${dengine})
    string(APPEND DENGINE_PKG_LIBS "-l${i} ")
endforeach()

configure_file(dengine.pc.in dengine.pc @ONLY)

find_program(ZIP_EXECUTABLE zip)
if(NOT ZIP_EXECUTABLE)
    message(WARNING "zip not found. reading installed saved assets won't work!")
else()
    execute_process(
        COMMAND git ls-files assets
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE ASSET_FILES
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    string(REPLACE "\n" ";" ASSET_FILE_LIST "${ASSET_FILES}")
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/assets.zip
        DEPENDS ${ASSET_FILE_LIST} #does mean file will have to be committed to reflect in cmake. run cmake twice on new asset??
        COMMAND ${ZIP_EXECUTABLE} -r ${CMAKE_CURRENT_BINARY_DIR}/assets.zip . -x priv/\*
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/assets
        COMMENT "zipping assets into dengine-utils"
        VERBATIM
    )
    add_custom_target(assets_zip ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/assets.zip)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/assets.zip DESTINATION share/dengine-${DENGINE_VERSION})
endif()
install(FILES ${PROJECT_BINARY_DIR}/dengine_config/dengine_config.h DESTINATION include/dengine-${DENGINE_VERSION}/dengine_config)
install(DIRECTORY deps/glad/include/glad DESTINATION include/dengine-${DENGINE_VERSION}/deps/glad)
install(DIRECTORY deps/glad/include/KHR DESTINATION include/dengine-${DENGINE_VERSION}/deps/glad)
install(DIRECTORY deps/cglm/include/cglm DESTINATION include/dengine-${DENGINE_VERSION}/deps/cglm)
if(PYSRC)
    install(TARGETS cpython-portable DESTINATION lib/dengine-${DENGINE_VERSION})
endif()
# pkgconf
install(FILES "${PROJECT_BINARY_DIR}/dengine.pc" DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
