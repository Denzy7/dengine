#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

int main(int argc, char** argv)
{
    const char* in = argv[argc - 2];
    const char* out = argv[argc - 1];
    int enb = 0; /*end null byte, useful for string resource
                 like shaders*/
    for(int i = 0; i < argc; i++)
    {
        if(strcmp(argv[i], "-enb") == 0)
            enb = 1;
    }

    if(argc < 3)
    {
        printf("Pass the file to convert to C array in argv[1] and destination file to argv[2]\n");
        return 1;
    }

    FILE* f_in = fopen(in, "rb");
    if(!f_in)
    {
        printf("Cannot open %s for input\n", in);
        perror(NULL);
        return 1;
    }

    FILE* f_out = fopen(out, "wb");
    if(!f_out)
    {
        printf("Cannot open %s for output\n", out);
        perror(NULL);
        return 1;
    }

    const char* varname = NULL;
    const char* varname_nopath = strrchr(in, '/');
    if(varname_nopath)
        varname = varname_nopath + 1;
    else
        varname = in;

    char* varname_sanitized;
    size_t varnameln = strlen(varname);

    if(varname[0] >= '0' && varname[0] <= '9')
    {
        varname_sanitized = calloc(varnameln + 2, 1);
        varname_sanitized[0] = '_';
        strncpy(varname_sanitized + 1, varname, varnameln);
    }else
    {
        varname_sanitized = strdup(varname);
    }

    for(size_t i = 0; i < varnameln; i++)
    {
        int goodchar = 0;
        if(
                (varname_sanitized[i] >= 'A' && varname_sanitized[i] <= 'Z') ||
                (varname_sanitized[i] >= 'a' && varname_sanitized[i] <= 'z') ||
                (varname_sanitized[i] >= '0' && varname_sanitized[i] <= '9'))
        {
            goodchar = 1;
        }

        if(!goodchar)
            varname_sanitized[i] = '_';
    }

    fseeko(f_in, 0, SEEK_END);
    off_t ln = ftello(f_in);
    size_t write = 0;
    rewind(f_in);

    //write ln
    fprintf(f_out, "unsigned int %s_ln = %u;\n\n", varname_sanitized,  (uint32_t)ln);
    fprintf(f_out, "unsigned char %s[] = {\n\n\t", varname_sanitized);

    while(ln > 0)
    {
        uint8_t byte = fgetc(f_in);

        fprintf(f_out, "0x");

        if(byte <= 0xa)
            fprintf(f_out, "0");

        fprintf(f_out, "%x", byte);
        fprintf(f_out, ", ");
        write++;

        if(write % 8 == 0 && write != 0)
            fprintf(f_out,"\n\t");

        ln--;
    }
    if(enb)
        fprintf(f_out, "\n\n\t0x0");
    fprintf(f_out, "\n\n};\n");
    fclose(f_in);
    fclose(f_out);
    free(varname_sanitized);

    return 0;
}
