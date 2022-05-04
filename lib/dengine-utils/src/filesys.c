#include "dengine-utils/filesys.h"
#include "dengine-utils/os.h"

#include <stdio.h>  //fopen, fread
#include <stdlib.h> //malloc
#include <string.h> //str

#include "dengine-utils/logging.h"

const size_t dirbuflen = 2048;
char* srcdir = NULL,* assetdir = NULL,* cachedir = NULL,* filesdir = NULL;
int hasloggedassetdir=0, filesysinit = 0;

int dengineutils_filesys_init()
{
    if(filesysinit)
        return 0;

    srcdir = calloc(dirbuflen, sizeof(char));
    assetdir = calloc(dirbuflen, sizeof(char));
    cachedir = calloc(dirbuflen, sizeof(char));
    filesdir = calloc(dirbuflen, sizeof(char));
    filesysinit = assetdir && srcdir && cachedir && filesdir;

    return filesysinit;
}

int dengineutils_filesys_isinit()
{
    return filesysinit;
}

void dengineutils_filesys_terminate()
{
    free(assetdir);
    free(srcdir);
    free(cachedir);
    free(filesdir);
}

int dengineutils_filesys_file2mem_load(File2Mem* file2mem)
{
    file2mem->size = 0;
    FILE* fp = fopen(file2mem->file, "rb");
    if(!fp)
    {
        dengineutils_logging_log("ERROR::DENGINE_UTILS_FILE2MEM::CANNOT_READ::%s", file2mem->file);
        return 0;
    }else
    {
        fseek(fp, 0, SEEK_END);
        file2mem->size = ftell(fp);
        rewind(fp);

        // +1 = '\0'
        file2mem->mem = malloc(file2mem->size + 1);
        memset(file2mem->mem, 0, file2mem->size + 1);
        if(file2mem)
        {
            fread(file2mem->mem, 1, file2mem->size, fp);
        }else
        {
            dengineutils_logging_log("ERROR::DENGINE_UTILS_FILE2MEM::malloc failed!");
            return 0;
        }

        fclose(fp);
        return 1;
    }
}

void dengineutils_filesys_file2mem_free(File2Mem* file2mem)
{
    if(file2mem->mem)
        free(file2mem->mem);
    else
        dengineutils_logging_log("ERROR::DENGINE_UTILS_FILE2MEM::can't free invalid memory!");
}

const char* dengineutils_filesys_get_srcdir()
{
    if(!srcdir)
    {
        dengineutils_logging_log("ERROR::cannot get srcdir. filesys has not been init!");
        return NULL;
    }
    memset(srcdir, 0, dirbuflen);
    #ifndef DENGINE_HIDESRCDIR
    const char* thisfile = __FILE__;
    const char* thisfile_dir = strstr(thisfile, "dengine");
    if (thisfile_dir) {
        size_t strln = strlen(thisfile) - strlen(thisfile_dir) + strlen("dengine");
        strncpy(srcdir, thisfile, strln);
    }
    #endif
    if (strlen(srcdir)==0)
        dengineutils_logging_log("WARNING::srcdir was hidden by distributor");

    return srcdir;

}

const char* _dengineutils_filesys_get_assetsdir_resolve(const char* dir)
{
    //look for default vert shader. expand this to check sth cool like an md5 hash or file list
    const char* dftshdrvert="shaders/default.vert.glsl";

    snprintf(assetdir, dirbuflen, "%s/assets/%s", dir,dftshdrvert);
    if (fopen(assetdir, "rb")) {
        snprintf(assetdir, dirbuflen, "%s/assets", dir);

        if(!hasloggedassetdir)
        {
            dengineutils_logging_log("INFO::using asset dir %s", assetdir);
            hasloggedassetdir=1;
        }
        return assetdir;
    }

    return NULL;
}

