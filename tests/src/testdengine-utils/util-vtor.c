#include <stdlib.h>

#include <dengine-utils/vtor.h>
#include <dengine-utils/logging.h>
#include <dengine-utils/rng.h>

typedef struct
{
    float x, y;
}vtor2_data;

typedef struct
{
    float* x, *y;
    float* nullptr;
}vtor3_data;

int main(int argc, char *argv[])
{
    dengineutils_rng_set_seedwithtime();

    //a simple vector that stores some singular type e.g. floats
    vtor* vtor1 = malloc(sizeof (vtor));
    vtor_create(vtor1, sizeof (float));
    for (int i = 0; i < 3; i++) {
        float randomflt = (float) dengineutils_rng_int(100) / (float)10;
        dengineutils_logging_log("pushback vtor1 %f", randomflt);
        vtor_pushback(vtor1, &randomflt);
    }
    float* data1 = vtor1->data;
    for (int i = 0; i < vtor1->count; i++) {
        dengineutils_logging_log("readback vtor1 %f", data1[i]);
    }

    //a vector that stores a struct
    vtor* vtor2 = malloc(sizeof (vtor));
    vtor_create(vtor2, sizeof (vtor2_data));
    for (int i = 0; i < 3; i++) {
        float randomfltx = (float) dengineutils_rng_int(100) / (float)10;
        float randomflty = (float) dengineutils_rng_int(1000) / (float)100;
        vtor2_data randomdata = {randomfltx, randomflty};
        dengineutils_logging_log("pushback vtor2 x:%f, y:%f", randomdata.x, randomdata.y);
        vtor_pushback(vtor2, &randomdata);
    }
    vtor2_data* data2 = vtor2->data;
    for (int i = 0; i < 3; i++) {
        dengineutils_logging_log("readback vtor2 x:%f, y:%f", data2[i].x, data2[i].y);
    }

    //a complex vector that stores pointers and some values
    vtor* vtor3 = malloc(sizeof (vtor));
    vtor_create(vtor3, sizeof (vtor3_data));

    for (int i = 0; i < 3; i++) {
        vtor3_data randomdata;

        randomdata.x = calloc(4, sizeof (float));
        for (int j = 0; j < 4; j++) {
            float randomflt = (float) dengineutils_rng_int(100) / (float)10;
            randomdata.x[j] = randomflt;
            dengineutils_logging_log("pushback vtor3 %d at %d, random = %f",i,j, randomflt);
        }

        randomdata.y = calloc(8, sizeof (float));
        for (int j = 0; j < 8; j++) {
            float randomflt = (float) dengineutils_rng_int(1000) / (float)100;
            randomdata.y[j] = randomflt;
            dengineutils_logging_log("pushback vtor3 %d at %d, random = %f",i,j, randomflt);
        }
        randomdata.nullptr = NULL;
        vtor_pushback(vtor3, &randomdata);
    }

    vtor3_data* data3 = vtor3->data;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            float getrandom = data3[i].x[j];
            dengineutils_logging_log("readback vtor3 %d at %d, random = %f",i,j, getrandom);
        }

        for (int j = 0; j < 8; j++) {
            float getrandom = data3[i].y[j];
            dengineutils_logging_log("readback vtor3 %d at %d, random = %f",i,j, getrandom);
        }
    }

    //Free those pointers to avoid a leak!
    for (int i = 0; i < 3; i++) {
        float* x = data3[i].x;
        float* y = data3[i].y;
        free(x);
        free(y);
        float* nullptr = data3[i].nullptr;
        if (!nullptr) {
            dengineutils_logging_log("nullptr. nothing to see here!");
        }
    }

    vtor_free(vtor1);
    free(vtor1);

    vtor_free(vtor2);
    free(vtor2);

    vtor_free(vtor3);
    free(vtor3);
    return 0;
}
