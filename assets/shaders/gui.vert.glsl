#version 100
attribute vec2 aPos;
attribute vec2 aTexCoord;
varying vec2 TexCoord;
uniform mat4 projection;
void main()
{
    gl_Position = projection * vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}

