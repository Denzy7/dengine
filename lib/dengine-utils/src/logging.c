#include "dengine-utils/logging.h"

#include "dengine_config.h" //MAX_LOG_STR_SIZE

#include "dengine-utils/os.h"
#include "dengine-utils/vtor.h" //callbacks
#include "dengine-utils/filesys.h" //files_dir
#include "dengine-utils/thread.h" //logthr
#include "dengine-utils/macros.h"

#include <stdio.h>  //printf
#include <stdarg.h> //vsprint
#include <string.h> //sprintf
#include <time.h> //ctime

#ifdef DENGINE_LINUX
#include <unistd.h> //pipe2
//ANSI COLORS
#define ANSI_Red "\033[0;31m"
#define ANSI_Green "\033[0;32m"
#define ANSI_Blue "\033[0;34m"
#define ANSI_Yellow "\033[0;33m"
#define ANSI_Cyan "\033[0;36m"
#define ANSI_White "\033[0;37m"
#define ANSI_Reset "\033[0m"
#elif defined(DENGINE_WIN32)
#include <windows.h>
//WIN32 COLORS
#define WIN32_Red 12
#define WIN32_Green 10
#define WIN32_Blue 9
#define WIN32_Yellow 14
#define WIN32_Cyan 11
#define WIN32_White 15
#endif

//android logcat
#ifdef DENGINE_ANDROID
#include <android/log.h>
#define ANDROID_LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, DENGINE_ANDROID_LOG_TAG, __VA_ARGS__))
#define ANDROID_LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, DENGINE_ANDROID_LOG_TAG, __VA_ARGS__))
#define ANDROID_LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, DENGINE_ANDROID_LOG_TAG, __VA_ARGS__))
void _dengineutils_logging_androidcb(const char* log, const char* trip);
#endif

//PLAFORM SPECIFIC GLOBAL VARS
#ifdef DENGINE_LINUX
//logging pair linux
static const char* logcolorpairsANSI[][2]=
{
    {"I", ANSI_Green},
    {"E", ANSI_Red},
    {"W", ANSI_Yellow},
    {"T", ANSI_Cyan},
};
//pipes for logging thread
int logfd[2];
#elif defined(DENGINE_WIN32)
//logging pair win32
static const char logcolorpairsWIN32[][2]=
{
    {'I', WIN32_Green},
    {'E', WIN32_Red},
    {'W', WIN32_Yellow},
    {'T', WIN32_Cyan},
};
HANDLE hLogPipe;
#endif

typedef struct
{
    LoggingCallback cb;
}LoggingCallbackVtor;

// start log thread
void* _dengineutils_logging_logthr(void* arg);
int logthrstarted = 0;
vtor logcallbacks;

//log buffer and file
static char LOG_BUFFER[DENGINE_LOG_BUF_SIZE];
static char LOGFILE[2048];

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

#ifdef DENGINE_ANDROID
void _dengineutils_logging_androidcb(const char* log, const char* trip)
{
    //write stdout and stderr to logcat
    ANDROID_LOGI("std: %s", trip);
}
#endif

int dengineutils_logging_init()
{
    Thread logthr;
    if(dengineutils_thread_create(_dengineutils_logging_logthr, NULL, &logthr))
        logthrstarted = 1;

    vtor_create(&logcallbacks, sizeof(LoggingCallbackVtor));

    // android stdout stderr callback
#ifdef DENGINE_ANDROID
    dengineutils_logging_addcallback(_dengineutils_logging_androidcb);
#endif

    return 1;
}

void dengineutils_logging_terminate()
{
    vtor_free(&logcallbacks);
}

void dengineutils_logging_set_filelogging(int value)
{
    log2file = value;
    FILE* f = fopen(dengineutils_logging_get_logfile(), "a");
    if(f)
    {
        fprintf(f, "\n====================\n");
        fclose(f);
    }
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

    const char* iserr = strstr(LOG_BUFFER, "ERROR::");

    if(log2file)
    {
        time_t t;
        time(&t);
        FILE* f = fopen(dengineutils_logging_get_logfile(), "a");
        if(f)
        {
            fprintf(f, "%s\t%s\n", ctime(&t), LOG_BUFFER);
            fclose(f);
        }
    }

    const char* delim = "";
    for(int i = 0; i < DENGINE_ARY_SZ(offsetneedles); i++)
    {
        if(strstr(LOG_BUFFER, offsetneedles[i]))
            delim =  offsetneedles[i];
    }
#ifdef DENGINE_ANDROID
    //use logcat. any stdout writes will be sent to callback
    if(iserr)
        ANDROID_LOGE("%s",LOG_BUFFER + strlen(delim));
    else
        ANDROID_LOGI("%s",LOG_BUFFER + strlen(delim));
#else
    printf("\n");
    dengineutils_logging_set_consolecolor(LOG_BUFFER[0]);
    printf("%s", LOG_BUFFER + strlen(delim));
    dengineutils_logging_set_consolecolor(0);
    printf("\n");
    if(iserr && msgboxerr)
        dengineutils_os_dialog_messagebox("Critical Error!", LOG_BUFFER + strlen(delim), 1);
#endif
}

void dengineutils_logging_set_msgboxerror(int value)
{
    msgboxerr=value;
}

int dengineutils_logging_get_msgboxerror()
{
    return msgboxerr;
}

void* _dengineutils_logging_logthr(void* arg)
{
#ifdef DENGINE_LINUX
    ssize_t sz;
    //read pipe here with a trip buffer
    char trip[BUFSIZ];

    dengineutils_logging_log("WARNING::Log thread about to start. stdout and stderr will be redirected to callbacks");

    setvbuf(stdout, NULL, _IOLBF, 0); // make stdout line-buffered
    setvbuf(stderr, NULL, _IONBF, 0); // make stderr unbuffered

    //pipe and dup2 stdout and stderr
    pipe(logfd);
    dup2(logfd[1], STDOUT_FILENO);
    dup2(logfd[1], STDERR_FILENO);

    while((sz = read(logfd[0], trip, sizeof trip - 1)) > 0)
    {
        if(trip[sz - 1] == '\n') {
            --sz;
        }
        trip[sz] = 0;  // add null-terminator
        //call callbacks
        LoggingCallbackVtor* cbs = logcallbacks.data;
        for(size_t i = 0; i < logcallbacks.count; i++)
        {
            if(cbs)
            {
                LoggingCallback cb = cbs[i].cb;
                cb(LOG_BUFFER, trip);
            }
        }
    }
#endif
    return NULL;
}

void dengineutils_logging_addcallback(LoggingCallback callback)
{
    LoggingCallbackVtor var;
    var.cb = callback;
    if(logthrstarted)
        vtor_pushback(&logcallbacks, &var);
    else
        dengineutils_logging_log("WARNING::Cannot use callbacks when log thread is not running. call dengineutils_logging_init() or set INIT_OPT enable_logthread=1");
}

const char* dengineutils_logging_get_logfile()
{
    if(dengineutils_filesys_isinit())
    {
        snprintf(LOGFILE, sizeof(LOGFILE), "%s/logs", dengineutils_filesys_get_filesdir_dengine());
        if(!dengineutils_os_direxist(LOGFILE))
            dengineutils_os_mkdir(LOGFILE);
        snprintf(LOGFILE, sizeof(LOGFILE), "%s/logs/%s", dengineutils_filesys_get_filesdir_dengine(), DENGINE_LOG_FILE);
        return LOGFILE;
    }else
    {
        // potentially dangerous if we get a non writable file
        return DENGINE_LOG_FILE;
    }
}
