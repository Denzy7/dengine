#include "dengine-model/model.h"
#include "dengine-utils/vtor.h"
#include "dengine/loadgl.h"

#include "dengine-utils/logging.h"
#include "dengine-utils/timer.h"
#include "dengine-utils/debug.h"

#include <stdio.h> //fopen
#include <stddef.h> //size_t
#include <stdlib.h> //malloc, free
#include <string.h> //memset

#include <cglm/cglm.h> //normalize, sub

Primitive* _denginemodel_load_obj(const void* mem, const size_t sz, size_t* meshes, const Shader* shader);

Primitive* denginemodel_load_file(DengineModelFormat format, const char* file, size_t* meshes , const Shader* shader)
{
    DENGINE_DEBUG_ENTER;

    FILE* f_model = fopen(file, "rb");
    if(!f_model)
    {
        dengineutils_logging_log("ERROR::Model %s not found!", file);
        return NULL;
    }

    fseek(f_model, 0, SEEK_END);
    size_t sz = ftell(f_model);

    void* mem = malloc(sz + 1);
    memset(mem, 0, sz + 1);

    rewind(f_model);
    fread(mem, 1, sz, f_model);
    fclose(f_model);

    Primitive* load = NULL;

    if (format == DENGINE_MODEL_FORMAT_OBJ) {
        load = _denginemodel_load_obj(mem, sz, meshes, shader);
    }

    free(mem);

    return load;
}

Primitive* denginemodel_load_mem(DengineModelFormat format, const void* mem, const size_t sz, size_t* meshes, const Shader* shader)
{
    DENGINE_DEBUG_ENTER;

    if(format == DENGINE_MODEL_FORMAT_OBJ)
        return _denginemodel_load_obj(mem, sz, meshes, shader);
    else
        return NULL;
}

