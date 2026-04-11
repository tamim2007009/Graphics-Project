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
uniform int lightingComponentMode; // 1=Ambient, 2=Directional, 3=Diffuse, 4=Combined

in vec3 GouraudAmbient;
in vec3 GouraudDiffuse;
in vec3 GouraudSpecular;

uniform float dayNightMix;

vec3 CalcPointLight(Material mat, PointLight light, vec3 N, vec3 Pos, vec3 V);
vec3 CalcPointLightDiffuseOnly(Material mat, PointLight light, vec3 N, vec3 Pos, vec3 V);

vec3 CalcDirLight(Material mat, DirectionalLight light, vec3 N, vec3 Pos, vec3 V);
vec3 CalcDirLightDiffuseOnly(Material mat, DirectionalLight light, vec3 N, vec3 Pos, vec3 V);
vec3 CalcDirLightOnly(Material mat, DirectionalLight light, vec3 N, vec3 Pos, vec3 V);

vec3 CalcSpotLight(Material mat, SpotLight light, vec3 N, vec3 Pos, vec3 V);
vec3 CalcSpotLightDiffuseOnly(Material mat, SpotLight light, vec3 N, vec3 Pos, vec3 V);

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
        // ===== GOURAUD SHADING =====
        vec3 result = vec3(0.0);
        
        if (lightingComponentMode == 1) {
            // AMBIENT ONLY
            result = mat.ambient * GouraudAmbient;
        } else if (lightingComponentMode == 2) {
            // DIRECTIONAL ONLY
            result = mat.diffuse * GouraudDiffuse;
        } else if (lightingComponentMode == 3) {
            // DIFFUSE ONLY
            result = mat.diffuse * GouraudDiffuse;
        } else {
            // COMBINED (mode 4)
            result = mat.ambient * GouraudAmbient + mat.diffuse * GouraudDiffuse;
        }
        
        FragColor = vec4(result, 1.0);
        return;
    }

    // ===== PHONG SHADING (Fragment-based) =====
    vec3 dayAmbient = globalAmbient;
    vec3 nightAmbient = globalAmbient * 0.15;
    vec3 effectiveAmbient = mix(dayAmbient, nightAmbient, dayNightMix);
    
    vec3 result = vec3(0.0);
    
    if (lightingComponentMode == 1) {
        // ===== MODE 1: AMBIENT ONLY =====
        result = mat.ambient * effectiveAmbient;
    }
    else if (lightingComponentMode == 2) {
        // ===== MODE 2: DIRECTIONAL ONLY =====
        result += CalcDirLightOnly(mat, dirLight, N, FragPos, V);
        result += CalcDirLightOnly(mat, dirLight2, N, FragPos, V);
    }
    else if (lightingComponentMode == 3) {
        // ===== MODE 3: DIFFUSE ONLY =====
        for (int i = 0; i < numPointLights; i++) {
            result += CalcPointLightDiffuseOnly(mat, pointLights[i], N, FragPos, V);
        }
        result += CalcDirLightDiffuseOnly(mat, dirLight, N, FragPos, V);
        result += CalcDirLightDiffuseOnly(mat, dirLight2, N, FragPos, V);
        for (int i = 0; i < numSpotLights; i++) {
            result += CalcSpotLightDiffuseOnly(mat, spotLights[i], N, FragPos, V);
        }
    }
    else {
        // ===== MODE 4: COMBINED =====
        result = mat.ambient * effectiveAmbient;
        
        for (int i = 0; i < numPointLights; i++) {
            result += CalcPointLight(mat, pointLights[i], N, FragPos, V);
        }
        result += CalcDirLight(mat, dirLight, N, FragPos, V);
        result += CalcDirLight(mat, dirLight2, N, FragPos, V);
        for (int i = 0; i < numSpotLights; i++) {
            result += CalcSpotLight(mat, spotLights[i], N, FragPos, V);
        }
    }

    FragColor = vec4(result, 1.0);
}

