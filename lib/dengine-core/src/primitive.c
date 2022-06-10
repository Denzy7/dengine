#include "dengine/primitive.h"

#include "dengine/loadgl.h" //gltypes
#include "dengine/entrygl.h" //entrygl
#include "dengine-utils/vtor.h"

#include <string.h> //memset

void dengine_primitive_setup(Primitive* primitive, const Shader* shader)
{
    VAO entry_vao;
    Buffer entry_ibo, entry_vbo;
    dengine_entrygl_buffer(GL_ARRAY_BUFFER, &entry_vbo);
    dengine_entrygl_buffer(GL_ELEMENT_ARRAY_BUFFER, &entry_ibo);

    int prof;
    glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &prof);

    if(prof == GL_CONTEXT_CORE_PROFILE_BIT)
    {
        dengine_entrygl_vao(&entry_vao);
        dengine_vao_gen(1, &primitive->vao);
        dengine_vao_bind(&primitive->vao);
    }

    dengine_buffer_gen(1, &primitive->array);
    dengine_buffer_bind(GL_ARRAY_BUFFER, &primitive->array);
    dengine_buffer_data(GL_ARRAY_BUFFER,&primitive->array);

    dengine_buffer_gen(1, &primitive->index);
    dengine_buffer_bind(GL_ELEMENT_ARRAY_BUFFER, &primitive->index);
    dengine_buffer_data(GL_ELEMENT_ARRAY_BUFFER,&primitive->index);

    dengine_vertex_attribute_indexfromshader(shader, &primitive->aPos, "aPos");
    dengine_vertex_attribute_indexfromshader(shader, &primitive->aTexCoord, "aTexCoord");
    dengine_vertex_attribute_indexfromshader(shader, &primitive->aNormal, "aNormal");
    dengine_vertex_attribute_indexfromshader(shader, &primitive->aTangent, "aTangent");
    dengine_vertex_attribute_indexfromshader(shader, &primitive->aBiTangent, "aBiTangent");

    dengine_buffer_bind(GL_ELEMENT_ARRAY_BUFFER, &entry_ibo);
    dengine_buffer_bind(GL_ARRAY_BUFFER, &entry_vbo);

    if(prof == GL_CONTEXT_CORE_PROFILE_BIT)
    {
        dengine_vao_bind(&entry_vao);
    }

    primitive->offset = NULL;
}

void dengine_primitive_attributes_enable(const Primitive* primitive, const Shader* shader)
{
    if(primitive->aPos.index != -1)
    {
        dengine_vertex_attribute_setup(&primitive->aPos);
        dengine_vertex_attribute_enable(&primitive->aPos);
    }

    if(primitive->aTexCoord.index != -1)
    {
        dengine_vertex_attribute_setup(&primitive->aTexCoord);
        dengine_vertex_attribute_enable(&primitive->aTexCoord);
    }

    if(primitive->aNormal.index != -1)
    {
        dengine_vertex_attribute_setup(&primitive->aNormal);
        dengine_vertex_attribute_enable(&primitive->aNormal);
    }

    if(primitive->aTangent.index != -1)
    {
        dengine_vertex_attribute_setup(&primitive->aTangent);
        dengine_vertex_attribute_enable(&primitive->aTangent);
    }

    if(primitive->aBiTangent.index != -1)
    {
        dengine_vertex_attribute_setup(&primitive->aBiTangent);
        dengine_vertex_attribute_enable(&primitive->aBiTangent);
    }
}

void dengine_primitive_destroy(Primitive* primitive)
{
    int prof;
    glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &prof);
    if(prof == GL_CONTEXT_CORE_PROFILE_BIT)
    {
        dengine_vao_destroy(1, &primitive->vao);
    }
    dengine_buffer_destroy(1, &primitive->array);
    dengine_buffer_destroy(1, &primitive->index);
}

void dengine_primitive_gen_quad(Primitive* primitive, const Shader* shader)
{
    memset(primitive,0,sizeof (Primitive));

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
        0, 1, 2, 2, 3, 0
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

    dengine_primitive_setup(primitive, shader);
}

