#include <dengine-utils/logging.h>
#include <dengine-utils/filesys.h>
int main(int argc, char *argv[])
{
    //no need to init log to log
    dengineutils_logging_log("hello world");

    //colors
    dengineutils_logging_set_consolecolor(DENGINE_LOGGING_COLOR_GREEN);
    dengineutils_logging_log("hello world!");
    dengineutils_logging_log("green colors!");
    //remember to reset
    dengineutils_logging_set_consolecolor(DENGINE_LOGGING_COLOR_RESET);
    dengineutils_logging_log("default color");

    /*
     * some strings trigger a certain color then reset after print.
     * see lib/dengine-utils/src/logging.c logpairs struct and
     * `logpairs` static const variable
     */
    dengineutils_logging_log("INFO::green for info");
    dengineutils_logging_log("WARNING::yellow for warnings");
    dengineutils_logging_log("GL::magenta for opengl errors");
    dengineutils_logging_log("TODO::cyan for todo. added because its very easy to add new colors!");

    dengineutils_logging_log("ERROR::red for general errors");
    dengineutils_logging_set_msgboxerror(1);
    dengineutils_logging_log("ERROR::msgboxerror throws a messagebox on each error\n"
                             "beware this causes memory leaks if using gtk!\n"
                             "https://stackoverflow.com/questions/16659781/memory-leaks-in-gtk-hello-world-program");

    dengineutils_logging_set_filelogging(1);
    dengineutils_logging_log("WARNING::its possible to log to a file [ %s ] if its writable."
                             "triggers like INFO:: will not be cleared.",
                             dengineutils_logging_get_logfile());

    dengineutils_filesys_init();
    dengineutils_logging_log("INFO::if filesys is init, its will always be writable."
                             " as it will be redirected to a user file [ %s ] "
                             "useful for android",
                             dengineutils_logging_get_logfile());
    dengineutils_filesys_terminate();
    return 0;
}
