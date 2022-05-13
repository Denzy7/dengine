#include "dengine-utils/zipread.h"

#include <stdlib.h> //calloc
#include <string.h> //memcmp

#include "dengine-utils/logging.h"
#include "dengine-utils/str.h" //strndup
#include "dengine-utils/os.h" //mkdir,direxist

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

int dengineutils_zipread_load(const Stream* stream, ZipRead* zipread)
{
    char find_eocdr = 0;
    uint8_t eocdr_mem[OFF_EOCDR];

    dengineutils_stream_seek(stream, -OFF_EOCDR, SEEK_END);
    dengineutils_stream_read(eocdr_mem, 1, OFF_EOCDR, stream);

    if(!memcmp(eocdr_mem, EOCD_MGC, 4))
    {
        find_eocdr = 1;
    }else
    {
        dengineutils_stream_seek(stream, 0, SEEK_END);
        off_t sz = stream->size;
        while(sz > 0)
        {
            dengineutils_stream_seek(stream, sz - 1, SEEK_SET);
            dengineutils_stream_read(eocdr_mem, 1, 4, stream);
            if(!memcmp(eocdr_mem, EOCD_MGC, 4))
            {
                find_eocdr = 1;
                //read remaining bytes
                dengineutils_stream_read(eocdr_mem + 4, 1, OFF_EOCDR - 4, stream);
                break;
            }
            sz--;
        }
    }

    if(!find_eocdr)
    {
        dengineutils_logging_log("ERROR::EOCDR not found. This is probably not a zip file");
        return 0;
    }
    EOCDR* eocdr = NULL;
    eocdr = (EOCDR*)eocdr_mem;
    if(eocdr->off_cd == UINT32_MAX)
    {
        dengineutils_logging_log("Incompatible with Zip64");
        return 0;
    }

    eocdr = calloc(1, sizeof(EOCDR));
    memcpy(eocdr, eocdr_mem, OFF_EOCDR);

    //read eocdr comment
    if(eocdr->comment_sz)
    {
        eocdr->comment = calloc(eocdr->comment_sz + 1, 1);
        dengineutils_stream_read(eocdr->comment, 1, eocdr->comment_sz, stream);
    }

    dengineutils_stream_seek(stream, eocdr->off_cd, SEEK_SET);
    CDFHR* cdfhrs = calloc(eocdr->cd_records, sizeof(CDFHR));
    for(uint16_t i = 0; i < eocdr->cd_records; i++)
    {
        dengineutils_stream_read(&cdfhrs[i].cd, 4, 1, stream);

        dengineutils_stream_read(&cdfhrs[i].version_made, 2, 1, stream);
        dengineutils_stream_read(&cdfhrs[i].version_extract, 2, 1, stream);

        dengineutils_stream_read(&cdfhrs[i].flags, 2, 1, stream);
        dengineutils_stream_read(&cdfhrs[i].compression, 2, 1, stream);

        dengineutils_stream_read(&cdfhrs[i].lastmod_time, 2, 1, stream);
        dengineutils_stream_read(&cdfhrs[i].lastmod_date, 2, 1, stream);

        dengineutils_stream_read(&cdfhrs[i].crc32, 4, 1, stream);

        dengineutils_stream_read(&cdfhrs[i].sz_compressed, 4, 1, stream);
        dengineutils_stream_read(&cdfhrs[i].sz_uncompressed, 4, 1, stream);

        dengineutils_stream_read(&cdfhrs[i].sz_name, 2, 1, stream);
        dengineutils_stream_read(&cdfhrs[i].sz_extra, 2, 1, stream);
        dengineutils_stream_read(&cdfhrs[i].sz_comment, 2, 1, stream);
        dengineutils_stream_read(&cdfhrs[i].disk, 2, 1, stream);

        dengineutils_stream_read(&cdfhrs[i].attrs_internal, 2, 1, stream);
        dengineutils_stream_read(&cdfhrs[i].attrs_external, 4, 1, stream);

        dengineutils_stream_read(&cdfhrs[i].off_lfh, 4, 1, stream);

        //name
        cdfhrs[i].name = calloc(cdfhrs[i].sz_name + 1, 1);
        dengineutils_stream_read(cdfhrs[i].name, 1, cdfhrs[i].sz_name, stream);

        //extra
        cdfhrs[i].extra = malloc(cdfhrs[i].sz_extra);
        dengineutils_stream_read(cdfhrs[i].extra, 1, cdfhrs[i].sz_extra, stream);

        //comment
        if(cdfhrs[i].sz_comment)
        {
            cdfhrs[i].comment = malloc(cdfhrs[i].sz_comment);
            dengineutils_stream_read(cdfhrs[i].comment, 1, cdfhrs[i].sz_comment, stream);
        }
    }
    zipread->eocdr = eocdr;
    zipread->cdfhrs = cdfhrs;

    return 1;
}

int dengineutils_zipread_decompress_zip(const Stream* stream, const ZipRead* zipread, const char* dest)
{
    for(uint16_t i = 0; i < zipread->eocdr->cd_records; i++)
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

int dengineutils_zipread_decompress_cdfhr(const Stream* stream, const CDFHR* cdfhr, const char* dest)
{
    uint8_t lfh_mem[OFF_LFHR];
    dengineutils_stream_seek(stream, cdfhr->off_lfh, SEEK_SET);

    dengineutils_stream_read(lfh_mem, 1, OFF_LFHR, stream);

    int ret = 0;

    if(!memcmp(lfh_mem, LFH_MGC, 4))
    {
        //skip name + extra + empty 4 bytes
        dengineutils_stream_seek(stream, cdfhr->sz_extra + cdfhr->sz_name + 4, SEEK_CUR);

        uint8_t* data = malloc(cdfhr->sz_compressed);
        dengineutils_stream_read(data, 1, cdfhr->sz_compressed, stream);

        if(cdfhr->compression == 0)
        {
            //store in dest
            _dengineutils_zipread_writeout(data, cdfhr, dest);
            ret = 1;
        }else if(cdfhr->compression == 8)
        {
#ifdef DENGINE_HAS_LIBZ
            uint8_t* uncomp = malloc(cdfhr->sz_uncompressed);

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
                ret = 0;
                goto releaseres;
            }

            zs.next_out = uncomp;
            zs.avail_out = cdfhr->sz_uncompressed;

            res = inflate(&zs, Z_FINISH);

            if(res == Z_STREAM_END)
            {
                _dengineutils_zipread_writeout(uncomp, cdfhr, dest);
                ret = 1;
            }else
            {
                dengineutils_logging_log("WARNING::inflate failed with %d\n%s", res, zs.msg);
                ret = 0;
            }
            inflateEnd(&zs);

            releaseres:

            free(uncomp);
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