void dengine_primitive_gen_plane(Primitive* primitive, const Shader* shader)
{
    memset(primitive,0,sizeof (Primitive));

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

                float s = x < 0.0f ? 1.0f : 0.0f;
                float t = z < 0.0f ? 1.0f : 0.0f;

                vtor_pushback(&plane_array, &s);
                vtor_pushback(&plane_array, &t);

                float x_n = 0.0f;
                float y_n = 1.0f;
                float z_n = 0.0f;

                vtor_pushback(&plane_array, &x_n);
                vtor_pushback(&plane_array, &y_n);
                vtor_pushback(&plane_array, &z_n);

                 float x_tan = 1.0f;
                 float y_tan = 0.0f;
                 float z_tan = 0.0f;

                 vtor_pushback(&plane_array, &x_tan);
                 vtor_pushback(&plane_array, &y_tan);
                 vtor_pushback(&plane_array, &z_tan);

                 float x_bitan = 0.0f;
                 float y_bitan = 0.0f;
                 float z_bitan = 1.0f;

                 vtor_pushback(&plane_array, &x_bitan);
                 vtor_pushback(&plane_array, &y_bitan);
                 vtor_pushback(&plane_array, &z_bitan);

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

        size_t stride = 14 * sizeof(float);

        //aPos
        primitive->aPos.size = 3;
        primitive->aPos.stride = stride;
        primitive->aPos.type = GL_FLOAT;
        primitive->aPos.ptr = NULL;

        //aTexCoord
        primitive->aTexCoord.size = 2;
        primitive->aTexCoord.stride = stride;
        primitive->aTexCoord.type = GL_FLOAT;
        primitive->aTexCoord.ptr = (void*)(3 * sizeof(float));

        //aNormal
        primitive->aNormal.size = 3;
        primitive->aNormal.stride = stride;
        primitive->aNormal.type = GL_FLOAT;
        primitive->aNormal.ptr = (void*)(5 * sizeof(float));

        //aTangent
        primitive->aTangent.size = 3;
        primitive->aTangent.stride = stride;
        primitive->aTangent.type = GL_FLOAT;
        primitive->aTangent.ptr = (void*)(8 * sizeof(float));

        //aBiTangent
        primitive->aBiTangent.size = 3;
        primitive->aBiTangent.stride = stride;
        primitive->aBiTangent.type = GL_FLOAT;
        primitive->aBiTangent.ptr = (void*)(11 * sizeof(float));

        dengine_primitive_setup(primitive, shader);

        vtor_free(&plane_array);
}

