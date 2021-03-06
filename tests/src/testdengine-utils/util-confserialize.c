#include <dengine-utils/confserialize.h>
#include <dengine-utils/logging.h>
int main(int argc, char** argv)
{
    if(argc < 3)
    {
        dengineutils_logging_log("ERROR::argv[1] = *.conf file, argv[2] = value to find");
    }
    Conf* conf = NULL;

    if(argv[1])
    {
       conf = dengineutils_confserialize_new(argv[1], '=');

       if(dengineutils_confserialize_load(conf, 1))
       {
           unsigned int kc = dengineutils_confserialize_get_keycount(conf);
           dengineutils_logging_log("INFO::keys : %u", kc);
           char* asset = NULL;

           if(argv[2])
               asset = dengineutils_confserialize_get_value(argv[2], conf);

           if(asset)
               dengineutils_logging_log("INFO::%s -> %s", argv[2], asset);
           else
               dengineutils_logging_log("ERROR::%s key not found!", argv[2]);

           dengineutils_confserialize_free(conf);
       }
    }

    //Create a conf
    conf = dengineutils_confserialize_new("test.ini", '=');

    if(conf)
    {
        //put a block
        dengineutils_confserialize_put_block("this is a block", conf);
        //put some strings
        dengineutils_confserialize_put("hello", "world", conf);
        dengineutils_confserialize_put("lorem", "ipsum", conf);
        //put newline
        dengineutils_confserialize_put_newline(conf);
        dengineutils_confserialize_put("some_int", "2", conf);
        dengineutils_confserialize_put("some_float", "2.3", conf);

        dengineutils_confserialize_put_comment("this comment wont load", conf);
        //mess with utf-8 a lil
        dengineutils_confserialize_put("ТУРБО", "ПУШКА", conf);

        //write
        size_t write = dengineutils_confserialize_write(conf);
        dengineutils_logging_log("INFO::write %zu bytes to test.ini", write);

        dengineutils_confserialize_free(conf);
    }
}
