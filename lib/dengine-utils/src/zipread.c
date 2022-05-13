#include "dengine-utils/zipread.h"

#include <stdlib.h> //calloc
#include <string.h> //memcmp

#include "dengine-utils/logging.h"

// magic numbers for various zip sections
static const uint8_t LFH_MGC[] = {0x50, 0x4b, 0x03, 0x04};
static const uint8_t EOCD_MGC[] = {0x50, 0x4b, 0x05, 0x06};

ZipRead* dengineutils_zipread_read(const Stream* stream)
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
        return NULL;
    }
    EOCDR* eocdr = NULL;
    eocdr = (EOCDR*)eocdr_mem;
    if(eocdr->off_cd == UINT32_MAX)
    {
        dengineutils_logging_log("Incompatible with Zip64");
        return NULL;
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
    ZipRead* zip = calloc(1, sizeof(ZipRead));
    zip->eocdr = eocdr;
    zip->cdfhrs = cdfhrs;

    return zip;

}
