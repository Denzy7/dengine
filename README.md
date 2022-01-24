# dengine
Lightweight Renderer and Engine

# building
- **Desktop** (Linux, Windows) : Build with CMake. For Windows, MinGW is highly recommended. *Haven't tested on MSVC*

	- Clone the repo:
		
		If on Windows to get all needed dependencies you might want to pass `--recursive`:
		`git clone --recursive https://github.com/Denzy7/dengine`
	
		 If this is too much for you or takes an insanely long time, follow Linux users to the next step

		 On Linux and its derivatives, refer to [deps/README.md](deps/README.md) to install all needed dependencies:

	- Configure cmake:
	`cmake -S . -B build/desktop`

	**MinGW Note:**
	See [tools/mingw/README.md](tools/mingw/README.md)

	- Build the tree:
`cmake --build build/desktop`

- **Android** : Clone the repo as a Linux user.

Run `./gradlew` (Linux) or `gradlew.bat` (Windows). Install a JDK if don't have one

By default, this builds an APK which you can run on a Physical Device or Emulator.

To build the tests, run regular cmake. Use the toolchain in the NDK
`cmake -S . -B build/android -DCMAKE_TOOLCHAIN_FILE='<NDK_DIR>/build/cmake/android.toolchain.cmake' -DANDROID_API=24`

See other options at [Android docs](https://developer.android.com/studio/projects/configure-cmake#call-cmake-cli)

Run the tests with a terminal emulator like [Termux](https://f-droid.org/en/packages/com.termux/)

