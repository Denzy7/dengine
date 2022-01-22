#include "primitive.h"

#include "loadgl.h" //gltypes

#define VTOR_IMPLEMENTATION
#include "vtor.h"

void _dengine_primitive_setup(Primitive* primitive, Shader* shader)
{
    dengine_buffer_gen(1, &primitive->array);
    dengine_buffer_bind(GL_ARRAY_BUFFER, &primitive->array);
    dengine_buffer_data(GL_ARRAY_BUFFER,&primitive->array);

    dengine_buffer_gen(1, &primitive->index);
    dengine_buffer_bind(GL_ELEMENT_ARRAY_BUFFER, &primitive->index);
    dengine_buffer_data(GL_ELEMENT_ARRAY_BUFFER,&primitive->index);

    dengine_vertex_attribute_indexfromshader(shader, &primitive->aPos, "aPos");
    if(primitive->aPos.index != -1)
    {
        dengine_vertex_attribute_setup(&primitive->aPos);
        dengine_vertex_attribute_enable(&primitive->aPos);
    }

    dengine_vertex_attribute_indexfromshader(shader, &primitive->aTexCoord, "aTexCoord");
    if(primitive->aTexCoord.index != -1)
    {
        dengine_vertex_attribute_setup(&primitive->aTexCoord);
        dengine_vertex_attribute_enable(&primitive->aTexCoord);
    }


    dengine_buffer_bind(GL_ELEMENT_ARRAY_BUFFER, NULL);
    dengine_buffer_bind(GL_ARRAY_BUFFER, NULL);
}


void dengine_primitive_gen_quad(Primitive* primitive, Shader* shader)
{
    static float quad_array[]=
    {
        //Vert              //TexCoord
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };

    static uint16_t quad_index[]=
    {
        0, 1, 2, 2, 0, 3
    };

    primitive->draw_mode = GL_TRIANGLES;
    primitive->draw_type = GL_UNSIGNED_SHORT;
    //ARRAY
    primitive->array.data = quad_array;
    primitive->array.size = sizeof(quad_array);
    primitive->array.usage = GL_STATIC_DRAW;

    //INDEX
    primitive->index.data = quad_index;
    primitive->index.size = sizeof(quad_index);
    primitive->index.usage = GL_STATIC_DRAW;
    primitive->index_count = sizeof(quad_index) / sizeof(quad_index[0]);

    //aPos
    primitive->aPos.size = 3;
    primitive->aPos.stride = 5 * sizeof(float);
    primitive->aPos.type = GL_FLOAT;
    primitive->aPos.ptr = NULL;

    //aTexCoord
    primitive->aTexCoord.size = 2;
    primitive->aTexCoord.stride = 5 * sizeof(float);
    primitive->aTexCoord.type = GL_FLOAT;
    primitive->aTexCoord.ptr = (void*)(3 * sizeof(float));

    _dengine_primitive_setup(primitive, shader);
}

void dengine_primitive_gen_plane(Primitive* primitive, Shader* shader)
{
    vtor plane_array;
    vtor_create(&plane_array, sizeof(float));

    for(int i = -1; i <= 1; i += 2)
        {
            for(int j = -1; j <= 1; j += 2)
            {
                float x = (float)i;
                float y = 0;
                float z = (float)j;
                vtor_pushback(&plane_array, &x);
                vtor_pushback(&plane_array, &y);
                vtor_pushback(&plane_array, &z);

                float s = x < 0.0f ? 0.0f : 1.0f;
                float t = z < 0.0f ? 0.0f : 1.0f;

                vtor_pushback(&plane_array, &s);
                vtor_pushback(&plane_array, &t);

                // float x_n = 0.0f;
                // float y_n = 1.0f;
                // float z_n = 0.0f;

                // float x_tan = 0.0f;
                // float y_tan = 0.0f;
                // float z_tan = 0.0f;

                // float x_bitan = 0.0f;
                // float y_bitan = 0.0f;
                // float z_bitan = 0.0f;

//                printf(
//                "%f %f %f" //aPos
//                //" %f %f"//aTexCoord
//                //" %f %f %f"//aNormal
//                //" %f %f %f"//aTangent
//                //" %f %f %f"//aBiTangent
//                "\n",
//                x, y, z
//                // s, t,
//                // x_n, y_n, z_n,
//                // x_tan, y_tan, z_tan,
//                // x_bitan, y_bitan, z_bitan
//                );
            }
        }

        static uint16_t plane_index[] =
        {
          0, 1, 2, 2, 1, 3
        };

        primitive->draw_mode = GL_TRIANGLES;
        primitive->draw_type = GL_UNSIGNED_SHORT;

        //ARRAY
        primitive->array.data = plane_array.data;
        primitive->array.size = sizeof(float) * plane_array.count;
        primitive->array.usage = GL_STATIC_DRAW;

        //INDEX
        primitive->index.data = plane_index;
        primitive->index.size = sizeof(plane_index);
        primitive->index.usage = GL_STATIC_DRAW;
        primitive->index_count = sizeof(plane_index) / sizeof(plane_index[0]);


        //aPos
        primitive->aPos.size = 3;
        primitive->aPos.stride = 5 * sizeof(float);
        primitive->aPos.type = GL_FLOAT;
        primitive->aPos.ptr = NULL;

        //aTexCoord
        primitive->aTexCoord.size = 2;
        primitive->aTexCoord.stride = 5 * sizeof(float);
        primitive->aTexCoord.type = GL_FLOAT;
        primitive->aTexCoord.ptr = (void*)(3 * sizeof(float));

        _dengine_primitive_setup(primitive, shader);

        vtor_free(&plane_array);
}

