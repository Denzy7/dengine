#version 100

struct Camera
{
    mat4 projection;
    mat4 view;
    mat4 uview;
    vec3 position;
};

attribute vec3 aPos;
attribute vec2 aTexCoord;
attribute vec3 aNormal;
attribute vec3 aTangent;
attribute vec3 aBiTangent;

varying vec3 Normal, CamPos, FragPos;
varying vec2 TexCoord;
varying mat3 TBN;

uniform Camera camera;
uniform mat4 model;

void main()
{
    gl_Position = camera.projection * camera.view * model * vec4(aPos, 1.0);

    Normal = aNormal;
    TexCoord = aTexCoord;
    CamPos = camera.position;
    FragPos = vec3(model * vec4(aPos, 1.0));

    vec3 T = normalize(vec3(model * vec4(aTangent, 0)));
    vec3 B = normalize(vec3(model * vec4(aBiTangent, 0)));
    vec3 N = normalize(vec3(model * vec4(aNormal, 0)));

    TBN = mat3(T, B, N);
}//dont remove this comment