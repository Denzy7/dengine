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
uniform Camera camera;
uniform mat4 model;

void main()
{
    gl_Position = camera.projection * camera.view * model * vec4(aPos, 1.0);
}
