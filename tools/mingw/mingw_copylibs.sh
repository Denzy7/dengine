#!/bin/bash
prefix="$1"
ver="$2"
exe="$3"
projbindir="$4"
xcompile="$5"
syspfx="$6"

dir="$(echo "$exe" | sed 's:[/\\][^/\\]*$::')"
ldd=$(command -v ldd)
grepstr="not found|system32|winsxs|linux-vdso|ld-linux"
sedstr="s/(0x[0-9a-fA-F]*)//g"
sedstr2="s/.*=> //g"

echo "info: p:$prefix v:$ver e:$exe d:$dir x:$xcompile ldd:$ldd syspfx:$syspfx"

if [ -n "$ldd" ] && [ "$xcompile" != "TRUE" ]; then
    echo "using system ldd"
    # msys2 ldd refuses to cooperate if not ending with dll
    exe_nsl="$exe"
    isnsl=0
    if [[ $exe == *.nsl ]]; then
        exe_nsl="${exe}.dll"
        isnsl=1
        cp -f "${exe}" "${exe_nsl}"
    fi
    "$ldd" "$exe_nsl" |  grep -Evi "$grepstr" | sed "$sedstr" | sed "$sedstr2" | while read -r file; do
    if [[ -f "$file" ]]; then
        cp -f "$file" "$dir"
        echo "Copied $file"
    fi
    # we dont wan't dupes in  outpput
    if [ "$isnsl" -eq 1 ]; then
        rm "$exe_nsl"
    fi
done
#exit 0
fi

python3 -m venv venv

if [[ -f ".\\venv\\Scripts\\pip" ]]; then
    pip=".\\venv\\Scripts\\pip"
    python=".\\venv\\Scripts\\python"
elif [[ -f "./venv/bin/pip" ]]; then
    pip="./venv/bin/pip"
    python="./venv/bin/python"
else
    echo "pip and python not found in either path."
    exit 1
fi

"$pip" install pefile
echo "using mingw-ldd"

lddstr=$("$python" mingw-ldd/mingw_ldd/mingw_ldd.py "$exe" --dll-lookup-dirs "$syspfx/bin" "$prefix/lib")
echo "${lddstr}"
echo "${lddstr}" | grep -Evi "$grepstr" | sed "$sedstr" | sed "$sedstr2" | while read -r file; do
if [[ -f "$file" ]]; then
    cp -f "$file" "$dir"
    echo "Copied $file"
fi
done

# special case ntldd depends on libssp
if [[ -f "$projbindir/ntldd/ntldd.exe" ]];then
    "$python" mingw-ldd/mingw_ldd/mingw_ldd.py  "$projbindir/ntldd/ntldd.exe" --dll-lookup-dirs "$syspfx/bin" | grep -Evi "$grepstr" | sed "$sedstr" | sed "$sedstr2" | while read -r file; do
    if [[ -f "$file" ]]; then
        cp -f "$file" "$projbindir/ntldd"
    fi
done
echo "using ntldd"
"$projbindir/ntldd/ntldd.exe" "$exe" | grep -Evi "$grepstr" | sed "$sedstr" | sed "$sedstr2" | while read -r file; do
if [[ -f "$file" ]]; then
    cp -r "$file" "$dir"
    echo "Copied $file"
fi
done

"$projbindir/ntldd/ntldd.exe" "$exe"
fi

