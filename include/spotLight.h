//
//  spotLight.h
//  SpotLight for focused illumination (e.g. store display highlights)
//

#ifndef spotLight_h
#define spotLight_h

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "shader.h"
#include <string>
#include <cmath>

class SpotLight {
public:
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float cutOff;      // inner cone angle in degrees
    float outerCutOff;  // outer cone angle in degrees
    float k_c;  // constant attenuation
    float k_l;  // linear attenuation
    float k_q;  // quadratic attenuation
    int lightNumber;

    SpotLight(glm::vec3 pos, glm::vec3 dir,
              glm::vec3 amb, glm::vec3 diff, glm::vec3 spec,
              float cutoff, float outerCutoff,
              float constant, float linear, float quadratic, int num)
    {
        position = pos;
        direction = glm::normalize(dir);
        ambient = amb;
        diffuse = diff;
        specular = spec;
        cutOff = cutoff;
        outerCutOff = outerCutoff;
        k_c = constant;
        k_l = linear;
        k_q = quadratic;
        lightNumber = num;
    }

    void setUpSpotLight(Shader& lightingShader)
    {
        lightingShader.use();
        std::string base = "spotLights[" + std::to_string(lightNumber) + "]";

        lightingShader.setVec3(base + ".position", position);
        lightingShader.setVec3(base + ".direction", direction);
        lightingShader.setVec3(base + ".ambient", ambientOn * ambient);
        lightingShader.setVec3(base + ".diffuse", diffuseOn * diffuse);
        lightingShader.setVec3(base + ".specular", specularOn * specular);
        lightingShader.setFloat(base + ".cutOff", cos(cutOff * 3.14159265f / 180.0f));
        lightingShader.setFloat(base + ".outerCutOff", cos(outerCutOff * 3.14159265f / 180.0f));
        lightingShader.setFloat(base + ".k_c", k_c);
        lightingShader.setFloat(base + ".k_l", k_l);
        lightingShader.setFloat(base + ".k_q", k_q);
    }

    void turnOff()
    {
        ambientOn = 0.0f;
        diffuseOn = 0.0f;
        specularOn = 0.0f;
    }
    void turnOn()
    {
        ambientOn = 1.0f;
        diffuseOn = 1.0f;
        specularOn = 1.0f;
    }

private:
    float ambientOn = 1.0f;
    float diffuseOn = 1.0f;
    float specularOn = 1.0f;
};

#endif /* spotLight_h */
