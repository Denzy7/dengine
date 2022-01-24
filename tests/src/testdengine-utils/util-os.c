#include <dengine-utils/os.h>
#include <dengine-utils/logging.h>

#include <stdlib.h> //free
int main(int argc, char** argv)
{
    //Not a must. But here to init GTK3
    dengineutils_os_init(argc, argv);

    const char* dir ="testdir/recursive/nonexistdir/yes";

    //Create a recursive directory
    if(dengineutils_os_mkdir(dir))
        dengineutils_logging_log("INFO::created dir %s", dir);
    else
        dengineutils_logging_log("ERROR::cannot create dir %s", dir);

    //Some dialogs
    dengineutils_os_dialog_messagebox("hello", "world", 0);

    dengineutils_os_dialog_messagebox("this is", "an error!", 1);

    //Open/Save file dialog
    char* openfile = dengineutils_os_dialog_fileopen("Open any file...");
    if(openfile)
    {
        dengineutils_os_dialog_messagebox("you tried opening", openfile, 0);
        free(openfile);
    }

    char* savefile = dengineutils_os_dialog_filesave("Type any filename...");
    if(savefile)
    {
        dengineutils_os_dialog_messagebox("you tried saving", savefile, 0);
        free(savefile);
    }

    return 0;
}

