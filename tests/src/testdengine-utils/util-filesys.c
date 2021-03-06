#include <dengine-utils/filesys.h>
#include <dengine-utils/logging.h>

#include <dengine-utils/rng.h>

int main(int argc, char** argv)
{
    if(argc < 2)
        dengineutils_logging_log("ERROR::Specify a file to argv[1]");

    dengineutils_filesys_init();

    File2Mem test;
    test.file = argv[1];
    if(dengineutils_filesys_file2mem_load(&test))
    {
        dengineutils_logging_log("INFO::loaded %s. sz : %zu bytes", test.file, test.size);

        dengineutils_filesys_file2mem_free(&test);
    }

    dengineutils_logging_log("files : %s", dengineutils_filesys_get_filesdir());

    dengineutils_logging_log("cache : %s", dengineutils_filesys_get_cachedir());

    dengineutils_filesys_get_assetsdir();

    dengineutils_filesys_terminate();
}
