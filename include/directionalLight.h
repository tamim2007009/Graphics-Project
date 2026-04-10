//
//  directionalLight.h
//  Directional Light with Position and Cutoff (Spotlight from Lamp)
//

#ifndef directionalLight_h
#define directionalLight_h

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "shader.h"
#include <cmath>
#include <string>

class DirectionalLight
{
public:
    glm::vec3 position;  // lamp position
    glm::vec3 direction; // aim direction (e.g. downward)
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float cutoffAngle; // half-angle in degrees

    DirectionalLight(glm::vec3 pos, glm::vec3 dir, glm::vec3 amb, glm::vec3 diff, glm::vec3 spec, float cutoff = 45.0f)
    {
        position = pos;
        direction = dir;
        ambient = amb;
        diffuse = diff;
        specular = spec;
        cutoffAngle = cutoff;
    }

    void setUpDirectionalLight(Shader &lightingShader, const std::string &name = "dirLight")
    {
        lightingShader.use();
        lightingShader.setVec3(name + ".position", position);
        lightingShader.setVec3(name + ".direction", direction);
        lightingShader.setVec3(name + ".ambient", ambientOn * ambient);
        lightingShader.setVec3(name + ".diffuse", diffuseOn * diffuse);
        lightingShader.setVec3(name + ".specular", specularOn * specular);
        lightingShader.setFloat(name + ".cutoff", cos(cutoffAngle * 3.14159265f / 180.0f));
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

#endif /* directionalLight_h */
