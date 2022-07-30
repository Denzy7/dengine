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
#include <unistd.h>
#endif
#ifdef DENGINE_WIN32
#include <windows.h>
#endif
#ifdef DENGINE_ANDROID
//android logcat
#include <android/log.h>
#define ANDROID_LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, DENGINE_ANDROID_LOG_TAG, __VA_ARGS__))
#define ANDROID_LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, DENGINE_ANDROID_LOG_TAG, __VA_ARGS__))
#define ANDROID_LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, DENGINE_ANDROID_LOG_TAG, __VA_ARGS__))
#endif

#ifdef DENGINE_WIN32
//WIN32 COLORS
#define Col_Red "\x0c"
#define Col_Green "\x0a"
#define Col_Blue "\x09"

#define Col_Cyan "\x0b"
#define Col_Yellow "\x0e"
#define Col_Magenta "\x0d"

#define Col_White "\x0f"
#define Col_Reset Col_White
#else
//ANSI COLORS
#define Col_Red "\033[0;31m"
#define Col_Green "\033[0;32m"
#define Col_Blue "\033[0;34m"

#define Col_Cyan "\033[0;36m"
#define Col_Yellow "\033[0;33m"
#define Col_Magenta "\033[0;35m"

#define Col_White "\033[0;37m"
#define Col_Reset "\033[0m"
#endif

#ifdef DENGINE_ANDROID
void _dengineutils_logging_androidcb(const char* log, const char* trip);
#endif

static char* colors[]=
{
    Col_Red,
    Col_Green,
    Col_Blue,

    Col_Cyan,
    Col_Yellow,
    Col_Magenta,

    Col_White,
    Col_Reset,
};

typedef struct
{
    const char* triggerstr;
    ConsoleColor color;
}_LogPair;

static const _LogPair logpairs[]=
{
    {"INFO::", DENGINE_LOGGING_COLOR_GREEN},
    {"ERROR::", DENGINE_LOGGING_COLOR_RED},
    {"WARNING::", DENGINE_LOGGING_COLOR_YELLOW},
    {"TODO::", DENGINE_LOGGING_COLOR_CYAN},
    {"GL::", DENGINE_LOGGING_COLOR_MAGENTA},
};
//PLAFORM SPECIFIC GLOBAL VARS
#ifdef DENGINE_LINUX
int logfd[2];
#elif defined(DENGINE_WIN32)
HANDLE hLogPipe;
WORD stdoutInitAttrs = 0;
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

//messagebox on error disabled (potential gtk memory leaks after gtk_init!!!)
int msgboxerr = 0;

int dengineutils_logging_init()
{
    Thread logthr;
    dengineutils_thread_create(_dengineutils_logging_logthr, NULL, &logthr);
    vtor_create(&logcallbacks, sizeof(LoggingCallbackVtor));

    // android stdout callback
#ifdef DENGINE_ANDROID
    dengineutils_logging_addcallback(_dengineutils_logging_androidcb);
#endif

    return 1;
}

void dengineutils_logging_terminate()
{
    logthrstarted = 0;
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

void dengineutils_logging_set_consolecolor(ConsoleColor colour)
{
    const char* colstr = colors[colour];
#if defined(DENGINE_LINUX)
    printf("%s", colstr);
#elif defined(DENGINE_WIN32)
    //Win32 set console
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if(stdoutInitAttrs == 0)
    {
        CONSOLE_SCREEN_BUFFER_INFO Info;
        GetConsoleScreenBufferInfo(hConsole, &Info);
        stdoutInitAttrs = Info.wAttributes;
    }

    if(colour == DENGINE_LOGGING_COLOR_RESET && stdoutInitAttrs != 0)
        SetConsoleTextAttribute(hConsole, stdoutInitAttrs);
    else
        SetConsoleTextAttribute(hConsole, colstr[0]);
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

    const _LogPair* pair = NULL;

    const char* delim = "";
    for(int i = 0; i < DENGINE_ARY_SZ(logpairs); i++)
    {
        if(strstr(LOG_BUFFER, logpairs[i].triggerstr))
        {
            delim =  logpairs[i].triggerstr;
            pair = &logpairs[i];
        }
    }
#ifdef DENGINE_ANDROID
    //use logcat. any stdout writes will be sent to callback
    if(iserr)
        ANDROID_LOGE("%s",LOG_BUFFER + strlen(delim));
    else
        ANDROID_LOGI("%s",LOG_BUFFER + strlen(delim));
#else
    printf("\n");
    if(pair != NULL)
        dengineutils_logging_set_consolecolor(pair->color);
    printf("%s", LOG_BUFFER + strlen(delim));
    if(pair != NULL)
        dengineutils_logging_set_consolecolor(DENGINE_LOGGING_COLOR_RESET);
    printf("\n");

    fflush(stdout);

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
    if(logthrstarted != 0)
    {
        dengineutils_logging_log("ERROR::log thread already started!");
        return NULL;
    }

    dengineutils_logging_log("WARNING::Log thread about start. stdout will be redirected to callback. \n"
                             "fprintf(stderr, \"...<format>...\",...); is still usable (stderr)");

    const char* files = dengineutils_filesys_get_filesdir_dengine();
    char newstdoutname[2048];
    snprintf(newstdoutname, sizeof(newstdoutname), "%s/%s", files, "stdout.log");
    FILE* redir_stdout = freopen(newstdoutname, "w", stdout);
    if(redir_stdout == NULL)
    {
        dengineutils_logging_log("ERROR::Failed to redirect stdout");
        return NULL;
    }
    off_t off_current = 0, off_last = 0;

    logthrstarted = 1;

    while(logthrstarted)
    {
        off_current = ftello(redir_stdout);
        if(off_current != off_last)
        {
            off_last = off_current;
            LoggingCallbackVtor* cbs = logcallbacks.data;
            for(size_t i = 0; i < logcallbacks.count; i++)
            {
                if(cbs)
                {
                    LoggingCallback cb = cbs[i].cb;
                    cb(LOG_BUFFER, NULL);
                }
            }
        }
    }

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

#ifdef DENGINE_ANDROID
void _dengineutils_logging_androidcb(const char* log, const char* trip)
{
    //write stdout and stderr to logcat
    ANDROID_LOGI("std: %s", trip);
}
#endif
