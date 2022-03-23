//default general purpose debug shader
#version 100

struct Camera
{
    mat4 projection;
    mat4 view;
    mat4 uview;
    vec3 position;
};

attribute vec3 aPos;
attribute vec3 aNormal;
uniform Camera camera;
uniform mat4 model;

varying vec3 Normal;

void main()
{
    gl_Position = camera.projection * camera.view * model * vec4(aPos, 1.0);

    Normal = aNormal;
}
