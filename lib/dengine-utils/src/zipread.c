#include "dengine-utils/zipread.h"

#include <stdlib.h> //calloc
#include <string.h> //memcmp

#include "dengine-utils/logging.h"
#include "dengine-utils/str.h" //strndup
#include "dengine-utils/os.h" //mkdir,direxist

#include "dengine_config.h"
#ifdef DENGINE_HAS_LIBZ
#include <zlib.h>
#endif

#define OFF_EOCDR 22
#define OFF_CDFHR 46
#define OFF_LFHR 30

// magic numbers for various zip sections
static const uint8_t LFH_MGC[] = {0x50, 0x4b, 0x03, 0x04};
static const uint8_t EOCD_MGC[] = {0x50, 0x4b, 0x05, 0x06};

void _dengineutils_zipread_writeout(const void* mem, const CDFHR* cdfhr, const char* dest);

void _dengineutils_zipread_writeout(const void* mem, const CDFHR* cdfhr, const char* dest)
{
    const char* filename = strrchr((char*)cdfhr->name, '/');
    char finalfilename[8192];
    if(filename)
    {
        // MKDIR
        size_t sz_dir = cdfhr->sz_name - strlen(filename + 1);
        char* dir = dengineutils_str_ndup( (char*) cdfhr->name, sz_dir - 1);
        snprintf(finalfilename, sizeof(finalfilename), "%s/%s", dest, dir);
        free(dir);
    }else
    {
        snprintf(finalfilename, sizeof(finalfilename), "%s", dest);
    }

    if(!dengineutils_os_direxist(finalfilename))
        dengineutils_os_mkdir(finalfilename);

    snprintf(finalfilename, sizeof(finalfilename), "%s/%s", dest, cdfhr->name);

    FILE* out = fopen(finalfilename, "wb");
    if(out)
    {
        fwrite(mem, 1, cdfhr->sz_uncompressed, out);
        fclose(out);
    }
}

int dengineutils_zipread_load(Stream* stream, ZipRead* zipread)
{
    memset(zipread, 0, sizeof(ZipRead));

    char find_eocdr = 0;
    uint8_t eocdr_mgc_test[4];

    /*off_t eocdr_off = */

    dengineutils_stream_seek(stream, -OFF_EOCDR, SEEK_END);
    while(!find_eocdr)
    {
        dengineutils_stream_read(eocdr_mgc_test, 1, 4, stream);

        if(!memcmp(eocdr_mgc_test, EOCD_MGC, 4))
        {
            find_eocdr = 1;
            dengineutils_stream_seek(stream, -4, SEEK_CUR);
            break;
        }
        dengineutils_stream_seek(stream, -5, SEEK_CUR);
        if(stream->pos <= 4)
            break;
    }

    if(!find_eocdr)
    {
        const char* path = stream->path ? stream->path : "This";
        dengineutils_logging_log("ERROR::EOCDR not found. %s is probably not a zip file", path);
        return 0;
    }

    dengineutils_stream_read(&zipread->eocdr, offsetof(EOCDR, comment), 1, stream);

    if(zipread->eocdr.comment_sz){
        zipread->eocdr.comment = calloc(zipread->eocdr.comment_sz + 1, 1);
        dengineutils_stream_read(zipread->eocdr.comment, zipread->eocdr.comment_sz, 1, stream);
    }

    if(zipread->eocdr.off_cd == UINT32_MAX)
    {
        dengineutils_logging_log("ERROR::Zip64 is currently not supported :|");
        return 0;
    }

    //read eocdr comment

    dengineutils_stream_seek(stream, zipread->eocdr.off_cd, SEEK_SET);
    CDFHR* cdfhrs = calloc(zipread->eocdr.cd_records, sizeof(CDFHR));
    for(uint16_t i = 0; i <  zipread->eocdr.cd_records; i++)
    {
        dengineutils_stream_read(&cdfhrs[i].cd, offsetof(CDFHR, name), 1, stream);

        //name
        cdfhrs[i].name = calloc(cdfhrs[i].sz_name + 1, 1);
        dengineutils_stream_read(cdfhrs[i].name, 1, cdfhrs[i].sz_name, stream);

        //extra
        cdfhrs[i].extra = malloc(cdfhrs[i].sz_extra);
        dengineutils_stream_read(cdfhrs[i].extra, 1, cdfhrs[i].sz_extra, stream);

        //comment
        if(cdfhrs[i].sz_comment)
        {
            cdfhrs[i].comment = calloc(cdfhrs[i].sz_comment, 1);
            dengineutils_stream_read(cdfhrs[i].comment, 1, cdfhrs[i].sz_comment, stream);
        }
    }
    zipread->cdfhrs = cdfhrs;

    return 1;
}

