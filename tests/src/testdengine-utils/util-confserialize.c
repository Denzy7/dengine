#include <dengine-utils/confserialize.h>
#include <dengine-utils/logging.h>
int main(int argc, char** argv)
{
    if(argc < 3)
    {
        dengineutils_logging_log("ERROR::argv[1] = *.conf file, argv[2] = value to find");
    }
    Conf conf;
    conf.file = argv[1];
    conf.separator = '=';

    if(dengineutils_confserialize_load(&conf, 1))
    {
        dengineutils_logging_log("INFO::keys : %zu", conf.keys_count);
        char* asset = NULL;

        if(argv[2])
            asset = dengineutils_confserialize_get(argv[2], &conf);

        if(asset)
            dengineutils_logging_log("INFO::%s -> %s", argv[2], asset);
        else
            dengineutils_logging_log("ERROR::%s key not found!", argv[2]);

        dengineutils_confserialize_free(&conf);
    }

    //Create a conf
    conf.file = "test.conf";
    if(dengineutils_confserialize_new(&conf))
    {
        //put some strings
        dengineutils_confserialize_put("hello", "world", &conf);
        dengineutils_confserialize_put("lorem", "ipsum", &conf);
        dengineutils_confserialize_put("some_int", "2", &conf);
        dengineutils_confserialize_put("some_float", "2.3", &conf);
        dengineutils_confserialize_put("#", "this comment wont load", &conf);

        //mess with utf-8 a lil
        dengineutils_confserialize_put("ТУРБО", "ПУШКА", &conf);

        //write
        size_t write = dengineutils_confserialize_write(&conf);
        dengineutils_logging_log("INFO::write %zu bytes to %s", write, conf.file);

        dengineutils_confserialize_free(&conf);
    }
}