Primitive* _denginemodel_load_obj(const void* mem, const size_t sz, size_t* meshes, const Shader* shader)
{
    const char* filestr = mem;

    const char* vertex_delim = "v ";
    const char* texcoord_delim = "vt ";
    const char* norm_delim = "vn ";
    const char* face_delim = "f ";
    const char* meshes_delim = "o ";

    vtor verts, tcoords, norms,offsets;
    vtor array_buf, index_buf;
    memset(&verts, 0, sizeof(vtor));
    memset(&tcoords, 0, sizeof(vtor));
    memset(&norms, 0, sizeof(vtor));
    memset(&offsets, 0, sizeof(vtor));
    memset(&array_buf, 0, sizeof(vtor));
    memset(&index_buf, 0, sizeof(vtor));

    size_t n_vertices = 0, n_texcoord = 0, n_normals = 0, n_faces = 0, n_meshes = 0;

    vtor_create(&verts, sizeof(float));
    vtor_create(&tcoords, sizeof(float));
    vtor_create(&norms, sizeof(float));

    vtor_create(&array_buf, sizeof(float));
    vtor_create(&index_buf, sizeof(size_t));
    vtor_create(&offsets, sizeof(size_t));

    const size_t ln_buf_sz = 1024;
    const size_t cmp_buf_sz = 10;
    const size_t name_buf_sz = 256;

    //length and compare buffer
    char* ln_buf = malloc(ln_buf_sz);
    char* cmp_buf = malloc(cmp_buf_sz);
    char* name_buf = malloc(name_buf_sz);

    size_t read = 0;
    size_t last = 0;
    size_t last_off = 0, last_idx = 0;

    vec3 edge1, edge2, tangent, bitangent;
    vec2 deltaUV1, deltaUV2;

    dengineutils_timer_update();
    double start = dengineutils_timer_get_current();

    while (read < sz)
    {
        if (filestr[read] == '\n')
        {
            memset(ln_buf, 0, ln_buf_sz);
            strncpy(ln_buf, filestr + last + 1, read - last);
            last = read;

            float x, y, z;

            //MESHES
            strncpy(cmp_buf, ln_buf, strlen(meshes_delim));
            if(!strcmp(cmp_buf, meshes_delim))
            {
                sscanf(ln_buf,"o %s", name_buf);
                //printf("mesh : %s\n", name_buf);

                if(n_meshes>0)
                {

                    last_off = index_buf.count - last_idx;
                    last_idx = index_buf.count;

                    //dengineutils_logging_log("a : %zu", last_off);

                    vtor_pushback(&offsets,&last_off);
                }

                n_meshes++;
            }
            memset(cmp_buf, 0, cmp_buf_sz);

            //VERTICES
            strncpy(cmp_buf, ln_buf, strlen(vertex_delim));
            if(!strcmp(cmp_buf, vertex_delim))
            {
                sscanf(ln_buf, "v %f %f %f", &x, &y, &z);
                //printf("vert : %f %f %f\n", x, y, z);

                vtor_pushback(&verts, &x);
                vtor_pushback(&verts, &y);
                vtor_pushback(&verts, &z);

                n_vertices++;
            }
            memset(cmp_buf, 0, cmp_buf_sz);

            //TEX_COORDS
            strncpy(cmp_buf, ln_buf, strlen(texcoord_delim));
            if(!strcmp(cmp_buf, texcoord_delim))
            {
                sscanf(ln_buf, "vt %f %f", &x, &y);
                //printf("tcoord : %f %f\n", x, y);

                vtor_pushback(&tcoords, &x);
                vtor_pushback(&tcoords, &y);

                n_texcoord++;
            }
            memset(cmp_buf, 0, cmp_buf_sz);

            //NORMALS
            strncpy(cmp_buf, ln_buf, strlen(norm_delim));
            if(!strcmp(cmp_buf, norm_delim))
            {
                sscanf(ln_buf, "vn %f %f %f", &x, &y, &z);
                //printf("normal : %f %f %f\n", x, y, z);

                vtor_pushback(&norms, &x);
                vtor_pushback(&norms, &y);
                vtor_pushback(&norms, &z);

                n_normals++;
            }
            memset(cmp_buf, 0, cmp_buf_sz);

            //FACES. Assumes they are triangulated
            uint32_t x1, x2, x3, y1, y2, y3, z1, z2, z3;
            strncpy(cmp_buf, ln_buf, strlen(face_delim));
            if(!strcmp(cmp_buf, face_delim))
            {
                sscanf(ln_buf, "f %u/%u/%u %u/%u/%u %u/%u/%u",
                       &x1, &y1, &z1, &x2, &y2, &z2, &x3, &y3, &z3);

//                printf("face : %hu/%hu/%hu %hu/%hu/%hu %hu/%hu/%hu\n",
//                       x1, y1, z1, x2, y2, z2, x3, y3, z3);
                n_faces++;

                size_t point1 = x1 - 1;
                size_t point2 = x2 - 1;
                size_t point3 = x3 - 1;

                float* vert_ptr = verts.data;
                float* tcoord_ptr = tcoords.data;
                float* normal_ptr = norms.data;

                edge1[0] = vert_ptr[ (3 * (x2 - 1))] - vert_ptr[ (3 * (x1 - 1))];
                edge1[1] = vert_ptr[ (3 * (x2 - 1)) + 1] - vert_ptr[ (3 * (x1 - 1)) + 1];
                edge1[2] = vert_ptr[ (3 * (x2 - 1)) + 2] - vert_ptr[ (3 * (x1 - 1)) + 2];

                edge2[0] = vert_ptr[ (3 * (x3 - 1))] - vert_ptr[ (3 * (x1 - 1))];
                edge2[1] = vert_ptr[ (3 * (x3 - 1)) + 1] - vert_ptr[ (3 * (x1 - 1)) + 1];
                edge2[2] = vert_ptr[ (3 * (x3 - 1)) + 2] - vert_ptr[ (3 * (x1 - 1)) + 2];

                deltaUV1[0] = tcoord_ptr[ (2 * (y2 - 1))] - tcoord_ptr[ (2 * (y1 - 1))];
                deltaUV1[1] = tcoord_ptr[ (2 * (y2 - 1)) + 1] - tcoord_ptr[ (2 * (y1 - 1)) + 1];

                deltaUV2[0] = tcoord_ptr[ (2 * (y3 - 1))] - tcoord_ptr[ (2 * (y1 - 1))];
                deltaUV2[1] = tcoord_ptr[ (2 * (y3 - 1)) + 1] - tcoord_ptr[ (2 * (y1 - 1)) + 1];

                float f = 1.0f / (deltaUV1[0] * deltaUV2[1] - deltaUV2[0] * deltaUV1[1]);

                tangent[0] = f * (deltaUV2[1] * edge1[0] - deltaUV1[1] * edge2[0]);
                tangent[1] = f * (deltaUV2[1] * edge1[1] - deltaUV1[1] * edge2[1]);
                tangent[2] = f * (deltaUV2[1] * edge1[2] - deltaUV1[1] * edge2[2]);

                glm_vec3_normalize(tangent);

                bitangent[0] = f * (-deltaUV2[0] * edge1[0] + deltaUV1[0] * edge2[0]);
                bitangent[1] = f * (-deltaUV2[0] * edge1[1] + deltaUV1[0] * edge2[1]);
                bitangent[2] = f * (-deltaUV2[0] * edge1[2] + deltaUV1[0] * edge2[2]);

                glm_vec3_normalize(bitangent);

                //point 1
                for(size_t i = 0; i < 3; i++)
                    vtor_pushback(&array_buf, &vert_ptr[ (3 * (x1 - 1)) + i]);

                for(size_t i = 0; i < 2; i++)
                    vtor_pushback(&array_buf, &tcoord_ptr[ (2 * (y1 - 1)) + i]);

                for(size_t i = 0; i < 3; i++)
                    vtor_pushback(&array_buf, &normal_ptr[ (3 * (z1 - 1)) + i]);

                for(size_t i = 0; i < 3; i++)
                    vtor_pushback(&array_buf, &tangent[i]);

                for(size_t i = 0; i < 3; i++)
                    vtor_pushback(&array_buf, &bitangent[i]);

                //point 2
                for(size_t i = 0; i < 3; i++)
                    vtor_pushback(&array_buf, &vert_ptr[ (3 * (x2 - 1)) + i]);

                for(size_t i = 0; i < 2; i++)
                    vtor_pushback(&array_buf, &tcoord_ptr[ (2 * (y2 - 1)) + i]);

                for(size_t i = 0; i < 3; i++)
                    vtor_pushback(&array_buf, &normal_ptr[ (3 * (z2 - 1)) + i]);

                for(size_t i = 0; i < 3; i++)
                    vtor_pushback(&array_buf, &tangent[i]);

                for(size_t i = 0; i < 3; i++)
                    vtor_pushback(&array_buf, &bitangent[i]);

                //point 3
                for(size_t i = 0; i < 3; i++)
                    vtor_pushback(&array_buf, &vert_ptr[ (3 * (x3 - 1)) + i]);

                for(size_t i = 0; i < 2; i++)
                    vtor_pushback(&array_buf, &tcoord_ptr[ (2 * (y3 - 1)) + i]);

                for(size_t i = 0; i < 3; i++)
                    vtor_pushback(&array_buf, &normal_ptr[ (3 * (z3 - 1)) + i]);

                for(size_t i = 0; i < 3; i++)
                    vtor_pushback(&array_buf, &tangent[i]);

                for(size_t i = 0; i < 3; i++)
                    vtor_pushback(&array_buf, &bitangent[i]);

                vtor_pushback(&index_buf, &point1);
                vtor_pushback(&index_buf, &point2);
                vtor_pushback(&index_buf, &point3);
            }
            memset(cmp_buf, 0, cmp_buf_sz);

        }
        read++;
    }
    for(size_t i = 0; i < index_buf.count; i++)
    {
        size_t* ptr = index_buf.data;
        //normalize the buffer
        ptr[i] = i;
    }

//    for(size_t i = 0; i < array_buf.count; i++)
//    {
//        if(i % 8 == 0 && i > 0)
//            printf("\n=====\n");

//        if(i % 24 == 0)
//            printf("\npolygon %zu\n=====\n", i);

//        float* ptr = array_buf.data;
//        printf("%6.01ff,", ptr[i]);
//    }
//    printf("\n=====\n");

    dengineutils_timer_update();
    double done = dengineutils_timer_get_current();

    dengineutils_logging_log("INFO::meshes : %zu, vertices : %zu, tex_coords : %zu, normals : %zu, faces : %zu, indices : %zu\nTIME : %.3fms", n_meshes, n_vertices, n_texcoord, n_normals, n_faces, index_buf.count, done - start);

    if(offsets.count==0)
    {
        vtor_pushback(&offsets,&index_buf.count);
    }else {
        last_off = index_buf.count - last_idx;
        vtor_pushback(&offsets,&last_off);
        //dengineutils_logging_log("l:%zu",last_off);
    }

    Primitive* ret = NULL;
    if(n_meshes > 0)
    {
        ret = calloc(n_meshes,sizeof (Primitive));
        if(meshes)
            *meshes=n_meshes;
                            //v + te+ n + ta+ b
        const size_t stride = 3 + 2 + 3 + 3 + 3;
        const size_t elemsize = sizeof(float);
        size_t last_offset = 0;
        for(size_t i = 0; i < n_meshes; i++)
        {
            float* abptr = array_buf.data;
            size_t* offptr = offsets.data;

            size_t n_idx=offptr[i];

            size_t n_abufelems = n_idx * stride;

            float* p_abuf=calloc(n_abufelems, elemsize);
            uint16_t* p_ibuf=calloc(n_idx,sizeof(uint16_t));

            //dengineutils_logging_log("cpy:%zu",last_n_idx);

            memcpy(p_abuf,abptr + last_offset, elemsize * n_abufelems);
            size_t offset = stride * n_idx;
            last_offset+=offset;

//            for(size_t j= 0; j < n_abufelems; j++)
//                printf("%6.01ff ", p_abuf[j]);

//            printf("\n");

            for (size_t j=0; j < n_idx; j++)
            {
                if (j < UINT16_MAX - 3)  {
                    p_ibuf[j]=j;
                }else
                {
                    dengineutils_logging_log("WARNING::Index buffer is just about full. Consider reducing the number of vertices in this mesh!");
                    break;
                }
            }

//            for(size_t i = 0; i < n_idx * 8; i++)
//            {
//                if(i % 8 == 0 && i > 0)
//                    printf("\n=====\n");

//                if(i % 24 == 0)
//                    printf("\npolygon %zu\n=====\n", i);

//                float* ptr = p_abuf;
//                printf("%6.01ff,", ptr[i]);
//            }
//            printf("\n=====\n");

            ret[i].draw_mode = GL_TRIANGLES;
            ret[i].draw_type = GL_UNSIGNED_SHORT;

            //ARRAY
            ret[i].array.data = p_abuf;
            ret[i].array.size = n_abufelems * elemsize;
            ret[i].array.usage = GL_STATIC_DRAW;

            //INDEX
            ret[i].index.data = p_ibuf;
            ret[i].index.size = sizeof(uint16_t) * n_idx;
            ret[i].index.usage = GL_STATIC_DRAW;
            ret[i].index_count = n_idx;

            //aPos
            ret[i].aPos.size = 3;
            ret[i].aPos.stride = stride * elemsize;
            ret[i].aPos.type = GL_FLOAT;
            ret[i].aPos.ptr = NULL;

            //aTexCoord
            ret[i].aTexCoord.size = 2;
            ret[i].aTexCoord.stride = stride * elemsize;
            ret[i].aTexCoord.type = GL_FLOAT;
            ret[i].aTexCoord.ptr = (void*)(3 * sizeof(float));

            //aNormal
            ret[i].aNormal.size = 3;
            ret[i].aNormal.stride = stride * elemsize;
            ret[i].aNormal.type = GL_FLOAT;
            ret[i].aNormal.ptr = (void*)(5 * sizeof(float));

            //aTangent
            ret[i].aTangent.size = 3;
            ret[i].aTangent.stride = stride * elemsize;
            ret[i].aTangent.type = GL_FLOAT;
            ret[i].aTangent.ptr = (void*)(8 * sizeof(float));

            //aBiTangent
            ret[i].aBiTangent.size = 3;
            ret[i].aBiTangent.stride = stride * elemsize;
            ret[i].aBiTangent.type = GL_FLOAT;
            ret[i].aBiTangent.ptr = (void*)(11 * sizeof(float));

            dengine_primitive_setup(&ret[i], shader);

            free(p_abuf);
            free(p_ibuf);
        }
    }

    free(ln_buf);
    free(cmp_buf);
    free(name_buf);

    vtor_free(&verts);
    vtor_free(&tcoords);
    vtor_free(&norms);

    vtor_free(&array_buf);
    vtor_free(&index_buf);

    vtor_free(&offsets);

    return ret;
}
