import os
import sys
import shutil

install_prefix = sys.argv[1]
lib_install_path = os.path.join(install_prefix, "lib")
os.makedirs(lib_install_path, exist_ok=True)

for path in sys.path:
    if path and os.path.isdir(path) and "abc.py" in os.listdir(path):
        dest_path = os.path.join(lib_install_path, os.path.basename(path))
        shutil.copytree(path, dest_path, dirs_exist_ok=True)

def remove_unwanted_dirs(base_dir):
    for root, dirs, _ in os.walk(base_dir, topdown=False):
        for dir_name in dirs:
            if dir_name in ("__pycache__", "site-packages"):
                dir_path = os.path.join(root, dir_name)
                shutil.rmtree(dir_path, ignore_errors=True)

remove_unwanted_dirs(lib_install_path)

print("python stblib installed")
