# custom interpretor for Windows using MinGW
Shoutout to [the smart folk over at the AUR](https://aur.archlinux.org/packages/mingw-w64-python) for building CPython with MinGW 🍻

- Obtain the latest releases of `python-dengine-script-*-.zip and python-modules-*-.zip`. 
- Open cmd.exe and type `%APPDATA%` then create a folder `dengine` here if it doesn't exist
- Extract the `python-dengine-script` here into a folder `python-dengine-script-<VER>-<PYVER>-<ARCH>`. `VER` is just a number for the release. `PYVER` is the Python version used. `ARCH` is the architecture. `i686=32-bit`, `x86_64=64-bit`

ITS IMPORTANT YOU MAINTAIN THE FOLDER NAMING. ENSURE INSIDE `python-dengine-script-<VER>-<PYVER>-<ARCH>` IS A FOLDER `python-stdlib` and `python-dengine-script-mingw32.exe` as well as some other DLL's

- Extract `python-modules.zip` to `python-dengine-script-<VER>-<PYVER>-<ARCH>/python-modules`
- You can test if `python-dengine-script-mingw32.exe` works and can import something like `import dengine.filesys as fs`
- Now inside VSCode add a workspace interpretor to `python-dengine-script-mingw32.exe`

Hopefully IntelliSense should now work!