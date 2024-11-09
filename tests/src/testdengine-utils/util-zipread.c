#include <stdlib.h>

#include <dengine-utils/zipread.h>
#include <dengine-utils/logging.h>
#include <dengine-utils/macros.h>

int main(int argc, char *argv[])
{
    const char* zip = argv[1];
    if(!zip)
    {
        dengineutils_logging_log("ERROR::Pass a zipfile to argv[0]");
        return -1;
    }

    Stream zipstream ; 
    if(!dengineutils_stream_new(zip,  DENGINEUTILS_STREAM_TYPE_FILE, DENGINEUTILS_STREAM_MODE_READ, &zipstream))
    {
        dengineutils_logging_log("ERROR::Failed to create zipstream");
        return -1;
    }

    ZipRead zipread;
    int read = dengineutils_zipread_load(&zipstream, &zipread);
    if(!read)
    {
        dengineutils_logging_log("ERROR::Failed to read zip");
        return -1;
    }
    dengineutils_logging_log("INFO::%u Central Directory Records", zipread.eocdr.cd_records);

    char compr_meth[30];
    for(uint16_t i = 0; i < zipread.eocdr.cd_records; i++)
    {
        CDFHR* cdfhr = &zipread.cdfhrs[i];
        if(cdfhr->compression == 0)
            snprintf(compr_meth, sizeof(compr_meth), "%s", "Store");
        else if(cdfhr->compression == 8)
            snprintf(compr_meth, sizeof(compr_meth), "%s", "Deflate");
        else
            snprintf(compr_meth, sizeof(compr_meth), "%s - (%hu)", "Unsupported", cdfhr->compression);
        dengineutils_logging_log("TODO::"
                                 "%hu\n"
                                 "file:%s\n"
                                 "compr:%u, uncompr:%u\n"
                                 "time:%hu, data:%hu\n"
                                 "compr_meth:%s\n"
                                 "encr:%s, crc:%s, utf-8:%s",
                                 i,
                                 cdfhr->name,
                                 cdfhr->sz_compressed, cdfhr->sz_uncompressed,
                                 cdfhr->lastmod_time, cdfhr->lastmod_date,
                                 compr_meth,
                                 DENGINE_ISBITSET(cdfhr->flags, 0) ? "yes" : "no",
                                 DENGINE_ISBITSET(cdfhr->flags, 3) ? "yes" : "no",
                                 DENGINE_ISBITSET(cdfhr->flags, 11) ? "yes" : "no"
                                 );
    }

    dengineutils_logging_log("INFO::%u Central Directory Records Done!", zipread.eocdr.cd_records);

    void* zip2mem;
    uint32_t zip2mem_sz;
    if(dengineutils_zipread_decompress_cdfhr_mem(&zipstream, &zipread.cdfhrs[0], &zip2mem, &zip2mem_sz))
    {
        dengineutils_logging_log("Read cdfhr[0] [%s] to mem %u", zipread.cdfhrs[0].name, zip2mem_sz);
        FILE* f = fopen("zip2mem", "wb");
        if(f)
        {
            fwrite(zip2mem, 1, zip2mem_sz, f);
            fclose(f);
            dengineutils_logging_log("INFO::write zip2mem");
            free(zip2mem);
        }
    }

    const char* zipout = "zipout";
    if(dengineutils_zipread_decompress_zip(&zipstream, &zipread, zipout))
    {
        dengineutils_logging_log("INFO::extracted to %s", zipout);
        dengineutils_zipread_free(&zipread);
    }

    dengineutils_stream_destroy(&zipstream);
    return 0;
}
