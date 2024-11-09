#include "dengine-utils/stream.h"

#include <stdlib.h> //malloc
#include <string.h> //strdup

#include "dengine-utils/logging.h"

#define DENGINEUTILS_STREAM_MSG_TYPE4ANDROID "ERROR::StreamType=ANDROIDASSET is only for Android"
#define DENGINEUTILS_STREAM_MSG_OPENFAILED "ERROR::Failed to open"
#define DENGINEUTILS_STREAM_MSG_UNKNOWNTYPE "ERROR::Unknown StreamType"
#define DENGINEUTILS_STREAM_MSG_UNKNOWNMODE "ERROR::Unknown StreamMode"
#define DENGINEUTILS_STREAM_MSG_RO "ERROR::StreamType is read-only"

int dengineutils_stream_new(const char* path, StreamType type, StreamMode mode, Stream* stream)
{
    memset(stream, 0, sizeof(Stream));
    if(type == DENGINEUTILS_STREAM_TYPE_FILE)
    {
        FILE* file = NULL;
        if(mode == DENGINEUTILS_STREAM_MODE_READ)
        {
            file = fopen(path, "rb");
        }else if(mode == DENGINEUTILS_STREAM_MODE_WRITE)
        {
            file = fopen(path, "wb");
        }else if(mode == DENGINEUTILS_STREAM_MODE_APPEND)
        {
            file = fopen(path, "ab");
        }else
        {
            dengineutils_logging_log(DENGINEUTILS_STREAM_MSG_UNKNOWNMODE " for [ %s ]", path);
            return 0;
        }

        if(!file)
        {
            dengineutils_logging_log(DENGINEUTILS_STREAM_MSG_OPENFAILED " [ %s ]", path);
            return 0;
        }

        stream->path = strdup(path);
        stream->fp = file;
        stream->type = type;
        stream->mode = mode;
        fseeko(stream->fp, 0, SEEK_END);
        stream->size = ftello(stream->fp);
        rewind(stream->fp);
        return 1;
    }else if(type == DENGINEUTILS_STREAM_TYPE_ANDROIDASSET)
    {
#ifdef DENGINE_ANDROID
        StreamMode _mode = mode;
        if(mode == DENGINEUTILS_STREAM_MODE_READ)
        {
            // assume its buffer
            _mode = DENGINEUTILS_STREAM_MODE_BUFFER;
        }else if(mode > DENGINEUTILS_STREAM_MODE_READ)
        {
            dengineutils_logging_log(DENGINEUTILS_STREAM_MSG_UNKNOWNMODE);
            return 0;
        }

        AAssetManager* mgr = dengineutils_android_get_assetmgr();
        AAsset* asset = AAssetManager_open(mgr, path, _mode);

        if(!asset)
        {
            dengineutils_logging_log(DENGINEUTILS_STREAM_MSG_OPENFAILED " [ %s ]", path);
            return 0;
        }

        stream->path = strdup(path);
        stream->asset = asset;
        stream->type = type;
        stream->mode = _mode;
        stream->size = AAsset_getLength(asset);
        stream->buffer = AAsset_getBuffer(asset);
        return 1;
#else
        dengineutils_logging_log(DENGINEUTILS_STREAM_MSG_TYPE4ANDROID);
        return 0;
#endif
    }else
    {
        dengineutils_logging_log(DENGINEUTILS_STREAM_MSG_UNKNOWNTYPE);
        return 0;
    }
}

int dengineutils_stream_new_mem(const void* buffer, size_t sz, Stream* stream)
{
    memset(stream, 0, sizeof(Stream));
    stream->buffer = buffer;
    stream->size = sz;
    stream->type = DENGINEUTILS_STREAM_TYPE_MEMORY;
    return 1;
}
size_t dengineutils_stream_read(void* dest, const size_t size, const size_t count, Stream* stream)
{
    size_t ret = 0;
    if(stream->type == DENGINEUTILS_STREAM_TYPE_FILE)
    {
        ret = fread(dest, size, count, stream->fp);
        stream->eof = feof(stream->fp);
    }else if(stream->type == DENGINEUTILS_STREAM_TYPE_ANDROIDASSET)
    {
#ifdef DENGINE_ANDROID
        ret = AAsset_read(stream->asset, dest, size * count);
#else
        dengineutils_logging_log(DENGINEUTILS_STREAM_MSG_TYPE4ANDROID);
#endif
    }else if (stream->type == DENGINEUTILS_STREAM_TYPE_MEMORY)
    {
        memcpy(dest, stream->buffer + stream->pos, size * count);
        stream->pos += size * count;
        stream->eof = stream->pos >= stream->size;
        return size;
    }else
    {
        dengineutils_logging_log(DENGINEUTILS_STREAM_MSG_UNKNOWNTYPE);
        return 0;
    }
    return ret;
}
;
size_t dengineutils_stream_write(const void* src, const size_t size, const size_t count, const Stream* stream)
{
    if(stream->type == DENGINEUTILS_STREAM_TYPE_FILE)
    {
        return fwrite(src, size, count, stream->fp);
    }else if(stream->type == DENGINEUTILS_STREAM_TYPE_ANDROIDASSET || stream->type == DENGINEUTILS_STREAM_TYPE_MEMORY) 
    {
        dengineutils_logging_log(DENGINEUTILS_STREAM_MSG_RO);
        return 0;
    }else
    {
        dengineutils_logging_log(DENGINEUTILS_STREAM_MSG_UNKNOWNTYPE);
        return 0;
    }
}

off_t dengineutils_stream_seek(Stream* stream, const off_t offset, const int whence)
{
    if(stream->type == DENGINEUTILS_STREAM_TYPE_FILE)
    {
        fseeko(stream->fp, offset, whence);
        stream->pos = ftello(stream->fp);
        return stream->pos;
    }else if(stream->type == DENGINEUTILS_STREAM_TYPE_ANDROIDASSET)
    {
#ifdef DENGINE_ANDROID
        return AAsset_seek(stream->asset, offset, whence);
#else
        dengineutils_logging_log(DENGINEUTILS_STREAM_MSG_TYPE4ANDROID);
        return 0;
#endif
    }else if(stream->type == DENGINEUTILS_STREAM_TYPE_MEMORY)
    {
        if(whence == SEEK_CUR)
            stream->pos += offset;
        else if(whence == SEEK_SET)
            stream->pos = offset;
        else if(whence == SEEK_END)
            stream->pos = stream->size + offset;
        /* cant have memory stream reading random memory! */
        stream->eof = stream->pos >= stream->size;
        return stream->pos;
    }else{
        dengineutils_logging_log(DENGINEUTILS_STREAM_MSG_UNKNOWNTYPE);
        return 0;
    }
}

void dengineutils_stream_destroy(Stream* stream)
{
    if(stream->fp)
        fclose(stream->fp);
#ifdef DENGINE_ANDROID
    if(stream->asset)
        AAsset_close(stream->asset);
#endif
    free(stream->path);
}
