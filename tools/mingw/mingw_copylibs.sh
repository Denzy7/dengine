#!/bin/bash
prefix="$1"
ver="$2"
exe="$3"
projbindir="$4"
xcompile="$5"
dir="$(echo "$exe" | sed 's:[/\\][^/\\]*$::')"
ldd=$(command -v ldd)
grepstr="not found|system32|winsxs|linux-vdso|ld-linux"
sedstr="s/(0x[0-9a-fA-F]*)//g"
sedstr2="s/.*=> //g"


echo "info: p:$prefix v:$ver e:$exe d:$dir x:$xcompile ldd:$ldd"

if [ -n "$ldd" ] && [ "$xcompile" != "TRUE" ]; then
    echo "using system ldd"
    "$ldd" "$exe" |  grep -Evi "$grepstr" | sed "$sedstr" | sed "$sedstr2" | while read -r file; do
    if [[ -f "$file" ]]; then
        cp "$file" "$dir"
        echo "Copied $file"
    fi
done
    exit 0
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
"$python" mingw-ldd/mingw-ldd.py "$exe"

"$python" mingw-ldd/mingw-ldd.py "$exe" | grep -Evi "$grepstr" | sed "$sedstr" | sed "$sedstr2" | while read -r file; do
    if [[ -f "$file" ]]; then
        cp "$file" "$dir"
        echo "Copied $file"
    fi
done



# special case ntldd depends on libssp
if [[ -f "$projbindir/ntldd/ntldd.exe" ]];then
    "$python" mingw-ldd/mingw-ldd.py "$projbindir/ntldd/ntldd.exe" | grep -Evi "$grepstr" | sed "$sedstr" | sed "$sedstr2" | while read -r file; do
    if [[ -f "$file" ]]; then
        cp "$file" "$projbindir/ntldd"
    fi
done
    echo "using ntldd"
    "$projbindir/ntldd/ntldd.exe" "$exe" | grep -Evi "$grepstr" | sed "$sedstr" | sed "$sedstr2" | while read -r file; do
    if [[ -f "$file" ]]; then
        cp "$file" "$dir"
        echo "Copied $file"
    fi
done

    "$projbindir/ntldd/ntldd.exe" "$exe"
fi

