#version 100
precision mediump float;
varying vec2 TexCoord;
uniform sampler2D tex;
uniform vec4 col;
uniform int istext;
void main()
{
    vec4 a = vec4(0.0);
    if(istext == 1)
       a = vec4(1.0, 1.0, 1.0, texture2D(tex, TexCoord).r) * col;
    else
       a = vec4( texture2D(tex, TexCoord).rgb + col.rgb, col.a);
    gl_FragColor = a;
}