#include <dengine-math/math3d.h>
#include <stdio.h>
int main()
{
    //Make mat4x4 a
    Matrix mata;
    mata.column = 4;
    mata.row = 4;
    denginemath_make_mat(&mata);

    //Set identity
    denginemath_set_identity(&mata);
    printf("mat(a):\n");
    denginemath_print_matrix(&mata);

    //Some rng
    denginemath_set_random(&mata, 5);
    printf("mat(a):\n");
    denginemath_print_matrix(&mata);

    //Make mat4x4 b
    Matrix matb;
    matb.column = 4;
    matb.row = 4;
    denginemath_make_mat(&matb);
    denginemath_set_random(&matb, 6);
    printf("mat(b):\n");
    denginemath_print_matrix(&matb);

    //Make mat4x4 dest
    Matrix dest;
    dest.column = 4;
    dest.row = 4;
    denginemath_make_mat(&dest);

    denginemath_multipy_mat(&dest, &mata, &matb);
    printf("mat(dest):\n");
    denginemath_print_matrix(&dest);

    //free when done
    denginemath_free_mat(&mata);
    denginemath_free_mat(&matb);
    denginemath_free_mat(&dest);
}
