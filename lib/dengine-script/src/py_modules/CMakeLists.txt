add_library(inpt SHARED inptmodule.c)
set_target_properties(inpt PROPERTIES PREFIX "")
target_link_libraries(inpt dengine-core ${python3_LIBRARIES})

add_library(timer SHARED timermodule.c)
set_target_properties(timer PROPERTIES PREFIX "")
target_link_libraries(timer dengine-utils ${python3_LIBRARIES})

add_library(logging SHARED loggingmodule.c)
set_target_properties(logging PROPERTIES PREFIX "")
target_link_libraries(logging dengine-utils ${python3_LIBRARIES})

add_library(filesys SHARED filesysmodule.c)
set_target_properties(filesys PROPERTIES PREFIX "")
target_link_libraries(filesys dengine-utils ${python3_LIBRARIES})

add_library(common SHARED commonmodule.c)
set_target_properties(common PROPERTIES PREFIX "")
target_link_libraries(common dengine-core dengine-utils ${python3_LIBRARIES})

add_library(scene SHARED scenemodule.c)
set_target_properties(scene PROPERTIES PREFIX "")
target_link_libraries(scene dengine-utils dengine-scene common ${python3_LIBRARIES})

if(WIN32)
    set_target_properties(inpt PROPERTIES SUFFIX ".pyd")
    set_target_properties(timer PROPERTIES SUFFIX ".pyd")
    set_target_properties(logging PROPERTIES SUFFIX ".pyd")
    set_target_properties(filesys PROPERTIES SUFFIX ".pyd")
    set_target_properties(common PROPERTIES SUFFIX ".pyd")
    set_target_properties(scene PROPERTIES SUFFIX ".pyd")
endif()




