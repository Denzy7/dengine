add_library(inpt SHARED inptmodule.c inptmodule.h)
set_target_properties(inpt PROPERTIES PREFIX "")

add_library(timer SHARED timermodule.c timermodule.h)
set_target_properties(timer PROPERTIES PREFIX "")