const char* dengineutils_filesys_get_assetsdir()
{
#ifdef DENGINE_ANDROID
    //load from AAssetManager
    return "";
#endif

    if(!assetdir)
    {
        dengineutils_logging_log("ERROR::cannot get assetsdir. filesys has not been init!");
        return NULL;
    }

    //Check for assets from cwd compile
    if (_dengineutils_filesys_get_assetsdir_resolve(dengineutils_os_get_cwd()))
        return assetdir;

    //Check for assets from envvar
    const char* envvar = getenv("DENGINEASSETS");
    if(envvar)
    {
        char* envvardup=strdup(envvar);

        //ensure no "assets" in str
        char* aasetsinstr=strstr(envvardup,"assets");
        if(aasetsinstr)
            memset(aasetsinstr,0,strlen("assets"));

        //may have naively put a '/' or '\' at end
        if(envvardup[strlen(envvardup)-1] == DENGINE_PATH_SEP)
            envvardup[strlen(envvardup)-1]=0;

        //resolve...
        const char* assets=_dengineutils_filesys_get_assetsdir_resolve(envvardup);
        free(envvardup);

        if(assets)
            return assets;
    }

    //Check for assets from dengine filesdir
    if (_dengineutils_filesys_get_assetsdir_resolve(dengineutils_filesys_get_filesdir_dengine()))
    {
        return assetdir;
    }

    /*
     * if all fails, use compile dir. this is very very very risky if sources compiled
     * on one machine try looking this up
     *
     * like for eg.
     * - compiling sources on Linux then running a test in wine
     * - setting python path with a linux path on a windows build
     */

    if (_dengineutils_filesys_get_assetsdir_resolve(dengineutils_filesys_get_srcdir()))
    {
        dengineutils_logging_log("WARNING::Using srcdir as assets dir");
        return assetdir;
    }


    dengineutils_logging_log("ERROR::Could not find assets directory.\n"
                             "Try:\n\t"
                             "-moving it next to the executable\n\t"
                             "-moving it to %s\n\t"
                             "-setting envvar DENGINEASSETS\n\t"
                             "-recompiling sources on this machine",
                             dengineutils_filesys_get_filesdir_dengine());

    return NULL;
}

const char* dengineutils_filesys_get_filesdir()
{
#ifdef DENGINE_ANDROID
    //Set by jni by dengine_android_set_filesdir
    return filesdir;
#elif defined(DENGINE_LINUX)
    snprintf(filesdir, dirbuflen, "%s/.local/share",getenv("HOME"));
#elif defined(DENGINE_WIN32)
    snprintf(filesdir, dirbuflen, "%s", getenv("APPDATA"));
#endif
    dengineutils_os_mkdir(filesdir);
    return filesdir;
}

const char* dengineutils_filesys_get_filesdir_dengine()
{
    char* files = strdup(dengineutils_filesys_get_filesdir());
    snprintf(filesdir, dirbuflen, "%s/dengine", files);
    free(files);
    dengineutils_os_mkdir(filesdir);
    return filesdir;
}

const char* dengineutils_filesys_get_cachedir()
{
#ifdef DENGINE_ANDROID
    //Set by jni by dengine_android_set_cachedir
    return cachedir;
#elif defined(DENGINE_LINUX)
    snprintf(cachedir, dirbuflen, "%s/.cache",getenv("HOME"));
#elif defined(DENGINE_WIN32)
    snprintf(cachedir, dirbuflen, "%s", getenv("TEMP"));
#endif
    dengineutils_os_mkdir(cachedir);
    return cachedir;
}

void dengineutils_filesys_set_filesdir(const char* dir)
{
    if(!filesdir)
    {
        dengineutils_logging_log("ERROR::cannot set filesdir. filesys has not been init!");
        return;
    }

    snprintf(filesdir, dirbuflen, "%s", dir);
}

void dengineutils_filesys_set_cachedir(const char* dir)
{
    if(!cachedir)
    {
        dengineutils_logging_log("ERROR::cannot set cachedir. filesys has not been init!");
        return;
    }

    snprintf(cachedir, dirbuflen, "%s", dir);
}
