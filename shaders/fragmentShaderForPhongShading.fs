#version 330 core
out vec4 FragColor;
in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float k_c;
    float k_l;
    float k_q;
};

struct DirectionalLight {
    vec3 position;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float cutoff;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float cutOff;
    float outerCutOff;
    float k_c;
    float k_l;
    float k_q;
};

#define MAX_POINT_LIGHTS 20
#define MAX_SPOT_LIGHTS 8

uniform vec3 viewPos;
uniform int numPointLights;
uniform int numSpotLights;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];
uniform DirectionalLight dirLight;
uniform DirectionalLight dirLight2;
uniform Material material;
uniform vec3 globalAmbient;

uniform bool useTexture;
uniform bool blendWithColor;
uniform sampler2D texture1;
uniform float texTiling;

uniform bool noLightCondition;
uniform int shadingMode; // 0 = Phong, 1 = Gouraud

in vec3 GouraudAmbient;
in vec3 GouraudDiffuse;
in vec3 GouraudSpecular;

// Day/Night interpolation factor (0.0 = full day, 1.0 = full night)
uniform float dayNightMix;

vec3 CalcPointLight(Material mat, PointLight light, vec3 N, vec3 Pos, vec3 V);
vec3 CalcDirLight(Material mat, DirectionalLight light, vec3 N, vec3 Pos, vec3 V);
vec3 CalcSpotLight(Material mat, SpotLight light, vec3 N, vec3 Pos, vec3 V);

void main()
{
    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);

    Material mat = material;
    if(useTexture) {
        float tl = (texTiling > 0.0) ? texTiling : 1.0;
        vec3 tc = texture(texture1, TexCoords * tl).rgb;
        if(blendWithColor) {
            mat.ambient = tc * material.ambient;
            mat.diffuse = tc * material.diffuse;
        } else {
            mat.ambient = tc * 0.4;
            mat.diffuse = tc;
        }
    }

    if (noLightCondition) {
        FragColor = vec4(mat.diffuse, 1.0);
        return;
    }

    if (shadingMode == 1) {
        // Gouraud shading: lighting was calculated in Vertex Shader
        vec3 result = mat.ambient * GouraudAmbient + mat.diffuse * GouraudDiffuse + mat.specular * GouraudSpecular;
        FragColor = vec4(result, 1.0);
        return;
    }

    // Day/Night ambient blending
    vec3 dayAmbient = globalAmbient;
    vec3 nightAmbient = globalAmbient * 0.15;
    vec3 effectiveAmbient = mix(dayAmbient, nightAmbient, dayNightMix);
    vec3 result = mat.ambient * effectiveAmbient;

    // Point lights
    for (int i = 0; i < numPointLights; i++)
    {
        result += CalcPointLight(mat, pointLights[i], N, FragPos, V);
    }

    // Directional lights (sun/lampposts)
    result += CalcDirLight(mat, dirLight, N, FragPos, V);
    result += CalcDirLight(mat, dirLight2, N, FragPos, V);

    // Spot lights (store highlights)
    for (int i = 0; i < numSpotLights; i++)
    {
        result += CalcSpotLight(mat, spotLights[i], N, FragPos, V);
    }

    FragColor = vec4(result, 1.0);
}

vec3 CalcPointLight(Material mat, PointLight light, vec3 N, vec3 Pos, vec3 V)
{
    vec3 L = normalize(light.position - Pos);
    vec3 R = reflect(-L, N);

    float d = length(light.position - Pos);
    float attenuation = 1.0 / (light.k_c + light.k_l * d + light.k_q * d * d);

    vec3 ambient  = mat.ambient * light.ambient * attenuation;
    vec3 diffuse  = mat.diffuse * max(dot(N, L), 0.0) * light.diffuse * attenuation;
    
    // Blinn-Phong for realistic specular
    vec3 H = normalize(L + V);
    vec3 specular = mat.specular * pow(max(dot(N, H), 0.0), mat.shininess) * light.specular * attenuation;

    return (ambient + diffuse + specular);
}

vec3 CalcDirLight(Material mat, DirectionalLight light, vec3 N, vec3 Pos, vec3 V)
{
    vec3 fragToLight = normalize(light.position - Pos);
    vec3 lightDir = normalize(light.direction);
    float theta = dot(-fragToLight, lightDir);

    float d = length(light.position - Pos);
    float attenuation = 1.0 / (1.0 + 0.045 * d + 0.0075 * d * d);

    vec3 ambient = mat.ambient * light.ambient * attenuation;
    vec3 diffuse  = vec3(0.0);
    vec3 specular = vec3(0.0);

    if (theta > light.cutoff)
    {
        // Smooth edge for spotlight effect
        float epsilon = 0.05; // soft edge
        float intensity = clamp((theta - light.cutoff) / epsilon, 0.0, 1.0);

        vec3 L = fragToLight;
        vec3 H = normalize(L + V);
        diffuse  = mat.diffuse * max(dot(N, L), 0.0) * light.diffuse * attenuation * intensity;
        specular = mat.specular * pow(max(dot(N, H), 0.0), mat.shininess) * light.specular * attenuation * intensity;
    }

    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(Material mat, SpotLight light, vec3 N, vec3 Pos, vec3 V)
{
    vec3 L = normalize(light.position - Pos);
    float d = length(light.position - Pos);
    float attenuation = 1.0 / (light.k_c + light.k_l * d + light.k_q * d * d);

    // Spotlight cone with soft edge
    float theta = dot(normalize(-L), normalize(light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    vec3 H = normalize(L + V);
    vec3 ambient  = mat.ambient * light.ambient * attenuation;
    vec3 diffuse  = mat.diffuse * max(dot(N, L), 0.0) * light.diffuse * attenuation * intensity;
    vec3 specular = mat.specular * pow(max(dot(N, H), 0.0), mat.shininess) * light.specular * attenuation * intensity;

    return (ambient + diffuse + specular);
}
