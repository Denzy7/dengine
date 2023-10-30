#version 100
precision mediump float;
varying vec2 TexCoord;
uniform sampler2D tex;
uniform vec4 col;
void main()
{
    gl_FragColor = vec4(texture2D(tex, TexCoord).rgb + col.rgb, col.a);
}
