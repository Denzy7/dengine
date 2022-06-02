cmake_minimum_required(VERSION 3.10)
project(dengine-rc VERSION 0.0.0)

if(CMAKE_CROSSCOMPILING)
    find_package(DengineRC)
else()
    add_executable(dengine-rc main.c)
    export(TARGETS dengine-rc FILE
          "${CMAKE_BINARY_DIR}/DengineRCConfig.cmake")
endif()

## rc example

## resource list
#set(myresources
##    myres/myfile1.zip
##    myres/myfile2.png
##    myres/myfile3.txt
##    ...
#)

## output after compile
#set(outres)

## check for cross compile
#if(CMAKE_CROSSCOMPILING)
#    find_package(DengineRC)
#endif()

## compile
#foreach(res ${myresources})
## inres and outres should be absolute paths for fair measure
#    set(inresname ${CMAKE_CURRENT_SOURCE_DIR}/${res})
#    get_filename_component(barename ${res} NAME)
#    set(outresname ${CMAKE_CURRENT_BINARY_DIR}/${barename}-res.c)
#    add_custom_command(
#        OUTPUT ${outresname}
#        COMMAND dengine-rc ${inresname} ${outresname}
#        DEPENDS ${inresname}
#        VERBATIM
#    )

## append
#    list(APPEND outres ${outresname})
#endforeach()

## add to a target(library, exe, ...)
#add_library(target ${outres})

#then extern in code
# extern unsigned char myfile1_zip[];
# extern unsigned int myfile1_zip_len;
