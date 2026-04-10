//
//  pointLight.h
//  Extended for multiple indexed point lights
//
//  Created by Nazirul Hasan on 22/9/23.
//  Modified for Parking Garage Project.
//

#ifndef pointLight_h
#define pointLight_h

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "shader.h"
#include <string>

class PointLight {
public:
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float k_c;
    float k_l;
    float k_q;
    int lightNumber;

    PointLight(float posX, float posY, float posZ,
        float ambR, float ambG, float ambB,
        float diffR, float diffG, float diffB,
        float specR, float specG, float specB,
        float constant, float linear, float quadratic, int num)
    {
        position = glm::vec3(posX, posY, posZ);
        ambient = glm::vec3(ambR, ambG, ambB);
        diffuse = glm::vec3(diffR, diffG, diffB);
        specular = glm::vec3(specR, specG, specB);
        k_c = constant;
        k_l = linear;
        k_q = quadratic;
        lightNumber = num;
    }

    void setUpPointLight(Shader& lightingShader)
    {
        lightingShader.use();

        std::string base = "pointLights[" + std::to_string(lightNumber) + "]";

        lightingShader.setVec3(base + ".position", position);
        lightingShader.setVec3(base + ".ambient", ambientOn * ambient);
        lightingShader.setVec3(base + ".diffuse", diffuseOn * diffuse);
        lightingShader.setVec3(base + ".specular", specularOn * specular);
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
    void turnAmbientOn() { ambientOn = 1.0f; }
    void turnAmbientOff() { ambientOn = 0.0f; }
    void turnDiffuseOn() { diffuseOn = 1.0f; }
    void turnDiffuseOff() { diffuseOn = 0.0f; }
    void turnSpecularOn() { specularOn = 1.0f; }
    void turnSpecularOff() { specularOn = 0.0f; }

private:
    float ambientOn = 1.0f;
    float diffuseOn = 1.0f;
    float specularOn = 1.0f;
};

#endif /* pointLight_h */
