# windows
**Recommended you use MSYS2. This method involves manually setting up MinGW.**  

Ensure you have an existing installation of MinGW. If not, go ahead and download its [installer](http://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win32/Personal%20Builds/mingw-builds/installer/mingw-w64-install.exe/download) for Windows.  
**If you want more recent versions of MinGW** see below

Once done, locate the install location. Proceed to add `<install-folder/bin` to `PATH`. There are many ways to do this like control panel and `set` in cmd.

The installer may put MinGW in another sub folder `mingw32`

If you want more recent versions of MinGW please visit [WinLibs](https://winlibs.com) and download an archive. Extract it to some folder and locate `bin` folder and add its full path name to `PATH`. This is probably the best MinGW for Windows as it has the latest and greatest in GCC and has a bundled python

# linux
See [MinGW Downloads](https://www.mingw-w64.org/downloads/) for Linux dev packages
In a nutshell, install:  
**Ubuntu/Debian** : `mingw-w64`  
**Arch** : `mingw-w64-gcc` 

The following helpers can be used:  
- **Arch User Repository** : `mingw-w64-cmake mingw-w64-environment mingw-w64-pkg-config`  
  Simply replace any command with `cmake` with `<arch>-w64-mingw32-cmake` 
  e.g.  
  32-bit cmake : `i686-w64-mingw32-cmake -S . -B build`  
  64-bit cmake : `x86_64-w64-mingw32-cmake -S . -B build`  


# msys2
MSYS2 has mingw already up-to-date and built in and is simply a linux enviromnent. The scripts to copy libs should also work fine with msys2

# cmake

With that, run:  
`cmake -S . -B build/desktop`

(MinGW users pass) `-G "MinGW Makefiles"`

If you are getting an error `CMAKE_MAKE_PROGRAM not set`, did you add the `bin` folder to `PATH`?

Proceed with build  

If you use the provided toolchain file (REALLY NOT RECOMMENDED UNLESS USING REALLY OLD CMAKE OR BUILDING IN LINUX WIHOUT MINGW-HELPERS!):
- Take note of the architechture you selected when installing. It's usually x86_64 (64 bit) or i686 (32 bit)
- Locate the installation folder. It typically has directories like `bin`, `include` and `lib`. On Linux, its usually located in `/usr/<arch>-w64-mingw` or `/usr/local/<arch>-w64-mingw`
- pass the following to cmake : `-DCMAKE_TOOLCHAIN_FILE=tools/mingw/mingw.toolchain.cmake -DMINGW_DIR=<location-of-mingw> -DMINGW_ARCH=<architecture>`






