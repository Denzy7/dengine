#include "filesys.h"
#include "dengine-utils/os.h"

#include <stdio.h>  //fopen, fread
#include <stdlib.h> //malloc
#include <string.h> //str

#include "logging.h"

char srcdir[1024],assetdir[1024];
int hasloggedassetdir=0;

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
    memset(srcdir, 0, sizeof (srcdir));
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
    memset(assetdir, 0, sizeof (assetdir));

    //look for default vert shader. expand this to check sth cool like an md5 hash or file list
    const char* dftshdrvert="shaders/default.vert.glsl";

    snprintf(assetdir, sizeof (assetdir), "%s/assets/%s", dir,dftshdrvert);
    if (fopen(assetdir, "rb")) {
        snprintf(assetdir, sizeof (assetdir), "%s/assets", dir);

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
    //Check for assets from compile dir, cwd & envvar
    if (_dengineutils_filesys_get_assetsdir_resolve(dengineutils_filesys_get_srcdir()))
        return assetdir;

    if (_dengineutils_filesys_get_assetsdir_resolve(dengineutils_os_get_cwd()))
        return assetdir;

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

    dengineutils_logging_log("ERROR::Could not find assets directory.\n"
                             "Try:\n\t"
                             "-moving it next to the executable\n\t"
                             "-setting envvar DENGINEASSETS\n\t"
                             "-recompiling sources on this machine");
    return NULL;
}
