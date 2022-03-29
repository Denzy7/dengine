add_library(inpt SHARED inptmodule.c)
set_target_properties(inpt PROPERTIES PREFIX "")

add_library(timer SHARED timermodule.c)
set_target_properties(timer PROPERTIES PREFIX "")
