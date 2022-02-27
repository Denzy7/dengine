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

varying vec3 Normal, CamPos, FragPos;
varying vec2 TexCoord;

uniform Camera camera;
uniform mat4 model;

void main()
{
    gl_Position = camera.projection * camera.view * model * vec4(aPos, 1.0);

    Normal = aNormal;
    TexCoord = aTexCoord;
    CamPos = camera.position;
    FragPos = vec3(model * vec4(aPos, 1.0));
}
