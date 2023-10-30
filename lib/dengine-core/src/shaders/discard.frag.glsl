#version 100
precision mediump float;
varying vec2 TexCoord;
uniform sampler2D tex;
uniform float threshold;
uniform vec4 col;
void main()
{
    if(threshold < 0.1)
        discard;

    gl_FragColor = texture2D(tex, TexCoord) * col;
}
