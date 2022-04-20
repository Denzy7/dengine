#include "dengine-utils/logging.h"

#include "dengine-utils/os.h"

#include <stdio.h>  //printf
#include <stdarg.h> //vsprint
#include <string.h> //sprintf
#include <time.h> //ctime

#include "dengine_config.h" //MAX_LOG_STR_SIZE

#include "dengine/macros.h"

#if defined(DENGINE_ANDROID)
#include <android/log.h>
#define ANDROID_LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, DENGINE_ANDROID_LOG_TAG, __VA_ARGS__))
#define ANDROID_LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, DENGINE_ANDROID_LOG_TAG, __VA_ARGS__))
#define ANDROID_LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, DENGINE_ANDROID_LOG_TAG, __VA_ARGS__))
#elif defined (DENGINE_WIN32)
#include <windows.h>
#endif // defined


char LOG_BUFFER[DENGINE_LOG_BUF_SIZE];

//Disable file log by default to avoid crash on Android
int log2file = 0;

//messagebox on error
int msgboxerr=1;


static const char* offsetneedles[]=
{
    "INFO::",
    "ERROR::",
    "WARNING::",
    "TODO::",
};

#ifdef DENGINE_LINUX
//ANSI COLORS
#define  ANSI_Red "\033[0;31m"
#define  ANSI_Green "\033[0;32m"
#define  ANSI_Blue "\033[0;34m"
#define  ANSI_Yellow "\033[0;33m"
#define  ANSI_Cyan "\033[0;36m"
#define  ANSI_White "\033[0;37m"
#define ANSI_Reset "\033[0m"
static const char* logcolorpairsANSI[][2]=
{
    {"I", ANSI_Green},
    {"E", ANSI_Red},
    {"W", ANSI_Yellow},
    {"T", ANSI_Cyan},
};
#elif defined(DENGINE_WIN32)
//WIN32 COLORS
#define WIN32_Red 12
#define WIN32_Green 10
#define WIN32_Blue 9
#define WIN32_Yellow 14
#define WIN32_Cyan 11
#define WIN32_White 15
static const char logcolorpairsWIN32[][2]=
{
    {'I', WIN32_Green},
    {'E', WIN32_Red},
    {'W', WIN32_Yellow},
    {'T', WIN32_Cyan},
};
#endif

void dengineutils_logging_set_filelogging(int value)
{
    log2file = value;
    FILE* f = fopen(DENGINE_LOG_FILE, "a");
    fprintf(f, "\n====================\n");
    fclose(f);
}

void dengineutils_logging_set_consolecolor(char head)
{
#if defined(DENGINE_LINUX)

    if(!head)
    {
        printf(ANSI_Reset);
    }else
    {
        //Color ansi output
        for(int i = 0; i < DENGINE_ARY_SZ(logcolorpairsANSI); i++)
        {
            if(logcolorpairsANSI[i][0][0] == head)
            {
                printf("%s", logcolorpairsANSI[i][1]);
            }
        }
    }
#elif defined(DENGINE_WIN32)
    //Win32 set console
    //TODO : Get current color instead of overriding user color
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    if(!head)
    {
        SetConsoleTextAttribute(hConsole, WIN32_White);
    }else
    {
        for(int i = 0; i < DENGINE_ARY_SZ(logcolorpairsWIN32); i++)
        {
            if(logcolorpairsWIN32[i][0] == head)
            {
                SetConsoleTextAttribute(hConsole, logcolorpairsWIN32[i][1]);
            }
        }
    }
#endif

}

void dengineutils_logging_log(const char* message, ...)
{
    //TODO : Check for incorrect args
    va_list argp;
    va_start(argp, message);
    memset(LOG_BUFFER, 0, sizeof LOG_BUFFER);
    vsnprintf(LOG_BUFFER, sizeof(LOG_BUFFER), message, argp);
    va_end(argp);

    if(log2file)
    {
        time_t t;
        time(&t);
        FILE* f = fopen(DENGINE_LOG_FILE, "a");
        fprintf(f, "%s\t%s\n", ctime(&t), LOG_BUFFER);
        fclose(f);
    }

    const char* delim = "";
    for(int i = 0; i < DENGINE_ARY_SZ(offsetneedles); i++)
    {
        if(strstr(LOG_BUFFER, offsetneedles[i]))
            delim =  offsetneedles[i];
    }

    printf("\n");
    dengineutils_logging_set_consolecolor(LOG_BUFFER[0]);
    printf("%s", LOG_BUFFER + strlen(delim));
    dengineutils_logging_set_consolecolor(0);
    printf("\n");

    #ifdef DENGINE_ANDROID
        ANDROID_LOGI("%s",LOG_BUFFER + strlen(delim));
    #endif // DENGINE_ANDROID

    if(LOG_BUFFER[0] == 'E' && msgboxerr)
    {
        dengineutils_os_dialog_messagebox("Critical Error!", LOG_BUFFER + strlen(delim), 1);
        #ifdef DENGINE_ANDROID
        ANDROID_LOGE("%s",LOG_BUFFER + strlen(delim));
        #endif // DENGINE_ANDROID
    }
}

void dengineutils_logging_set_msgboxerror(int value)
{
    msgboxerr=value;
}

int dengineutils_logging_get_msgboxerror()
{
    return msgboxerr;
}
