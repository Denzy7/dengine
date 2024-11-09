# stubs
provide static anaylsis tools (SAT's) like autocomplete tools to provide compile time type inference  

there have been several attempts to make multiple ABI's to work with out extensions but have resulted in failure like building a custom intepretor. when we say multiple ABI's we mean when dengine is build with MinGW but python running the SAT's is on MSVC, thus we cannot LoadLibrary a MinGW DLL from MSVC (i.e a Python extension like dengine.inpt.pyd). As you can see this is primarily a win32 problem since dengine code is not written with MSVC in mind.

However [stubs](https://peps.python.org/pep-0484/#stub-files) help by allowing us to manually declare available methods as well as types, kind of how we do with C header files, by copying them from their respective `PyMethodDef`'s and `PyTypeObject`'s. Being handwritten therefore is prone to error and may not exactly represent the methods and types available in the extension module, and may not provide relevant documentation

you can however overcome this whole stub situation by avoiding multiple ABI's by building dengine with the same compiler toolchain that built the python running the SAT, which is usually MSVC. If you are on Windows, you are pretty much stuck with stubs :(

Another advantage of stubs is they are python scripts, so no loading of native libraries with may not be in LD_LIBRARY_PATH or whatever Win32 uses for DLL path resolution

# usage
simply point your IDE or LSP server to the directory with the `dengine` folder. Ensure there are some `.pyi` files inside