// ===== FULL LIGHTING FUNCTIONS =====
vec3 CalcPointLight(Material mat, PointLight light, vec3 N, vec3 Pos, vec3 V)
{
    vec3 L = normalize(light.position - Pos);
    float d = length(light.position - Pos);
    float attenuation = 1.0 / (light.k_c + light.k_l * d + light.k_q * d * d);

    vec3 ambient  = mat.ambient * light.ambient * attenuation;
    vec3 diffuse  = mat.diffuse * max(dot(N, L), 0.0) * light.diffuse * attenuation;
    
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
        float epsilon = 0.05;
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

    float theta = dot(normalize(-L), normalize(light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    vec3 H = normalize(L + V);
    vec3 ambient  = mat.ambient * light.ambient * attenuation;
    vec3 diffuse  = mat.diffuse * max(dot(N, L), 0.0) * light.diffuse * attenuation * intensity;
    vec3 specular = mat.specular * pow(max(dot(N, H), 0.0), mat.shininess) * light.specular * attenuation * intensity;

    return (ambient + diffuse + specular);
}

// ===== DIFFUSE-ONLY FUNCTIONS =====
vec3 CalcPointLightDiffuseOnly(Material mat, PointLight light, vec3 N, vec3 Pos, vec3 V)
{
    vec3 L = normalize(light.position - Pos);
    float d = length(light.position - Pos);
    float attenuation = 1.0 / (light.k_c + light.k_l * d + light.k_q * d * d);
    
    vec3 diffuse = mat.diffuse * max(dot(N, L), 0.0) * light.diffuse * attenuation;
    return diffuse;
}

vec3 CalcDirLightDiffuseOnly(Material mat, DirectionalLight light, vec3 N, vec3 Pos, vec3 V)
{
    vec3 fragToLight = normalize(light.position - Pos);
    vec3 lightDir = normalize(light.direction);
    float theta = dot(-fragToLight, lightDir);

    float d = length(light.position - Pos);
    float attenuation = 1.0 / (1.0 + 0.045 * d + 0.0075 * d * d);

    vec3 diffuse = vec3(0.0);

    if (theta > light.cutoff) {
        float epsilon = 0.05;
        float intensity = clamp((theta - light.cutoff) / epsilon, 0.0, 1.0);

        vec3 L = fragToLight;
        diffuse = mat.diffuse * max(dot(N, L), 0.0) * light.diffuse * attenuation * intensity;
    }

    return diffuse;
}

vec3 CalcSpotLightDiffuseOnly(Material mat, SpotLight light, vec3 N, vec3 Pos, vec3 V)
{
    vec3 L = normalize(light.position - Pos);
    float d = length(light.position - Pos);
    float attenuation = 1.0 / (light.k_c + light.k_l * d + light.k_q * d * d);

    float theta = dot(normalize(-L), normalize(light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    vec3 diffuse = mat.diffuse * max(dot(N, L), 0.0) * light.diffuse * attenuation * intensity;

    return diffuse;
}

// ===== DIRECTIONAL-ONLY FUNCTION =====
vec3 CalcDirLightOnly(Material mat, DirectionalLight light, vec3 N, vec3 Pos, vec3 V)
{
    vec3 L = normalize(light.position - Pos);
    vec3 lightDir = normalize(light.direction);
    float theta = dot(-L, lightDir);

    vec3 result = vec3(0.0);

    // No distance attenuation for directional lights (sun-like)
    // Relaxed cutoff for broader illumination (cos(30°) ? 0.866)
    if (theta > 0.8) {
        // Smooth falloff at cone edges for natural lighting
        float intensity = smoothstep(0.8, 1.0, theta);
        
        // Calculate diffuse with full intensity
        float diffuseStrength = max(dot(N, L), 0.0);
        result = mat.diffuse * diffuseStrength * light.diffuse * intensity;
        
        // Add a base contribution for ambient fill in directional mode
        result += mat.ambient * light.ambient * 0.5;
    } else {
        // Outside cone, very slight illumination
        result = mat.ambient * light.ambient * 0.2;
    }

    return result;
}
