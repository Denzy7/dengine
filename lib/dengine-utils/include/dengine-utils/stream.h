#ifndef DENGINEUTILS_STREAM_H
#define DENGINEUTILS_STREAM_H

/*! \file stream.h
 *  File streaming
 */

#include <stddef.h> //size_t
#include <stdio.h> //fopen
#include "dengine_config.h"
#ifdef DENGINE_ANDROID
#include "dengine-utils/platform/android.h"
#include <android/asset_manager.h>
#endif

//#ifdef DENGINE_

typedef enum
{
    DENGINEUTILS_STREAM_TYPE_FILE,
    DENGINEUTILS_STREAM_TYPE_ANDROIDASSET,
}StreamType;

typedef enum
{
    /* ANDROID ASSET STREAM */
#ifdef DENGINE_ANDROID
    DENGINEUTILS_STREAM_MODE_RANDOM = AASSET_MODE_RANDOM,
    DENGINEUTILS_STREAM_MODE_STREAMING = AASSET_MODE_STREAMING,
    DENGINEUTILS_STREAM_MODE_BUFFER = AASSET_MODE_BUFFER,
#endif

    /* FILE STREAM */
    DENGINEUTILS_STREAM_MODE_READ = 4,
    DENGINEUTILS_STREAM_MODE_WRITE,
    DENGINEUTILS_STREAM_MODE_APPEND,


}StreamMode;

typedef struct _Stream
{
    FILE* fp;
    char* path;
#ifdef DENGINE_ANDROID
    AAsset* asset;
#endif
    StreamType type;
    StreamMode mode;
    off_t size;
}Stream;

#ifdef __cplusplus
extern "C" {
#endif

Stream* dengineutils_stream_new(const char* path, StreamType type, StreamMode mode);

size_t dengineutils_stream_read(void* dest, const size_t size, const size_t count, const Stream* stream);

size_t dengineutils_stream_write(const void* src, const size_t size, const size_t count, const Stream* stream);

off_t dengineutils_stream_seek(const Stream* stream, const off_t offset, const int whence);

void dengineutils_stream_destroy(Stream* stream);

#ifdef __cplusplus
}
#endif
#endif // DENGINEUTILS_STREAM_H
