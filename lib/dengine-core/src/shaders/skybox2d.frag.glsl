#version 100
precision mediump float;
varying vec3 TexCoord;

uniform sampler2D eqireqMap;

vec2 invAtan = vec2(0.1591, 0.3183);

vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{
    vec2 uv = SampleSphericalMap(normalize(TexCoord)); // make sure to normalize
    vec3 col = texture2D(eqireqMap,uv).rgb;
    gl_FragColor = vec4(col, 1.0);
}
