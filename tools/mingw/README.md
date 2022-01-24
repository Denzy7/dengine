# windows
Ensure you have an existing installation of MinGW. If not, go ahead and download its [installer](http://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win32/Personal%20Builds/mingw-builds/installer/mingw-w64-install.exe/download) for Windows.

Once done, locate the install location. Proceed to add `<install-folder/bin` to `PATH`. There are many ways to do this like control panel and the like.

The installer may put MinGW in another sub folder `mingw32`

# linux
See [MinGW Downloads](https://www.mingw-w64.org/downloads/) for Linux dev packages
In a nutshell, install:
**Ubuntu/Debian** : `mingw-w64`
**Arch** : `mingw-w64-gcc` 

# cmake
**Take note of the architechture you selected when installing. It's usually x86_64 (64 bit) or i686 (32 bit)**

Locate the installation folder. It typically has directories like `bin`, `include` and `lib`.

On Linux, its usually located in `/usr/<arch>-w64-mingw` or `/usr/local/<arch>-w64-mingw`

With that, run:
`cmake -S . -B build/desktop -DCMAKE_TOOLCHAIN_FILE=tools/mingw/mingw.toolchain.cmake -DMINGW_DIR=<location-of-mingw> -DMINGW_ARCH=<architecture>`

(Windows users pass) `-G "MinGW Makefiles"`

If you are getting an error `CMAKE_MAKE_PROGRAM not set`, did you add the `bin` folder to `PATH`?

Proceed with build


