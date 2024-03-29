# dengine
Simple 3D Graphics Engine

# cloning
Clone the repo. If on Windows to get all needed dependencies you might want to pass `--recursive`:  
`git clone --recursive https://github.com/Denzy7/dengine`  

If this is too much for you or takes an insanely long time, follow Linux users to the next step  

On Linux and its derivatives, refer to [deps/README.md](deps/README.md) to install all needed dependencies:

Change dir to repo:  
`cd dengine`

# building
- **DengineRC**: Build `dengine-rc` into source dir:  
	`cmake -S main/dengine-rc -B build/dengine-rc`  
        `cmake --build build/dengine-rc`
		 
- **Desktop** (Linux, Windows) : Build with CMake. For Windows, MinGW is highly recommended. *Haven't tested on MSVC with C99* 

	- (optional) Mess around with dengine_config.h.in for certain compile time options
	
	- Configure cmake:
        `cmake -S . -B build/desktop`
        
        - Optional options include:
            - `-DDENGINE_WIN`: Force Window API (Linux), can be X11, WAYLAND
            - `-DDENGINE_CONTEXT`: Force Context API (Linux), can be GLX(X11 only), EGL

	**MinGW Note:**
	See [tools/mingw/README.md](tools/mingw/README.md)

	- Build the tree:
        `cmake --build build/desktop`

- **Android** :
	- With Android Studio:
		- Simply open the apk test build.gradle with Android Studio
	- Without Android studio (terminal):  
		- Ensure [OpenJDK 11 is installed](https://openjdk.java.net/install/). Locate JRE from it. Call this `JAVA_HOME`. Ensure `JAVA_HOME/bin` has java executable.
		- Set environment variable to point to JAVA_HOME. Test with `java --version` if its actually openjdk 11.*
		- Run `./gradlew build` (Linux) or `gradlew.bat build` (Windows)
		- The following gradle options may speed up build by skipping certain unneeded tasks for Native Activity: `-x lintVitalAnalyzeRelease -x lintAnalyzeDebug -x lintDebug`	
		- By default, this builds an APK which you can run on any Physical Device or Emulator in app/build/output
                
        - Termux X11:
                - Configure cmake with option -DDENGINE_TERMUX_X11. Builds a regular executable as Desktop

	- To build the executable tests, run regular cmake. Use the toolchain in the NDK  
        `cmake -S . -B build/android -DCMAKE_TOOLCHAIN_FILE='<NDK_DIR>/build/cmake/android.toolchain.cmake' -DANDROID_API=24`

	See other options at [Android docs](https://developer.android.com/studio/projects/configure-cmake#call-cmake-cli)

	- Run the tests with a terminal emulator like [Termux](https://f-droid.org/en/packages/com.termux/). Termux has a very nice guide to [run executables on shared storage](https://wiki.termux.com/wiki/Termux-setup-storage)
	
# glue code
- See [glue/README.md](glue/README.md) on how to use it to simplify dengine code

# writing python scripts
- **Linux**: Its actually very easy here. Just install the modules located in `<CMAKE_BUILD_DIR>/dengine-script-build/modules-build/to-python3-site-pacakages`. Install `dengine` directory to your Python `site-packages` or add the directory to your IDE/Editor Python path (VSCode-OSS in this case) 
- **Windows**: Its very tricky here as it depends on the Python version installed and what it was compiled with. Upstream Python is build with MSVC. Note the precompiled releases use MinGW which is obviously **NOT** compatible with MSVC. So a little hack is to [build a custom interpretor with MinGW](main/python-dengine-script/README.md) and add it to VSCode. Alternatively, compile everything with MSVC, which I haven't tested. Then copy to `site-packages`(I don't have Windows nor MSVC. I'm also not a big MS fan 🤷)

# github actions releases
now availabe on github actions! you need to be logged in  
[check out the cmake workflow](https://github.com/Denzy7/dengine/actions/workflows/cmake-multi-platform.yml)