void dengine_primitive_gen_cube(Primitive* primitive, Shader* shader)
{
    vtor cube_array;
    vtor_create(&cube_array, sizeof(float));

    vtor cube_vertices;
    vtor_create(&cube_vertices, sizeof(float));
    for(int i = -1; i <= 1; i += 2)
    {
        for(int j = -1; j <= 1; j += 2)
        {
            for(int k = -1; k <= 1; k += 2)
            {
                float x = (float)i;
                float y = (float)j;
                float z = (float)k;

                vtor_pushback(&cube_vertices, &x);
                vtor_pushback(&cube_vertices, &y);
                vtor_pushback(&cube_vertices, &z);
            }
        }
    }

    static uint16_t cube_index[]=
    {
        0, 1, 2, 2, 1, 3,//-x
        7, 5, 6, 6, 5, 4,//+x
        0, 4, 5, 5, 1, 0,//-y
        2, 3, 7, 7, 6, 2,//+y
        0, 2, 6, 6, 4, 0,//-z
        1, 5, 7, 7, 3, 1,//+z
    };

    int index_sz = sizeof(cube_index) / (sizeof(cube_index[0]));

    //Normalize index buffer. but first place its values in array buffer
    for(int i = 0; i < index_sz; i++)
    {
        float* vertices = cube_vertices.data;
        int index = cube_index[i];

        float x = vertices[(3 * index)];
        float y = vertices[(3 * index) + 1];
        float z = vertices[(3 * index) + 2];

        float s;
        float t;

        if(i >= 0 && i < 12)
        {
            s = y < 0.0f ? 0.0f : 1.0f;
            t = z < 0.0f ? 0.0f : 1.0f;
        }
        if(i >= 12 && i < 24)
        {
            s = x < 0.0f ? 0.0f : 1.0f;
            t = z < 0.0f ? 0.0f : 1.0f;
        }
        if(i >= 24 && i < 36)
        {
            s = x < 0.0f ? 0.0f : 1.0f;
            t = y < 0.0f ? 0.0f : 1.0f;
        }

        vtor_pushback(&cube_array, &x);
        vtor_pushback(&cube_array, &y);
        vtor_pushback(&cube_array, &z);

        vtor_pushback(&cube_array, &s);
        vtor_pushback(&cube_array, &t);

        //now normalize
        cube_index[i] = i;
    }

    primitive->draw_mode = GL_TRIANGLES;
    primitive->draw_type = GL_UNSIGNED_SHORT;

    //ARRAY
    primitive->array.data = cube_array.data;
    primitive->array.size = sizeof(float) * cube_array.count;
    primitive->array.usage = GL_STATIC_DRAW;

    //INDEX
    primitive->index.data = cube_index;
    primitive->index.size = sizeof(cube_index);
    primitive->index.usage = GL_STATIC_DRAW;
    primitive->index_count = sizeof(cube_index) / sizeof(cube_index[0]);

    //aPos
    primitive->aPos.size = 3;
    primitive->aPos.stride = 5 * sizeof(float);
    primitive->aPos.type = GL_FLOAT;
    primitive->aPos.ptr = NULL;

    //aTexCoord
    primitive->aTexCoord.size = 2;
    primitive->aTexCoord.stride = 5 * sizeof(float);
    primitive->aTexCoord.type = GL_FLOAT;
    primitive->aTexCoord.ptr = (void*)(3 * sizeof(float));

    _dengine_primitive_setup(primitive, shader);

    vtor_free(&cube_array);
    vtor_free(&cube_vertices);
}
