#include "primitive.h"

#include "loadgl.h" //gltypes

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

void dengine_primitive_gen_quad(Primitive* primitive, Shader* shader)
{
    primitive->draw_mode = GL_TRIANGLES;
    primitive->draw_type = GL_UNSIGNED_SHORT;
    //ARRAY
    primitive->array.data = quad_array;
    primitive->array.size = sizeof(quad_array);
    primitive->array.usage = GL_STATIC_DRAW;

    dengine_buffer_gen(1, &primitive->array);
    dengine_buffer_bind(GL_ARRAY_BUFFER, &primitive->array);
    dengine_buffer_data(GL_ARRAY_BUFFER,&primitive->array);

    //INDEX
    primitive->index.data = quad_index;
    primitive->index.size = sizeof(quad_index);
    primitive->index.usage = GL_STATIC_DRAW;
    primitive->index_count = sizeof(quad_index) / sizeof(quad_index[0]);

    dengine_buffer_gen(1, &primitive->index);
    dengine_buffer_bind(GL_ELEMENT_ARRAY_BUFFER, &primitive->index);
    dengine_buffer_data(GL_ELEMENT_ARRAY_BUFFER,&primitive->index);

    //aPos
    primitive->aPos.size = 3;
    primitive->aPos.stride = 5 * sizeof(float);
    primitive->aPos.type = GL_FLOAT;
    primitive->aPos.ptr = NULL;

    dengine_vertex_attribute_indexfromshader(shader, &primitive->aPos, "aPos");
    if(primitive->aPos.index != -1)
    {
        dengine_vertex_attribute_setup(&primitive->aPos);
        dengine_vertex_attribute_enable(&primitive->aPos);
    }


    //aTexCoord
    primitive->aTexCoord.size = 2;
    primitive->aTexCoord.stride = 5 * sizeof(float);
    primitive->aTexCoord.type = GL_FLOAT;
    primitive->aTexCoord.ptr = (void*)(3 * sizeof(float));
    dengine_vertex_attribute_indexfromshader(shader, &primitive->aTexCoord, "aTexCoord");

    if(primitive->aTexCoord.index != -1)
    {
        dengine_vertex_attribute_setup(&primitive->aTexCoord);
        dengine_vertex_attribute_enable(&primitive->aTexCoord);
    }



    dengine_buffer_bind(GL_ELEMENT_ARRAY_BUFFER, NULL);
    dengine_buffer_bind(GL_ARRAY_BUFFER, NULL);
}

void dengine_primitive_gen_plane(Primitive* primitive, Shader* shader)
{

}
