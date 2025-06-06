#ifndef DENGINEUTILS_ZIPREAD_H
#define DENGINEUTILS_ZIPREAD_H

/*! \file zipread.h
 *  Very basic Reading and extracting zip files
 *
 *  Works only with assets/scripts/bootstrap.zip
 *  and similar archives created with
 *  http://www.info-zip.org/Zip.html
 *  `zip` command as it was its initial use
 *
 *  ZIP-64 is not yet implemented!
 */

#include <stdint.h> //uint16,32
#include "dengine-utils/stream.h" //stream seek, read

#pragma pack(push, 1)
typedef struct
{
    uint32_t eoc; /*!< End of central directory magic number */

    uint16_t disk_no;
    uint16_t disk_cd_start;
    uint16_t disk_cd_records;

    uint16_t cd_records;
    uint32_t cd_size;

    uint32_t off_cd;

    uint16_t comment_sz;
    char* comment;
}EOCDR;

typedef struct
{
    uint32_t cd; /*!< Central Directory magic number */

    uint16_t version_made; /*!< Version that created this file */
    uint16_t version_extract; /*!< Version to extract this file (min) */

    uint16_t flags; /*!< Bit bitflags */

    uint16_t compression; /*!< 0=None, 8=DEFLATE */

    uint16_t lastmod_time; /*!< Last modified time MS-DOS */
    uint16_t lastmod_date; /*!< Last modified date MS-DOS */

    uint32_t crc32; /*!< CRC32 if bit is set */

    uint32_t sz_compressed; /*!< Compressed size */
    uint32_t sz_uncompressed; /*!< Uncompressed size */
    uint16_t sz_name; /*!< Name length (no null terminator) */
    uint16_t sz_extra; /*!< Length of extra options  */
    uint16_t sz_comment; /*!< Length of comment */

    uint16_t disk; /*!< Disk file belongs to */

    uint16_t attrs_internal; /*!< Internal attributes */
    uint32_t attrs_external; /*!< External attributes */

    uint32_t off_lfh; /*!< Offset to file header record */

    char* name; /*!< Name of file */
    uint8_t* extra; /*!< Extra options */
    char* comment; /*!< File comment */
}CDFHR;
#pragma pack(pop)

typedef struct
{
    EOCDR eocdr;
    CDFHR* cdfhrs;
}ZipRead;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

int dengineutils_zipread_load(Stream* stream, ZipRead* zipread);

void dengineutils_zipread_free(const ZipRead* zipread);

int dengineutils_zipread_find_cdfhr(const char* path, CDFHR** cdfhr, const ZipRead* zipread); 

int dengineutils_zipread_decompress_cdfhr_mem(Stream* stream, const CDFHR* cdfhr, void** dest, uint32_t* size);

int dengineutils_zipread_decompress_cdfhr(Stream* stream, const CDFHR* cdfhr, const char* dest);

int dengineutils_zipread_decompress_zip(Stream* stream, const ZipRead* zipread, const char* dest);
#ifdef __cplusplus
}
#endif // __cplusplus

#endif // DENGINEUTILS_ZIPREAD_H