void dengine_primitive_gen_cube(Primitive* primitive, const Shader* shader)
{
    memset(primitive,0,sizeof (Primitive));

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
        uint16_t index = cube_index[i];

        float x = vertices[(3 * index)];
        float y = vertices[(3 * index) + 1];
        float z = vertices[(3 * index) + 2];

        float s;
        float t;

        if(i >= 0 && i < 12)
        {
            s = z < 0.0f ? 0.0f : 1.0f;
            t = y < 0.0f ? 0.0f : 1.0f;
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

        float x_n;
        float y_n;
        float z_n;

        float tangent[3], bitangent[3];

        if(i >= 0 && i < 6)
        {
            x_n = -1.0f;
            y_n = 0.0f;
            z_n = 0.0f;

            tangent[0] = 0.0f;
            tangent[1] = -1.0f;
            tangent[2] = 0.0f;

            bitangent[0] = 0.0f;
            bitangent[1] = 0.0f;
            bitangent[2] = -1.0f;
        }else if(i >= 6 && i < 12)
        {
            x_n = 1.0f;
            y_n = 0.0f;
            z_n = 0.0f;

            tangent[0] = 0.0f;
            tangent[1] = 1.0f;
            tangent[2] = 0.0f;

            bitangent[0] = 0.0f;
            bitangent[1] = 0.0f;
            bitangent[2] = 1.0f;
        }else if(i >= 12 && i < 18)
        {
            x_n = 0.0f;
            y_n = -1.0f;
            z_n = 0.0f;

            tangent[0] = -1.0f;
            tangent[1] = 0.0f;
            tangent[2] = 0.0f;

            bitangent[0] = 0.0f;
            bitangent[1] = 0.0f;
            bitangent[2] = -1.0f;
        }
        else if(i >= 18 && i < 24)
        {
            x_n = 0.0f;
            y_n = 1.0f;
            z_n = 0.0f;

            tangent[0] = 1.0f;
            tangent[1] = 0.0f;
            tangent[2] = 0.0f;

            bitangent[0] = 0.0f;
            bitangent[1] = 0.0f;
            bitangent[2] = 1.0f;
        }else if(i >= 24 && i < 30)
        {
            x_n = 0.0f;
            y_n = 0.0f;
            z_n = -1.0f;

            tangent[0] = -1.0f;
            tangent[1] = 0.0f;
            tangent[2] = 0.0f;

            bitangent[0] = 0.0f;
            bitangent[1] = -1.0f;
            bitangent[2] = 0.0f;
        }else if(i >= 30 && i < 36)
        {
            x_n = 0.0f;
            y_n = 0.0f;
            z_n = 1.0f;

            tangent[0] = 1.0f;
            tangent[1] = 0.0f;
            tangent[2] = 0.0f;

            bitangent[0] = 0.0f;
            bitangent[1] = 1.0f;
            bitangent[2] = 0.0f;
        }

        vtor_pushback(&cube_array, &x);
        vtor_pushback(&cube_array, &y);
        vtor_pushback(&cube_array, &z);

        vtor_pushback(&cube_array, &s);
        vtor_pushback(&cube_array, &t);

        vtor_pushback(&cube_array, &x_n);
        vtor_pushback(&cube_array, &y_n);
        vtor_pushback(&cube_array, &z_n);

        vtor_pushback(&cube_array, &tangent[0]);
        vtor_pushback(&cube_array, &tangent[1]);
        vtor_pushback(&cube_array, &tangent[2]);

        vtor_pushback(&cube_array, &bitangent[0]);
        vtor_pushback(&cube_array, &bitangent[1]);
        vtor_pushback(&cube_array, &bitangent[2]);
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

    size_t stride = 14 * sizeof(float);

    //aPos
    primitive->aPos.size = 3;
    primitive->aPos.stride = stride;
    primitive->aPos.type = GL_FLOAT;
    primitive->aPos.ptr = NULL;

    //aTexCoord
    primitive->aTexCoord.size = 2;
    primitive->aTexCoord.stride = stride;
    primitive->aTexCoord.type = GL_FLOAT;
    primitive->aTexCoord.ptr = (void*)(3 * sizeof(float));

    //aNormal
    primitive->aNormal.size = 3;
    primitive->aNormal.stride = stride;
    primitive->aNormal.type = GL_FLOAT;
    primitive->aNormal.ptr = (void*)(5 * sizeof(float));

    //aTangent
    primitive->aTangent.size = 3;
    primitive->aTangent.stride = stride;
    primitive->aTangent.type = GL_FLOAT;
    primitive->aTangent.ptr = (void*)(8 * sizeof(float));

    //aBiTangent
    primitive->aBiTangent.size = 3;
    primitive->aBiTangent.stride = stride;
    primitive->aBiTangent.type = GL_FLOAT;
    primitive->aBiTangent.ptr = (void*)(11 * sizeof(float));

    dengine_primitive_setup(primitive, shader);

    vtor_free(&cube_array);
    vtor_free(&cube_vertices);
}

void dengine_primitive_gen_grid(const uint16_t slice, Primitive* primitive, const Shader* shader)
{
    memset(primitive,0,sizeof (Primitive));

    vtor grid_vertices, grid_indices;
    vtor_create(&grid_vertices, sizeof(float));
    vtor_create(&grid_indices, sizeof(uint16_t));

    for(int i = -slice; i <= slice; i++)
    {
        for(int j = -slice; j <= slice; j++)
        {
            float x = (float)i / (float)slice;
            float y = 0.0f;
            float z = (float)j / (float)slice;

            vtor_pushback(&grid_vertices, &x);
            vtor_pushback(&grid_vertices, &y);
            vtor_pushback(&grid_vertices, &z);
        }
    }

    for(int i = 0; i < 2 * slice; i++)
    {
        for(int j = 0; j < 2 * slice; j++)
        {
            uint16_t row1 = i * ((2 * slice) + 1); //0
            uint16_t row2 = (i + 1) * ((2 * slice) + 1); //4

            uint16_t row1off = row1 + j;
            uint16_t row1off1 = row1off + 1;

            uint16_t row2off = row2 + j;
            uint16_t row2off1 = row2off + 1;

            vtor_pushback(&grid_indices, &row1off);
            vtor_pushback(&grid_indices, &row1off1);
            vtor_pushback(&grid_indices, &row1off1);
            vtor_pushback(&grid_indices, &row2off1);

            vtor_pushback(&grid_indices, &row2off1);
            vtor_pushback(&grid_indices, &row2off);
            vtor_pushback(&grid_indices, &row2off);
            vtor_pushback(&grid_indices, &row1off);
        }
    }

    primitive->draw_mode = GL_LINES;
    primitive->draw_type = GL_UNSIGNED_SHORT;

    //ARRAY
    primitive->array.data = grid_vertices.data;
    primitive->array.size = sizeof(float) * grid_vertices.count;
    primitive->array.usage = GL_STATIC_DRAW;

    //INDEX
    primitive->index.data = grid_indices.data;
    primitive->index.size = sizeof(uint16_t) * grid_indices.count;
    primitive->index.usage = GL_STATIC_DRAW;
    primitive->index_count = grid_indices.count;

    //aPos
    primitive->aPos.size = 3;
    primitive->aPos.stride = 3 * sizeof(float);
    primitive->aPos.type = GL_FLOAT;
    primitive->aPos.ptr = NULL;

    dengine_primitive_setup(primitive, shader);

    vtor_free(&grid_vertices);
    vtor_free(&grid_indices);
}

void dengine_primitive_gen_axis(Primitive* primitive, const Shader* shader)
{
    memset(primitive,0,sizeof (Primitive));
    static float axis_vertices[]=
    {
        //Lines
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
    };

    static uint16_t axis_indices[]=
    {
      0, 1, 0, 2, 0, 3,
    };

    primitive->draw_mode = GL_LINES;
    primitive->draw_type = GL_UNSIGNED_SHORT;

    //ARRAY
    primitive->array.data = axis_vertices;
    primitive->array.size = sizeof(axis_vertices);
    primitive->array.usage = GL_STATIC_DRAW;

    //INDEX
    primitive->index.data = axis_indices;
    primitive->index.size = sizeof(axis_vertices);
    primitive->index.usage = GL_STATIC_DRAW;
    primitive->index_count = sizeof(axis_vertices) / sizeof(axis_vertices[0]);

    //aPos
    primitive->aPos.size = 3;
    primitive->aPos.stride = 3 * sizeof(float);
    primitive->aPos.type = GL_FLOAT;
    primitive->aPos.ptr = NULL;

    dengine_primitive_setup(primitive, shader);
}
