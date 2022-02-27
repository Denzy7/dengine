#version 100
attribute vec3 aPos;

uniform mat4 projview;
uniform mat4 model;
void main()
{
    gl_Position = projview * model * vec4(aPos, 1.0);
}
