#include <pulse/simple.h>

#include <dengine/dengine.h>
#include <stb_vorbis.c>

//pa
pa_simple* server = NULL;
int err;

//stb_vorbis
int channels, rate, samples_total;
short* data = NULL;

extern unsigned int nocturne_chopin_ogg_ln;
extern unsigned char nocturne_chopin_ogg[];

Texture progress, progress_bg;

char now[512];

int simple_start(void* args)
{
    //pulse
    pa_sample_spec spec;


    dengineutils_logging_log("INFO::decoding audio...");

    uint8_t* mem = NULL;
    int mem_sz = 0;
    int custom = 0;
    Stream* f = NULL;

    const char* playing = "Now playing: ";

    if(args)
    {
        f = dengineutils_stream_new(args,
                                    DENGINEUTILS_STREAM_TYPE_FILE,
                                    DENGINEUTILS_STREAM_MODE_READ);
        if(f == NULL)
        {
            dengineutils_logging_log("cannot open : %s", args);
            return 1;
        }
        mem_sz = f->size;
        mem = malloc(mem_sz);
        dengineutils_stream_read(mem, 1, f->size, f);
        custom = 1;
        snprintf(now, sizeof(now), "%s %s",
                 playing,
                 (char*)args);
    }else
    {
        mem = nocturne_chopin_ogg;
        mem_sz = nocturne_chopin_ogg_ln;
        snprintf(now, sizeof(now), "%s %s",
                 playing,
                 "Nocturne - Chopin (play custom .ogg using argv[1]!)");
    }

    samples_total = stb_vorbis_decode_memory(mem, mem_sz,
                                 &channels, &rate, &data);

    if(custom)
    {
        free(mem);
        dengineutils_stream_destroy(f);
    }

    if(samples_total < 0)
    {
        dengineutils_logging_log("ERROR::Failed to decode ogg");
        return 1;
    }

    dengineutils_logging_log("INFO::done!\nch:%d, rate:%d, samples:%d", channels, rate, samples_total);

    spec.channels = channels;
    spec.format = PA_SAMPLE_S16LE;
    spec.rate = rate;

    server = pa_simple_new(NULL,
                           "nsl-pulseaudio",
                           PA_STREAM_PLAYBACK,
                           NULL,
                           "play stream",
                           &spec,
                           NULL,
                           NULL,
                           &err);

    if(server == NULL)
        return 1;

    static const float col1[3] = {1.0, 0.5, 0.3};
    dengine_texture_make_color(8, 8, col1, 3, &progress);

    static const float col2[3] = {0.3, 0.5, 1.0};
    dengine_texture_make_color(8, 8, col2, 3, &progress_bg);


    return 0;
}

int simple_update(void* args)
{
    static int samples_read = 0;
    //crude timing
    time_t t1, t2;
    t1 = samples_read;
    t2 =  samples_total / rate;

    int sample_sz = sizeof(short) * channels * rate;
    if(t1 <= t2)
    {
        //write 1 second sample
        pa_simple_write(server, data + (channels * rate * samples_read), sample_sz, &err);
        samples_read += 1;
    }else
    {
        //done!
        return 1;
    }

    //update time UI
    static char timestr[30];
    struct tm* tm;
    static const char* fmt = "%H:%M:%S";

    //update gui
    float fontsz = denginegui_get_fontsz();
    int wid;
    dengine_viewport_get(NULL, NULL, &wid, NULL);
    //HACK: half to fit in screen
    wid /= 2;

    tm = gmtime(&t1);
    strftime(timestr, sizeof(timestr), fmt, tm);
    denginegui_text(fontsz, fontsz, timestr, NULL);

    tm = gmtime(&t2);
    strftime(timestr, sizeof(timestr), fmt, tm);
    denginegui_text(wid, fontsz, timestr, NULL);

    const float progress_offset = 3;

    denginegui_panel(fontsz, fontsz * 2, wid, fontsz, &progress_bg, NULL, GLM_VEC4_BLACK);

    denginegui_panel(fontsz + progress_offset, (fontsz * 2) + progress_offset,
                     ((float)wid * ((float)t1 / (float)t2)) - (2 * progress_offset),
                     fontsz - (2 * progress_offset),
                     &progress, NULL, GLM_VEC4_BLACK);
    denginegui_text(fontsz, (fontsz * 3) + (2 * progress_offset) ,
                    now, NULL);

    return 0;
}

int simple_terminate(void* args)
{
    if(data != NULL)
        free(data);

    if(server != NULL)
        pa_simple_free(server);

    return 0;
}