int dengineutils_zipread_decompress_zip(Stream* stream, const ZipRead* zipread, const char* dest)
{
    for(uint16_t i = 0; i < zipread->eocdr.cd_records; i++)
    {
        CDFHR* cdfhr = &zipread->cdfhrs[i];

        int dec = dengineutils_zipread_decompress_cdfhr(stream, cdfhr, dest);
        if(!dec)
        {
            return 0;
        }
    }

    return 1;
}

int dengineutils_zipread_decompress_cdfhr_mem(Stream* stream, const CDFHR* cdfhr, void** dest, uint32_t* size)
{
    /*TODO: parse LFHR. some archivers put file info here??? */
    uint8_t lfh_mem[OFF_LFHR];
    dengineutils_stream_seek(stream, cdfhr->off_lfh, SEEK_SET);

    dengineutils_stream_read(lfh_mem, 1, OFF_LFHR, stream);

    int ret = 0;

    if(!memcmp(lfh_mem, LFH_MGC, 4))
    {
        //skip name + extra + empty 4 bytes
        dengineutils_stream_seek(stream, cdfhr->sz_extra + cdfhr->sz_name + 4, SEEK_CUR);

        uint8_t* data = malloc(cdfhr->sz_compressed);
        *size = cdfhr->sz_uncompressed;
        *dest = malloc(*size + 1);
        /* could be string :] */
        ((char*)*dest)[*size] = 0;
        dengineutils_stream_read(data, 1, cdfhr->sz_compressed, stream);

        if(cdfhr->compression == 0)
        {
            //store in dest
            memcpy(*dest, data, *size);
            ret = 1;
        }else if(cdfhr->compression == 8)
        {
#ifdef DENGINE_HAS_LIBZ

            z_stream zs;
            memset(&zs, 0, sizeof(z_stream));

            zs.next_in = data;
            zs.avail_in = cdfhr->sz_compressed;

            /*
             * https://www.zlib.net/manual.html#Advanced
             */
            int res = inflateInit2(&zs, -15);
            if(res != Z_OK)
            {
                dengineutils_logging_log("WARNING::inflateInit failed %s", zs.msg);
            }

            zs.next_out = *dest;
            zs.avail_out = *size;

            res = inflate(&zs, Z_FINISH);

            if(res == Z_STREAM_END)
            {
                ret = 1;
            }else
            {
                dengineutils_logging_log("WARNING::inflate failed with %d\n%s", res, zs.msg);
                ret = 0;
            }
            inflateEnd(&zs);
#else
            dengineutils_logging_log("WARNING::libz not linked. Cannot inflate");
            ret = 0;
#endif
        }else
        {
            dengineutils_logging_log("WARNING::Unsupported compression method");
        }
        free(data);
    }else
    {
        dengineutils_logging_log("ERROR::Cannot find LFHR magic number");
    }
    return ret;
}

int dengineutils_zipread_decompress_cdfhr(Stream* stream, const CDFHR* cdfhr, const char* dest)
{
    void* mem;
    uint32_t memsz;
    if(!dengineutils_zipread_decompress_cdfhr_mem(stream, cdfhr, &mem, &memsz))
        return 0;

    _dengineutils_zipread_writeout(mem, cdfhr, dest);
    return 1;
}

void dengineutils_zipread_free(const ZipRead* zipread)
{
    if(zipread->eocdr.comment_sz)
        free(zipread->eocdr.comment);

    for(uint16_t i = 0; i < zipread->eocdr.cd_records; i++)
    {
        CDFHR* cdfhr = &zipread->cdfhrs[i];
        free(cdfhr->name);
        free(cdfhr->extra);
        free(cdfhr->comment);
    }

    free(zipread->cdfhrs);
}

int dengineutils_zipread_find_cdfhr(const char* path, CDFHR** cdfhr, const ZipRead* zipread)
{
    for(uint32_t i = 0; i < zipread->eocdr.cd_records; i++)
    {
        if(!strcmp(zipread->cdfhrs[i].name, path)){
            *cdfhr = &zipread->cdfhrs[i];
            return 1;
        }
    }
    return 0;
}
