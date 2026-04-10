#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

out vec3 GouraudAmbient;
out vec3 GouraudDiffuse;
out vec3 GouraudSpecular;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform int shadingMode;

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

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

uniform vec3 viewPos;
uniform int numPointLights;
uniform int numSpotLights;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];
uniform DirectionalLight dirLight;
uniform DirectionalLight dirLight2;
uniform vec3 globalAmbient;
uniform float dayNightMix;
uniform Material material;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoords;

    GouraudAmbient = vec3(0.0);
    GouraudDiffuse = vec3(0.0);
    GouraudSpecular = vec3(0.0);

    if (shadingMode == 1) {
        vec3 N = normalize(Normal);
        vec3 V = normalize(viewPos - FragPos);
        vec3 Pos = FragPos;
        float shininess = material.shininess > 0.0 ? material.shininess : 32.0;

        vec3 dayAmbient = globalAmbient;
        vec3 nightAmbient = globalAmbient * 0.15;
        GouraudAmbient = mix(dayAmbient, nightAmbient, dayNightMix);

        // Point lights
        for (int i = 0; i < numPointLights; i++) {
            vec3 L = normalize(pointLights[i].position - Pos);
            vec3 H = normalize(L + V);
            float d = length(pointLights[i].position - Pos);
            float attenuation = 1.0 / (pointLights[i].k_c + pointLights[i].k_l * d + pointLights[i].k_q * d * d);
            
            GouraudAmbient  += pointLights[i].ambient * attenuation;
            GouraudDiffuse  += max(dot(N, L), 0.0) * pointLights[i].diffuse * attenuation;
            GouraudSpecular += pow(max(dot(N, H), 0.0), shininess) * pointLights[i].specular * attenuation;
        }

        // Dir light 1
        vec3 fragToLight = normalize(dirLight.position - Pos);
        vec3 lightDir = normalize(dirLight.direction);
        float theta = dot(-fragToLight, lightDir);
        GouraudAmbient += dirLight.ambient;
        if (theta > dirLight.cutoff) {
            vec3 L = fragToLight;
            vec3 H = normalize(L + V);
            GouraudDiffuse  += max(dot(N, L), 0.0) * dirLight.diffuse;
            GouraudSpecular += pow(max(dot(N, H), 0.0), shininess) * dirLight.specular;
        }

        // Dir light 2
        fragToLight = normalize(dirLight2.position - Pos);
        lightDir = normalize(dirLight2.direction);
        theta = dot(-fragToLight, lightDir);
        GouraudAmbient += dirLight2.ambient;
        if (theta > dirLight2.cutoff) {
            vec3 L = fragToLight;
            vec3 H = normalize(L + V);
            GouraudDiffuse  += max(dot(N, L), 0.0) * dirLight2.diffuse;
            GouraudSpecular += pow(max(dot(N, H), 0.0), shininess) * dirLight2.specular;
        }

        // Spot lights
        for (int i = 0; i < numSpotLights; i++) {
            vec3 L = normalize(spotLights[i].position - Pos);
            float d = length(spotLights[i].position - Pos);
            float attenuation = 1.0 / (spotLights[i].k_c + spotLights[i].k_l * d + spotLights[i].k_q * d * d);

            float theta = dot(normalize(-L), normalize(spotLights[i].direction));
            float epsilon = spotLights[i].cutOff - spotLights[i].outerCutOff;
            float intensity = clamp((theta - spotLights[i].outerCutOff) / epsilon, 0.0, 1.0);

            vec3 H = normalize(L + V);
            GouraudAmbient  += spotLights[i].ambient * attenuation;
            GouraudDiffuse  += max(dot(N, L), 0.0) * spotLights[i].diffuse * attenuation * intensity;
            GouraudSpecular += pow(max(dot(N, H), 0.0), shininess) * spotLights[i].specular * attenuation * intensity;
        }
    }
}
