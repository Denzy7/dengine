#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

int main(int argc, char** argv)
{
    const char* in = argv[1];
    const char* out = argv[2];
    if(!in)
    {
        printf("Pass the file to convert to C array in argv[1]\n");
        return 1;
    }

    if(!out)
    {
        printf("Pass the destination name to argv[2]\n");
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

    char* varname_sanitized = strdup(varname);
    size_t varnameln = strlen(varname);

    static const char badchars[]  =
    {
      '.', '-',
    };
    static const char goodchar = '_';

    int badchars_cnt = sizeof(badchars) / sizeof(badchars[0]);

    for(size_t i = 0; i < varnameln; i++)
    {
        for(int j = 0; j < badchars_cnt; j++)
        {
            if(varname_sanitized[i] == badchars[j])
               varname_sanitized[i] = goodchar;
        }
    }

    fseeko(f_in, 0, SEEK_END);
    off_t ln = ftello(f_in);
    rewind(f_in);

    //write ln
    fprintf(f_out, "unsigned int %s_ln = %u;\n", varname_sanitized,  (uint32_t)ln);
    fprintf(f_out, "unsigned char %s[] = {\n", varname_sanitized);

    while(ln > 0)
    {
        uint8_t byte = fgetc(f_in);
        fprintf(f_out, "0x%x", byte);
        if(ln != 1)
            fprintf(f_out, ",");
        ln--;
    }
    fprintf(f_out, "};\n");
    fclose(f_in);
    fclose(f_out);
    free(varname_sanitized);

    return 0;
}
