#utils
add_subdirectory(src/testdengine-utils "utils-build")

#executables for desktop
if(NOT ANDROID)    
add_subdirectory(src/testdengine-camera "camera-build")
add_subdirectory(src/testdengine-cube "cube-build")
add_subdirectory(src/testdengine-dengine "dengine-build")
add_subdirectory(src/testdengine-input "input-build")
add_subdirectory(src/testdengine-lighting "lighting-build")
add_subdirectory(src/testdengine-model "model-build")
add_subdirectory(src/testdengine-plane "plane-build")
add_subdirectory(src/testdengine-quad "quad-build")
add_subdirectory(src/testdengine-scene "scene-build")
add_subdirectory(src/testdengine-tooling "tooling-build")
add_subdirectory(src/testdengine-triangle "triangle-build")
add_subdirectory(src/testdengine-window "window-build")
endif()
