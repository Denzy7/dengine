#include "os.h"
#include "dengine_config.h"
#include "logging.h"

#include <dirent.h> //opendir
#include <stdio.h> //fopen, remove
#include <string.h>//strdup, strlen
#include <stdlib.h>//malloc

#include "window.h"

//Will this work on Win32?
#define PATH_SEP "/"
#if defined (DENGINE_LINUX)
#include <sys/stat.h>
int init = 0;
#elif defined(DENGINE_WIN32)
int init = 1;
#include <windows.h>
#endif

#ifdef DENGINE_HAS_GTK3
#include <gtk/gtk.h>
#endif

#define ERROR_TAG "ERROR::DENGINE_UTILS::OS"
#define WARNING_TAG "WARNING::DENGINE_UTILS::OS"


void dengineutils_os_init(int argc, char** argv)
{
#ifdef DENGINE_HAS_GTK3
    //fire up gtk
    gtk_init(&argc, &argv);
    init = 1;
#endif
}

void _dengineutils_os_init_null()
{
    dengineutils_os_init(0, NULL);
    //dengineutils_logging_log("%s::was not init before using GUI. Init with NULL", WARNING_TAG);
    init = 1;
}


int dengineutils_os_mkdir(const char* directory)
{
    int ok;
    //TODO : CLEAN THIS SHIT UP!
    const  char* ok_file = "ok.file";
    size_t directory_len = strlen(directory) + strlen(ok_file);
    char* path = strdup(directory);
    char* path_build = malloc(directory_len);
    memset(path_build, 0, directory_len);

    char* token;

    //for linux
    if(path[0] == PATH_SEP[0])
        strncat(path_build, PATH_SEP, directory_len);

    token = strtok(path, PATH_SEP);

    while( token != NULL ) {
        strncat(path_build, token, directory_len);
        strncat(path_build, PATH_SEP, directory_len);

        //syscalls...
        #ifdef DENGINE_LINUX
        mkdir(path_build, S_IRWXU);
        #endif

        #ifdef DENGINE_WIN32
        CreateDirectory(path_build, NULL);
        #endif

        //dengineutils_logging_log("%s", path_build);

        token = strtok(NULL, PATH_SEP);
    }

    DIR* okdir = opendir(path_build);

    if(okdir)
    {
        ok = 1;
    }else
    {
        ok = 0;
    }

    free(path);
    free(path_build);

    return ok;
}

void dengineutils_os_dialog_messagebox(const char* title, const char* message, int error)
{
    if(!init)
        _dengineutils_os_init_null();

#if defined (DENGINE_WIN32)
    if(error)
        MessageBox( NULL , message, title, MB_OK | MB_ICONERROR);
    else
        MessageBox( NULL , message, title, MB_OK | MB_ICONINFORMATION);
#elif defined (DENGINE_HAS_GTK3)
    int type = error ? GTK_MESSAGE_ERROR : GTK_MESSAGE_INFO;
    GtkWidget* dialog = gtk_message_dialog_new(
        NULL, GTK_DIALOG_MODAL, type, GTK_BUTTONS_OK,
        "%s",title );

    gtk_message_dialog_format_secondary_text(
        GTK_MESSAGE_DIALOG(dialog),
        "%s",message);

    gtk_dialog_run(GTK_DIALOG(dialog));

    gtk_widget_destroy(dialog);
#else
    printf("This would be a message box sent to the Windowing API for platform\n");
#endif

    //JNI toast message Android
    //https://github.com/demuyan/AndroidNDKbook/blob/master/ch02/JniToast/jni/jnitoast.c
}

char* dengineutils_os_dialog_fileopen(const char* title)
{
    if(!init)
        _dengineutils_os_init_null();
    char *filename = NULL;
#if defined(DENGINE_HAS_GTK3)
    GtkWidget* dialog = gtk_file_chooser_dialog_new(title,NULL,
                                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Open", GTK_RESPONSE_ACCEPT,
                                                    NULL);

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
       filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
    }

    gtk_widget_destroy(dialog);
#elif defined(DENGINE_WIN32)
        OPENFILENAME ofn;
        filename = malloc(1024);

        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL;
        ofn.lpstrTitle = title;
        ofn.lpstrFile = filename;
        ofn.lpstrFile[0] = 0;
        ofn.nMaxFile = 1024;

        if(!GetOpenFileName(&ofn))
        {
            //free
            free(filename);
            filename =NULL;
        }
#else
    //Use good ol' CLI
    filename = malloc(1024);
    dengineutils_logging_log("INFO::Enter a file to Open (1024 chars)");
    scanf("%1024s", filename);

#endif

    return filename;
}

char* dengineutils_os_dialog_filesave(const char* title)
{
    if(!init)
        _dengineutils_os_init_null();

    char* filename = NULL;
#if defined(DENGINE_HAS_GTK3)
    GtkWidget* dialog = gtk_file_chooser_dialog_new(title,NULL,
                                                    GTK_FILE_CHOOSER_ACTION_SAVE,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Save", GTK_RESPONSE_ACCEPT,
                                                    NULL);

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
       filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
    }

    gtk_widget_destroy(dialog);


#elif defined(DENGINE_WIN32)
        OPENFILENAME ofn;
        filename = malloc(1024);

        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL;
        ofn.lpstrTitle = title;
        ofn.lpstrFile = filename;
        ofn.lpstrFile[0] = 0;
        ofn.nMaxFile = 1024;

        if(!GetSaveFileName(&ofn))
        {
            //free
            free(filename);
            filename =NULL;
        }
#else
    //Use good ol' CLI
    filename = malloc(1024);
    dengineutils_logging_log("INFO::Enter where to save (1024 chars)");
    scanf("%1024s", filename);
#endif
    return filename;
}
