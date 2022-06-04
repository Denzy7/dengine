#version 100
attribute vec3 aPos;

varying vec3 TexCoord;

struct Camera
{
    mat4 projection;
    mat4 view;
    mat4 uview;
    vec3 position;
};

uniform Camera camera;

void main()
{
    TexCoord = aPos;
    vec4 pos = camera.projection * camera.uview * vec4(aPos,1.0);
    gl_Position = pos.xyww;
}//dont remove this comment