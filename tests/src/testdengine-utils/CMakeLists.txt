#confserialize
add_executable(testdengine-util-confserialize
        util-confserialize.c
    )
target_link_libraries(testdengine-util-confserialize
        dengine-utils)
set_property(TARGET testdengine-util-confserialize PROPERTY C_STANDARD 99)


#filesystem
add_executable(testdengine-util-filesys
        util-filesys.c
    )
target_link_libraries(testdengine-util-filesys
        dengine-utils)
set_property(TARGET testdengine-util-filesys PROPERTY C_STANDARD 99)


#os specifics
add_executable(testdengine-util-os
        util-os.c
    )
target_link_libraries(testdengine-util-os
        dengine-utils)
set_property(TARGET testdengine-util-os PROPERTY C_STANDARD 99)

#rng util
add_executable(testdengine-util-rng
        util-rng.c
    )
target_link_libraries(testdengine-util-rng
        dengine-utils)
set_property(TARGET testdengine-util-rng PROPERTY C_STANDARD 99)

#timer util
add_executable(testdengine-util-timer
        util-timer.c
    )
target_link_libraries(testdengine-util-timer
        dengine-utils)
if(UNIX)
    target_link_libraries(testdengine-util-timer m)
endif()
set_property(TARGET testdengine-util-timer PROPERTY C_STANDARD 99)

#str util
add_executable(testdengine-util-str
        util-str.c
    )
target_link_libraries(testdengine-util-str
        dengine-utils)
set_property(TARGET testdengine-util-str PROPERTY C_STANDARD 99)

#vtor util
add_executable(testdengine-util-vtor
        util-vtor.c
    )
target_link_libraries(testdengine-util-vtor
        dengine-utils)
set_property(TARGET testdengine-util-vtor PROPERTY C_STANDARD 99)

#read zip files
add_executable(testdengine-util-zipread util-zipread.c)
target_link_libraries(testdengine-util-zipread dengine-utils)
set_property(TARGET testdengine-util-zipread PROPERTY C_STANDARD 99)

#threading util
add_executable(testdengine-util-thread util-thread.c)
target_link_libraries(testdengine-util-thread dengine-utils)
if(UNIX)
    target_link_libraries(testdengine-util-thread dengine-utils m )
endif()
set_property(TARGET testdengine-util-thread PROPERTY C_STANDARD 99)
