Ensure you have an existing installation of MinGW. If not, go ahead and download its [installer](http://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win32/Personal%20Builds/mingw-builds/installer/mingw-w64-install.exe/download) for Windows.

See [MinGW Downloads](https://www.mingw-w64.org/downloads/) for Linux dev packages

**Take note of the architechture you select when installing. It's usually x86_64 or i686**

Locate the installation folder. It typically has directories like `bin`, `include` and `lib`.

On Linux, its usually located in `/usr/<arch>-w64-mingw` or `/usr/local/<arch>-w64-mingw`

With that, run:
`cmake -S . -B build/desktop -DCMAKE_TOOLCHAIN_FILE=tools/mingw/mingw.toolchain.cmake -DMINGW_DIR=<location-of-mingw> -DMINGW_ARCH=<architecture>`

If you get a warning, run it again to clear it as cmake runs twice on a toolchain file

Proceed with build


