# dengine
Simple 3D Graphics Engine

# cloning
Clone the repo. If on Windows to get all needed dependencies you might want to pass `--recursive`:  
`git clone --recursive https://github.com/Denzy7/dengine`  

If this is too much for you or takes an insanely long time, follow Linux users to the next step  

On Linux and its derivatives, refer to [deps/README.md](deps/README.md) to install all needed dependencies:

# building
- **DengineRC**: Build `dengine-rc` for build/host system if cross compiling:  
	`cmake -S main/dengine-rc -B build/dengine-rc`  
	`cmake --build build/dengine-rc`
	
	Remember to add `-DDengineRC_DIR=<absolute_path_to_DengineRCConfig.cmake>` to cmake configuration
		 
- **Desktop** (Linux, Windows) : Build with CMake. For Windows, MinGW is highly recommended. *Haven't tested on MSVC with C99* 

	- Configure cmake:
	`cmake -S . -B build/desktop -DDengineRC_DIR=<absolute_path_to_DengineRCConfig.cmake>`

	**MinGW Note:**
	See [tools/mingw/README.md](tools/mingw/README.md)

	- Build the tree:
        `cmake --build build/desktop`

- **Android** : Clone the repo as a Linux user.

	- Run `./gradlew` (Linux) or `gradlew.bat` (Windows). Install a JDK if don't have one

	- By default, this builds an APK which you can run on a Physical Device or Emulator.

	- To build the tests, run regular cmake. Use the toolchain in the NDK
	`cmake -S . -B build/android -DCMAKE_TOOLCHAIN_FILE='<NDK_DIR>/build/cmake/android.toolchain.cmake' -DANDROID_API=24 -DDengineRC_DIR=<absolute_path_to_DengineRCConfig.cmake>`

	See other options at [Android docs](https://developer.android.com/studio/projects/configure-cmake#call-cmake-cli)

	- Run the tests with a terminal emulator like [Termux](https://f-droid.org/en/packages/com.termux/)
	

# writing python scripts
- **Linux**: Its actually very easy here. Just install the modules located in `<CMAKE_BUILD_DIR>/dengine-script-build/modules-build/to-python3-site-pacakages`. Install `dengine` directory to your Python `site-packages` or add the directory to your IDE/Editor Python path (VSCode-OSS in this case) 
- **Windows**: Its very tricky here as it depends on the Python version installed and what it was compiled with. Upstream Python is build with MSVC. Note the precompiled releases use MinGW which is obviously **NOT** compatible with MSVC. So a little hack is to [build a custom interpretor with MinGW](main/python-dengine-script/README.md) and add it to VSCode. Alternatively, compile everything with MSVC, which I haven't tested. Then copy to `site-packages`(I don't have Windows nor MSVC. I'm also not a big MS fan 🤷)
