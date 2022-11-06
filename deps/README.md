# have you cloned me before?
Its a good idea to check if you have cloned the following deps before to save some disk space.
- [stb](https://github.com/nothings/stb.git)
- [cglm](https://github.com/recp/cglm.git)
- [cpython v3.9](https://github.com/python/cpython)

If so, make symlinks to this directory.
On Linux : `ln -sf <dir> deps/`
On Windows : `mklink <dir> deps\`

**You might want to use a full path instead of a relative path with symlinks**

Proceed to build

# default deps
Initialize submodules
`git submodule init`

Get default deps
`git submodule update deps/cglm deps/stb`

# optional deps
**If you will be compiling for Windows or Android you might want to get all optional deps to avoid missing libraries and header files**  
**If you want Python scripting, update deps/cpython (will take some time!)**  

`git submodule update deps/cpython`

# python
Depends if python is found on your system. And is compatible with the build system. If not this will result in a lenghty compile process. It might also result in an incompatible module set.

A general rule of thumb if you are using Linux, no need to build python. Just set CMake option DENGINE_USE_SYSTEM_PYTHON to ON. Otherwise if you are going to build on another system, definitely set it to OFF

# debian & ubuntu
`# apt install libgtk-3-dev libpython3.9-dev `
**If on Wayland ensure you have the following additional dependencies. Alternatively have XWayland installed and running**
`# apt install libwayland-dev libxkbcommon-dev wayland-protocols`
# arch
`# pacman -S gtk3 python`
**If on Wayland ensure you have the following additional dependencies. Alternatively have XWayland installed and running**
`# pacman -S wayland libxkbcommon wayland-protocols`

**Please note wayland support is experimental and may be unstable or not even build! Ensure you are on the the latest version of wayland or a rolling distro like arch**

# dengitor and gtk3
Building GTK+ 3 is really complicated. However [some really smart people in the AUR](https://aur.archlinux.org/packages/mingw-w64-gtk3) have already figured it out.

On Windows, you can use the `MSYS2` commandline to install GTK and build `dengitor` (I haven't actually tested this but [it seems to be in the package list](https://packages.msys2.org/base/mingw-w64-gtk3))

# others
Your distro probably has the packages you need. If not, get all optional deps. This might results in a slightly longer and larger build time
