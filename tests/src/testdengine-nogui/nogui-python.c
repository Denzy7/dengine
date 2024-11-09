#include <dengine/dengine.h>
#include <dengine-script/python/python.h>

int main(int argc, char *argv[])
{
    dengineutils_filesys_init();
    denginescript_init();
    const char* src=
            "import dengine.common as common\n"
            "import dengine.scene as scene\n"
            "import dengine.filesys as filesys\n"
            "import dengine.logging as logging\n"
            "def start():\n"
            "    print('Hello World')\n"
            "    logging.log('INFO::filesdir: ' + filesys.get_filesdir())\n"
            ;
    Script s;
    if(denginescript_python_compile(src, "_dengine_internal/nogui.py", &s))
    {
        denginescript_python_call(&s, DENGINE_SCRIPT_FUNC_START, NULL);
    }
    denginescript_terminate();
    dengineutils_filesys_terminate();
    return 0;
}
