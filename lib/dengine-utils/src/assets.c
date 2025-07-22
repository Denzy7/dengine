#include "dengine-utils/assets.h"

#include <stdio.h>
#include <string.h>

#include "dengine-utils/macros.h"
#include "dengine-utils/stream.h"
#include "dengine-utils/zipread.h"
#include "dengine-utils/logging.h"
#include "dengine-utils/filesys.h"

int _dengineutils_assets_loaded = 0;
ZipRead _dengineutils_assets_zip;
Stream _dengineutils_assets_zipstream;
int dengineutils_assets_zip_load(const char* extrapaths)
{
    memset(&_dengineutils_assets_zip, 0, sizeof(_dengineutils_assets_zip));
    memset(&_dengineutils_assets_zipstream, 0, sizeof(_dengineutils_assets_zipstream));
    /* load assets.zip */
    /*TODO: could we load assets.zip from apk assets? */
#ifndef DENGINE_ANDROID
    char buf[2048];
    /* hit or miss, i guess they never miss huh :( */
    static const char* possible_assetszip_paths[] = 
    {
        "../../../share/dengine-%s/assets.zip",
        "../../share/dengine-%s/assets.zip",
        "../share/dengine-%s/assets.zip",
        "share/dengine-%s/assets.zip",
        "assets.zip",
        "/usr/share/dengine-%s/assets.zip",
        "/usr/local/share/dengine-%s/assets.zip",
    };
    for(int i = 0; i < DENGINE_ARY_SZ(possible_assetszip_paths); i++)
    {
        snprintf(buf, sizeof(buf), possible_assetszip_paths[i], DENGINE_VERSION);
        FILE* ok = fopen(buf, "rb");
        if(ok != NULL)
        {
            fclose(ok);
            if(dengineutils_stream_new(buf, DENGINEUTILS_STREAM_TYPE_FILE, DENGINEUTILS_STREAM_MODE_READ, &_dengineutils_assets_zipstream) && dengineutils_zipread_load(&_dengineutils_assets_zipstream, &_dengineutils_assets_zip))
            {
                dengineutils_logging_log("TODO::load assets.zip from %s. cd_records: %u",
                        buf, _dengineutils_assets_zip.eocdr.cd_records);
                _dengineutils_assets_loaded = 1;
                return 1;

            }
        }
    }
#endif
    return 0;

}
int dengineutils_assets_zip_loaded()
{
    return _dengineutils_assets_loaded;
}
int dengineutils_assets_load(const char* path, void** mem, size_t* length)
{
    File2Mem f2m;
    memset(&f2m, 0, sizeof(f2m));
    CDFHR* cdfhr;
    char buf[2048];

#ifdef DENGINE_ANDROID
    f2m.file = path;
    dengineutils_android_asset2file2mem(&f2m);
#endif

    if(f2m.mem == NULL && _dengineutils_assets_loaded && _dengineutils_assets_zip.eocdr.cd_records > 0 && dengineutils_zipread_find_cdfhr(path, &cdfhr, &_dengineutils_assets_zip))
    {
        dengineutils_zipread_decompress_cdfhr_mem(&_dengineutils_assets_zipstream, cdfhr, &f2m.mem, (uint32_t*)&f2m.size);
        dengineutils_logging_log("TODO::load %s from assets.zip", path);
    }

    if(f2m.mem == NULL)
    {
        snprintf(buf, sizeof(buf),
                "%s/%s",
                dengineutils_filesys_get_assetsdir(), path);
        f2m.file = buf;
        dengineutils_filesys_file2mem_load(&f2m);

    }
    *mem = f2m.mem;
    if(length)
        *length = f2m.size;
    return 1;
}

void dengineutils_assets_zip_free()
{
    dengineutils_zipread_free(&_dengineutils_assets_zip);
    dengineutils_stream_destroy(&_dengineutils_assets_zipstream);
}

