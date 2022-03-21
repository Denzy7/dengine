#version 100
#define MAX_POINT_LIGHT 4
#define MAX_SPOT_LIGHT 4

precision mediump float;

struct LightOp
{
    int enable;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float strength;
};

struct ShadowOp
{
    int enable;

    int shadow_map_size;

    int pcf, pcf_samples;

    float max_bias, min_bias;

    float far_shadow, near_shadow;
};

struct DirLight
{
    vec3 position;
    mat4 shadow_projview;

    LightOp light;
    ShadowOp shadow;
};

struct PointLight
{
    vec3 position;

    LightOp light;
    ShadowOp shadow;

    float constant, linear, quadratic;
};

struct SpotLight
{
    vec3 direction;
    PointLight pointLight;
    float innerCutOff, outerCutOff;
};

varying vec3 Normal, CamPos, FragPos;
varying vec2 TexCoord;

uniform DirLight dLight;
uniform PointLight pLights[MAX_POINT_LIGHT];
uniform SpotLight sLights[MAX_SPOT_LIGHT];

uniform sampler2D dLightShadow;
uniform samplerCube pLightsShadow0;
uniform samplerCube sLightsShadow0;

uniform sampler2D diffuseTex;
uniform sampler2D specularTex;

vec3 texDiffuseCol = vec3(0.0);
vec3 texSpecularCol = vec3(0.0);

float specular_power = 64.0;

vec3 nNormal = vec3(0.0);
vec3 camDir = vec3(0.0);
float shadow_bias = 0.0;

vec3 ambient = vec3(0.0), diffuse = vec3(0.0), specular = vec3(0.0);

vec3 dirLightAdd();
vec3 pointLightAdd(PointLight pL, samplerCube cube);
vec3 spotLightAdd(SpotLight sL, samplerCube cube);

float shadowCalc2D(sampler2D twod, mat4 projview, int pcf, int pcf_samples, int shadow_map_size);
float shadowCalcCube(samplerCube cube, vec3 pos, float shadowfar);

void main()
{
    nNormal = normalize(Normal);
    camDir = normalize(CamPos - FragPos);

    texDiffuseCol = texture2D(diffuseTex, TexCoord).rgb;
    texSpecularCol = texture2D(specularTex, TexCoord).rgb;

    vec3 FragColor = vec3(0.0);
    FragColor+=dirLightAdd();

    for(int i = 0; i < 1; i++)
    {
        vec3 pLightInfl = vec3(0.0);
        vec3 sLightInfl = vec3(0.0);

        pLightInfl = pointLightAdd(pLights[i], pLightsShadow0);
        sLightInfl = spotLightAdd(sLights[i], sLightsShadow0);

        if(length(pLightInfl) > 0.0)
            FragColor+=pLightInfl;

        if(length(sLightInfl) > 0.0)
            FragColor+=sLightInfl;
    }

    gl_FragColor = vec4(FragColor, 1.0);
}

vec3 dirLightAdd()
{
    vec3 dir = normalize(-dLight.position);
    float diff = max(dot(nNormal, -dir), 0.0);

    vec3 reflectDir = reflect(dir, nNormal);
    float spec = pow(max(dot(camDir, reflectDir), 0.0), specular_power);

    if(length(texDiffuseCol) == 0.0)
    {
        ambient = dLight.light.ambient;
        diffuse = dLight.light.diffuse * diff * dLight.light.strength;
    }else
    {
        ambient = dLight.light.ambient * texDiffuseCol;
        diffuse = dLight.light.diffuse * diff * texDiffuseCol * dLight.light.strength;
    }

    specular= dLight.light.specular * spec * texSpecularCol * dLight.light.strength;

    float shadow = 0.0;

    if(dLight.shadow.enable == 1)
    {
        shadow_bias = max(dLight.shadow.max_bias * (1.0 - dot(nNormal, dir)), 0.005);
        shadow = shadowCalc2D(dLightShadow,
                                 dLight.shadow_projview,
                                 dLight.shadow.pcf,
                                 dLight.shadow.pcf_samples,
                                 dLight.shadow.shadow_map_size);
    }

    return ambient + (1.0 - shadow) * (diffuse + specular);
}

