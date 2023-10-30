#version 100
precision mediump float;
varying vec3 Normal;
void main()
{
    gl_FragColor = vec4(Normal, 1.0);
}