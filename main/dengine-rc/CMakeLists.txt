add_executable(dengine-rc main.c)

# resource list
#set(myresources
#    myres/myfile1
#    myres/myfile2
#    myres/myfile3
#    myres/myfile4
#)

# output after compile
#set(outres)

# compile
#foreach(res ${myresources})
#    set(inresname ${CMAKE_CURRENT_SOURCE_DIR}/${res})
#    set(outresname ${CMAKE_CURRENT_BINARY_DIR}/${res}-res.c)
#    add_custom_command(
#        OUTPUT ${outresname}
#        COMMAND dengine-rc ${inresname} ${outresname}
#        DEPENDS ${inresname}
#        VERBATIM
#    )

# append
#    list(APPEND outres ${outresname})
#endforeach()

# add to a target(library, exe, ...)
#add_library(rescompiled ${outres})
