#version 150
attribute vec3 aPos;

uniform mat4 model;
void main()
{
    gl_Position = model * vec4(aPos, 1.0);
}