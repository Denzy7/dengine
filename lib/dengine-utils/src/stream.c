#include "dengine-utils/stream.h"

#include <stdlib.h> //malloc
#include <string.h> //strdup

#include "dengine-utils/logging.h"

#define DENGINEUTILS_STREAM_MSG_TYPE4ANDROID "ERROR::StreamType=ANDROIDASSET is only for Android"
#define DENGINEUTILS_STREAM_MSG_OPENFAILED "ERROR::Failed to open"
#define DENGINEUTILS_STREAM_MSG_UNKNOWNTYPE "ERROR::Unknown StreamType"
#define DENGINEUTILS_STREAM_MSG_UNKNOWNMODE "ERROR::Unknown StreamMode"
#define DENGINEUTILS_STREAM_MSG_NOWRITEANDROID "ERROR::StreamType=ANDROIDASSET is not writable"

Stream* dengineutils_stream_new(const char* path, StreamType type, StreamMode mode)
{
    Stream* stream;
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
            return NULL;
        }

        if(!file)
        {
            dengineutils_logging_log(DENGINEUTILS_STREAM_MSG_OPENFAILED " [ %s ]", path);
            return NULL;
        }

        stream = calloc(1, sizeof(struct _Stream));
        stream->path = strdup(path);
        stream->fp = file;
        stream->type = type;
        stream->mode = mode;
        fseeko(stream->fp, 0, SEEK_END);
        stream->size = ftello(stream->fp);
        rewind(stream->fp);
        return stream;
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
            return NULL;
        }

        AAssetManager* mgr = dengineutils_android_get_assetmgr();
        AAsset* asset = AAssetManager_open(mgr, path, _mode);

        if(!asset)
        {
            dengineutils_logging_log(DENGINEUTILS_STREAM_MSG_OPENFAILED " [ %s ]", path);
            return NULL;
        }

        stream = calloc(1, sizeof(struct _Stream));
        stream->path = strdup(path);
        stream->asset = asset;
        stream->type = type;
        stream->mode = _mode;
        stream->size = AAsset_getLength(asset);
        return stream;
#else
        dengineutils_logging_log(DENGINEUTILS_STREAM_MSG_TYPE4ANDROID);
        return NULL;
#endif
    }else
    {
        dengineutils_logging_log(DENGINEUTILS_STREAM_MSG_UNKNOWNTYPE);
        return NULL;
    }
}

size_t dengineutils_stream_read(void* dest, const size_t size, const size_t count, const Stream* stream)
{
    if(stream->type == DENGINEUTILS_STREAM_TYPE_FILE)
    {
        return fread(dest, size, count, stream->fp);
    }else if(stream->type == DENGINEUTILS_STREAM_TYPE_ANDROIDASSET)
    {
#ifdef DENGINE_ANDROID
        return AAsset_read(stream->asset, dest, count);
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

size_t dengineutils_stream_write(const void* src, const size_t size, const size_t count, const Stream* stream)
{
    if(stream->type == DENGINEUTILS_STREAM_TYPE_FILE)
    {
        return fwrite(src, size, count, stream->fp);
    }else if(stream->type == DENGINEUTILS_STREAM_TYPE_ANDROIDASSET)
    {
        dengineutils_logging_log(DENGINEUTILS_STREAM_MSG_NOWRITEANDROID);
        return 0;
    } else
    {
        dengineutils_logging_log(DENGINEUTILS_STREAM_MSG_UNKNOWNTYPE);
        return 0;
    }
}

off_t dengineutils_stream_seek(const off_t offset, const int whence, const Stream* stream)
{
    if(stream->type == DENGINEUTILS_STREAM_TYPE_FILE)
    {
        fseeko(stream->fp, offset, whence);
        return ftello(stream->fp);
    }else if(stream->type == DENGINEUTILS_STREAM_TYPE_ANDROIDASSET)
    {
#ifdef DENGINE_ANDROID
        return AAsset_seek(stream->asset, offset, whence);
#else
        dengineutils_logging_log(DENGINEUTILS_STREAM_MSG_TYPE4ANDROID);
        return 0;
#endif
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
    free(stream);
}
