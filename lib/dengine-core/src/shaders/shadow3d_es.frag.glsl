#version 320 es
precision mediump float;
in vec4 FragPos;

uniform vec3 pos;
uniform float far;

void main()
{
    float distance = length(FragPos.xyz - pos);
    distance /= far;
    gl_FragDepth = distance;
}