vec3 pointLightAdd(PointLight pL, samplerCube cube)
{
    vec3 dir = normalize(pL.position - FragPos);
    float diff = max(dot(nNormal, dir), 0.0);

    vec3 reflectDir=reflect(-dir, nNormal);
    float spec = pow(max(dot(camDir, reflectDir), 0.0), specular_power);

    if(length(texDiffuseCol) == 0.0)
    {
        ambient = pL.light.ambient;
        diffuse = pL.light.diffuse * diff * pL.light.strength;
    }else
    {
        ambient = pL.light.ambient * texDiffuseCol;
        diffuse = pL.light.diffuse * diff * texDiffuseCol * pL.light.strength;
    }

    specular = pL.light.specular * spec * texSpecularCol * pL.light.strength;

    float shadow = 0.0;
    shadow_bias = max(0.1 * (1.0 - dot(nNormal, dir)), 0.005);
    shadow = shadowCalcCube(cube, pL.position, pL.shadow.far_shadow);

    float distance = length(pL.position - FragPos);
    float atten = 1.0 / (pL.constant + pL.linear * distance + pL.quadratic * (distance * distance));

    return atten*(ambient + (1.0 - shadow) * (diffuse+specular));
}

vec3 spotLightAdd(SpotLight sL, samplerCube cube)
{
    vec3 dir = normalize(sL.pointLight.position - FragPos);
    float diff = max(dot(nNormal, dir), 0.0);

    vec3 reflectDir=reflect(-dir, nNormal);
    float spec = pow(max(dot(camDir, reflectDir), 0.0), specular_power);

    if(length(texDiffuseCol) == 0.0)
    {
        ambient = sL.pointLight.light.ambient;
        diffuse = sL.pointLight.light.diffuse * diff * sL.pointLight.light.strength;
    }else
    {
        ambient = sL.pointLight.light.ambient * texDiffuseCol;
        diffuse = sL.pointLight.light.diffuse * diff * texDiffuseCol * sL.pointLight.light.strength;
    }

    specular = sL.pointLight.light.specular * spec * texSpecularCol * sL.pointLight.light.strength;

    float theta = dot(dir, normalize(-sL.direction));
    float epsilon = sL.innerCutOff - sL.outerCutOff;
    float intensity = clamp( (theta - sL.outerCutOff) / epsilon, 0.0, 1.0);
    diffuse*=intensity;

    float shadow = 0.0;
    shadow_bias = max(0.1 * (1.0 - dot(nNormal, dir)), 0.005);
    shadow = shadowCalcCube(cube, sL.pointLight.position, sL.pointLight.shadow.far_shadow);

    float distance = length(sL.pointLight.position - FragPos);
    float atten = 1.0 / (sL.pointLight.constant + sL.pointLight.linear * distance + sL.pointLight.quadratic * (distance * distance));
    return atten*(ambient + (1.0 - shadow) * (diffuse+specular));
}

float shadowCalc2D(sampler2D twod, mat4 projview, int pcf, int pcf_samples, int shadow_map_size)
{
    float shadow = 0.0;
    vec4 fragPosLightSpc = projview * vec4(FragPos, 1.0);
    vec3 coord = fragPosLightSpc.xyz / fragPosLightSpc.w;
    coord = coord * 0.5 + 0.5;
    float current = coord.z;

    if(pcf == 1)
    {
        vec2 texelsz = 1.0 / vec2(shadow_map_size, shadow_map_size);
        for(int x = -1; x <= 1; ++x)
        {
            for(int y = -1; y <= 1; ++y)
            {
                float pcf_val = texture2D(twod, coord.xy + vec2(x, y) * texelsz).r;

                shadow += current - shadow_bias > pcf_val ? 1.0 : 0.0;
            }
        }
        shadow /= float(pcf_samples);
    }else
    {
        float close = texture2D(twod, coord.xy).r;
        shadow = current - shadow_bias > close ? 1.0 : 0.0;
    }

    if(coord.z > 1.0)
       shadow = 0.0;
    return shadow;
}

float shadowCalcCube(samplerCube cube, vec3 pos, float shadowfar)
{
    float shadow = 0.0;
    vec3 distance = vec3(0.0);
    distance = FragPos - pos;

    if(length(textureCube(cube, distance).rgb) == 0.0)
       return 0.0;

    float current = length(distance);
    float close = textureCube(cube, distance).r;
    //return to depth
    close*=shadowfar;
    shadow = current - shadow_bias > close ? 1.0 : 0.0;
    return shadow;
}
