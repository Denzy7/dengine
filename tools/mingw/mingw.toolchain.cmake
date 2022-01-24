# original: https://gist.github.com/take-cheeze/2850831#file-toolchain-mingw-cmake
cmake_minimum_required(VERSION 3.6.0)

set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -O2 -pipe -fno-plt -fexceptions --param=ssp-buffer-size=4 -Wformat -Werror=format-security -fcf-protection")
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS}")

set(BUILD_SHARED_LIBS ON)

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
	
set(RC_COMPILERS
	/usr/bin/${MINGW_ARCH}-w64-mingw32-windres
	/usr/local/bin/${MINGW_ARCH}-w64-mingw32-windres
	${MINGW_DIR}/bin/${MINGW_ARCH}-w64-mingw32-windres.exe
)
set(CMAKE_MAKE_PROGRAM ${MINGW_DIR}/bin/mingw32-make)
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

foreach(RC_COMPILER IN LISTS RC_COMPILERS)
	if(EXISTS ${RC_COMPILER})
		message(">>> Found RC Compiler : " ${RC_COMPILER})
		set(CMAKE_RC_COMPILER ${RC_COMPILER})
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
