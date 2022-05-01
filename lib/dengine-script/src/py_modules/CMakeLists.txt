add_library(inpt SHARED inptmodule.c)
set_target_properties(inpt PROPERTIES PREFIX "")
target_link_libraries(inpt dengine-core)

add_library(timer SHARED timermodule.c)
set_target_properties(timer PROPERTIES PREFIX "")
target_link_libraries(timer dengine-utils)

add_library(logging SHARED loggingmodule.c)
set_target_properties(logging PROPERTIES PREFIX "")
target_link_libraries(logging dengine-utils)

add_library(filesys SHARED filesysmodule.c)
set_target_properties(filesys PROPERTIES PREFIX "")
target_link_libraries(filesys dengine-utils)

add_library(common SHARED commonmodule.c commonmodule.h)
set_target_properties(common PROPERTIES PREFIX "")
target_link_libraries(common dengine-core dengine-utils dengine-scene)

add_library(scene SHARED scenemodule.h scenemodule.c)
set_target_properties(scene PROPERTIES PREFIX "")
target_link_libraries(scene dengine-utils dengine-scene common)



