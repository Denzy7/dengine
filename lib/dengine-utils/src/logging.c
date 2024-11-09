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
#include <stdlib.h>
#include <time.h> //ctime
#ifdef DENGINE_WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <dirent.h>
#include <sys/inotify.h>
#include <sys/poll.h>
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

typedef enum 
{
    TARGET_NONE,
    TARGET_INFO,
    TARGET_ERROR,
    TARGET_WARNING,
    TARGET_TODO,
    TARGET_GL,
}LogTarget;
typedef struct
{
    const char* triggerstr;
    ConsoleColor color;
    LogTarget target;

}LogPair;

static const LogPair logpairs[]=
{
    {"INFO::", DENGINE_LOGGING_COLOR_GREEN, TARGET_INFO},
    {"ERROR::", DENGINE_LOGGING_COLOR_RED, TARGET_ERROR},
    {"WARNING::", DENGINE_LOGGING_COLOR_YELLOW, TARGET_WARNING},
    {"TODO::", DENGINE_LOGGING_COLOR_CYAN, TARGET_TODO},
    {"GL::", DENGINE_LOGGING_COLOR_MAGENTA, TARGET_GL},
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
    Condition donecond;
    /* win32 completely break if we idle wait */
    dengineutils_thread_condition_create(&donecond);
    dengineutils_thread_create(_dengineutils_logging_logthr, &donecond, &logthr);
    dengineutils_thread_set_name(&logthr, "LogThread");
    dengineutils_thread_condition_wait(&donecond, &logthrstarted);
    dengineutils_thread_condition_destroy(&donecond);

    if(logthrstarted == -1)
    {
        dengineutils_logging_log("ERROR::logthread exited with error");
        logthrstarted = 0;
        return 0;
    }
    memset(&logcallbacks, 0, sizeof(logcallbacks));
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
    /*TODO: programs(grep,...) usually have a --color=always
     * and check tty before changing terminal color. add similar option
     */
/*    if(isatty( fileno(stdout)))*/
        /*printf("%s", colstr);*/
    if(!logthrstarted) /* recipe for disaster to color when logthread running */ 
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

    const LogPair* pair = NULL;
    const char* delim = "";
    LogTarget target = TARGET_NONE;
    for(int i = 0; i < DENGINE_ARY_SZ(logpairs); i++)
    {
        if(strstr(LOG_BUFFER, logpairs[i].triggerstr))
        {
            delim =  logpairs[i].triggerstr;
            pair = &logpairs[i];
            target = pair->target;
        }
    }
#ifdef DENGINE_ANDROID
    //use logcat. any stdout writes will be sent to callback
    if(target == TARGET_ERROR)
        ANDROID_LOGE("%s",LOG_BUFFER + strlen(delim));
    else if(target == TARGET_WARNING)
        ANDROID_LOGW("%s",LOG_BUFFER + strlen(delim));
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

    if(target == TARGET_ERROR && msgboxerr)
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
    const char* logfilename = "stdout_logthr.log";
    FILE *redir_stdout;
    char buf[1024];
    char vbuf[2048];
    /*long lastoff = 0, off = 0;*/

#ifdef DENGINE_WIN32
    HANDLE logdirwatchdir = INVALID_HANDLE_VALUE;
    DWORD rd;
#else
    int inotifyfd = -1;
    char inotifyevent[sizeof(struct inotify_event) + NAME_MAX + 1];
    struct pollfd pollfd;
#endif

    char* logdir = strdup(dengineutils_logging_get_logfile());
    *strrchr(logdir, '/') = 0;
    size_t logfile_sz = strlen(logdir) + strlen(logfilename) + 2; 
    char* logfile = malloc( logfile_sz );
    snprintf(logfile, logfile_sz, "%s/%s", logdir, logfilename);

    dengineutils_logging_log(
            "WARNING::Log thread about start. stdout will be redirected to registered callbacks using file [ %s].\n"
            "This file just contains a copy of the log buffer and should not be used\n"
            "fprintf(stderr, ...) is still usable", logfile);

    if((redir_stdout = freopen(logfile, "wb", stdout)) == NULL)
    {
        dengineutils_logging_log("ERROR::cant freopen\n");
        goto thrfail;
    }
    
    memset(vbuf, 0, sizeof(vbuf));
    setvbuf(redir_stdout, vbuf, _IOLBF, sizeof(vbuf));

#ifdef DENGINE_WIN32
    /* win32 brain fart */
    for(size_t i = 0; i < strlen(logdir); i++)
    {
        if(logdir[i] == '/')
            logdir[i] = '\\';
    }
    for(size_t i = 0; i < strlen(logfile); i++)
    {
        if(logfile[i] == '/')
            logfile[i] = '\\';
    }
    logdirwatchdir = CreateFile(logdir, 
            FILE_LIST_DIRECTORY, 
            FILE_SHARE_READ, 
            NULL, 
            OPEN_EXISTING, 
            FILE_FLAG_BACKUP_SEMANTICS,
            NULL);
#elif defined (DENGINE_LINUX)
    if((inotifyfd = inotify_init1(IN_NONBLOCK)) < 0)
    {
        dengineutils_logging_log("ERROR::cant init inotify\n");
        goto thrfail;
    }

    if(inotify_add_watch(inotifyfd, logfile, IN_MODIFY) < 0)
    {
        dengineutils_logging_log("ERROR::cant inotify_add_watch\n");
        goto thrfail;
    }
    
#endif

    Condition * cond = arg;
    dengineutils_thread_condition_raise(cond);
    logthrstarted = 1;
    
    while (logthrstarted) {
        memset(buf, 0, sizeof(buf));
        memset(vbuf, 0, sizeof(vbuf));

#ifdef DENGINE_WIN32
        /* TODO; file probing on win32 is a nightmare tbh.
         * this only flawlessly works if the explorer
         * shell probes for file changes
         *
         * IDK WHY \(-.-)/
         */
        if(!ReadDirectoryChangesW(
                logdirwatchdir,
                buf,
                sizeof(buf),
                FALSE,
                FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_LAST_ACCESS | FILE_NOTIFY_CHANGE_ATTRIBUTES, 
                &rd, NULL, NULL))
        {
            fprintf(stderr, "readdir failed\n");
            goto threxit;
        }
        FILE_NOTIFY_INFORMATION* info = (FILE_NOTIFY_INFORMATION*)buf;
        size_t chars = wcstombs(NULL, info->FileName, 0);
        char* filename = calloc(chars + 1, 1);
        wcstombs(filename, info->FileName, info->FileNameLength);
        do{
            if(info->Action == FILE_ACTION_MODIFIED && strcmp(filename, logfile))
            {
                break;
            }
            info = info->NextEntryOffset ? (FILE_NOTIFY_INFORMATION*)(((void*)info) + info->NextEntryOffset) : NULL;
        }while(info);
        free(filename);
        memset(buf, 0, sizeof(buf));
#else
        memset(&pollfd, 0, sizeof(pollfd));
        pollfd.fd = inotifyfd;
        pollfd.events = POLLIN;
        if(poll(&pollfd, 1, -1) && pollfd.revents & POLLIN)
        {
            read(inotifyfd, inotifyevent, sizeof(inotifyevent));
        }
#endif
        LoggingCallbackVtor* cbs = logcallbacks.data;
        if(strrchr(vbuf, '\n'))
            *strrchr(vbuf, '\n') = 0;
        size_t vbufln = strlen(vbuf);
        for(size_t i = 0; i < logcallbacks.count; i++)
        {
            if(cbs && vbufln)
            {
                LoggingCallback cb = cbs[i].cb;
                /*TODO: raw unifiltered log? */
                cb(vbuf, vbuf);
            }
        }
    }

    logthrstarted = 0;
    goto threxit;

thrfail:
    logthrstarted = -1;
threxit:
#ifdef DENGINE_WIN32
    if(GetLastError())
    {
        DWORD dw = GetLastError();
        LPSTR lpMsgBuf;
        FormatMessageA(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                dw,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPSTR)&lpMsgBuf,
                0, NULL );

        fprintf(stderr, "logthread win32: %s\n", lpMsgBuf);
        LocalFree(lpMsgBuf);
    }
    CloseHandle(logdir);
#else
    close(inotifyfd);
#endif
    free(logdir);
    free(logfile);
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
    ANDROID_LOGI("std: %s", log);
}
#endif
