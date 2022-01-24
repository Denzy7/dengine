# have you cloned me before?
Its a good idea to check if you have cloned the following deps before to save some disk space.
- [glfw](https://github.com/glfw/glfw.git)
- [stb](https://github.com/nothings/stb.git)
- [cglm](https://github.com/recp/cglm.git)

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
**Windows users need to get all optional deps then proceed to build**
`git submodule update deps/glfw`

# debian & ubuntu
`# apt install libglfw3-dev libgtk-3-dev `
**Substitute libglfw3 for libglfw3-wayland if on wayland**

# arch
`# pacman -S glfw-x11 gtk3`
**Substitute glfw-x11 for glfw3-wayland if on wayland**

# others
Your distro probably has the packages you need. If not, get all optional deps. This might results in a slightly longer and larger build time
