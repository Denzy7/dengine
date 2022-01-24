# original: https://gist.github.com/take-cheeze/2850831#file-toolchain-mingw-cmake
cmake_minimum_required(VERSION 3.6.0)

# CMake invokes the toolchain file twice during the first build, but only once
# during subsequent rebuilds. 
if(MINGW_TOOLCHAIN_INCLUDED)
  return()
endif(MINGW_TOOLCHAIN_INCLUDED)
set(MINGW_TOOLCHAIN_INCLUDED true)

if(DEFINED MINGW_DIR AND MINGW_TOOLCHAIN_INCLUDED)
	message(STATUS "MinGW dir : ${MINGW_DIR}")
else()
	message(WARNING "Please specify an existing installation of MinGW (-DMINGW_DIR)")
endif()

if(DEFINED MINGW_ARCH AND MINGW_TOOLCHAIN_INCLUDED)
	message(STATUS "MinGW arch : ${MINGW_ARCH}")
else()
	message(WARNING "Please specify an Architecture of the MinGW (-DMINGW_ARCH) . (i686, x86_64...)")
endif()

# the name of the target operating system
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR ${MINGW_ARCH})

set(CMAKE_SYSROOT ${MINGW_DIR})

set(C_COMPILERS
	/usr/bin/${MINGW_ARCH}-w64-mingw32-gcc
	/usr/local/bin/${MINGW_ARCH}-w64-mingw32-gcc
	${MINGW_DIR}/bin/${MINGW_ARCH}-w64-mingw32-gcc.exe
	)

set(CXX_COMPILERS
	/usr/bin/${MINGW_ARCH}-w64-mingw32-g++
	/usr/local/bin/${MINGW_ARCH}-w64-mingw32-g++
	${MINGW_DIR}/bin/${MINGW_ARCH}-w64-mingw32-g++.exe
	)

foreach(C_COMPILER IN LISTS C_COMPILERS)
	if(EXISTS ${C_COMPILER})
		message(">>> Found C Compiler : " ${C_COMPILER})
		set(CMAKE_C_COMPILER ${C_COMPILER})
		break()
	endif()
endforeach()

foreach(CXX_COMPILER IN LISTS CXX_COMPILERS)
	if(EXISTS ${CXX_COMPILER})
		message(">>> Found CXX Compiler : " ${CXX_COMPILER})
		set(CMAKE_CXX_COMPILER ${CXX_COMPILER})
		break()
	endif()
endforeach()

# here is the target environment located
set(CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT} ${MINGW_DIR}/${MINGW_ARCH}-w64-mingw32)

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_FIND_LIBRARY_PREFIXES "lib" "")
set(CMAKE_FIND_LIBRARY_SUFFIXES ".dll" ".dll.a" ".lib" ".a")
