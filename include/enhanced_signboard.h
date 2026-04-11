#ifndef ENHANCED_SIGNBOARD_H
#define ENHANCED_SIGNBOARD_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "shader.h"
#include <string>

// Signboard structure for displaying area status
class EnhancedSignboard {
public:
    glm::vec3 position;
    glm::vec3 scale;
    std::string text;
    glm::vec3 textColor;
    glm::vec3 boardColor;
    glm::vec3 frameColor;
    float rotation;
    unsigned int textureID;
    
    bool isActive = true;
    
    EnhancedSignboard(glm::vec3 pos, glm::vec3 sz, const std::string& txt, 
                     glm::vec3 brdColor, glm::vec3 txtColor, glm::vec3 frmColor, unsigned int texID = 0)
        : position(pos), scale(sz), text(txt), textColor(txtColor), 
          boardColor(brdColor), frameColor(frmColor), rotation(0.0f), isActive(true), textureID(texID) {}
    
    // Draw a signboard with frame and status indicators
    void draw(unsigned int &cubeVAO, unsigned int &texCubeVAO, Shader &shader, glm::mat4 parentMatrix, float globalTime) {
        glm::mat4 model = glm::translate(parentMatrix, position);
        model = glm::rotate(model, glm::radians(rotation), glm::vec3(0, 1, 0));
        
        // Outer frame (metal border)
        shader.use();
        shader.setMat4("model", model);
        shader.setVec3("material.ambient", frameColor * 0.7f);
        shader.setVec3("material.diffuse", frameColor);
        shader.setVec3("material.specular", glm::vec3(0.5f));
        shader.setFloat("material.shininess", 64.0f);
        
        glm::mat4 frameModel = glm::scale(model, scale + glm::vec3(0.1f, 0.1f, 0.02f));
        shader.setMat4("model", frameModel);
        glBindVertexArray(cubeVAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        
        // Main board (colored background or texture)
        shader.setVec3("material.ambient", boardColor * 0.8f);
        shader.setVec3("material.diffuse", boardColor);
        shader.setVec3("material.specular", glm::vec3(0.2f));
        shader.setFloat("material.shininess", 32.0f);
        
        glm::mat4 boardModel = glm::scale(model, scale);
        shader.setMat4("model", boardModel);
        
        if (textureID != 0) {
            shader.setBool("useTexture", true);
            shader.setBool("blendWithColor", true);
            shader.setFloat("texTiling", 1.0f);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureID);
            shader.setInt("material.diffuseMap", 0);
            
            glBindVertexArray(texCubeVAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            
            shader.setBool("useTexture", false);
            shader.setBool("blendWithColor", false);
        } else {
            glBindVertexArray(cubeVAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        }
        
        // Return to standard geometry
        glBindVertexArray(cubeVAO);
        
        // Status indicator lights (top corners of board)
        float lightBright1 = isActive ? (sin(globalTime * 6.0f) * 0.3f + 0.7f) : 0.1f;
        float lightBright2 = isActive ? (cos(globalTime * 4.0f) * 0.3f + 0.7f) : 0.1f;
        
        // Left indicator (operational)
        glm::vec3 leftLightColor = isActive ? glm::vec3(0.1f, 1.0f, 0.1f) : glm::vec3(1.0f, 0.1f, 0.1f);
        shader.setVec3("material.ambient", leftLightColor * lightBright1);
        shader.setVec3("material.diffuse", leftLightColor);
        shader.setVec3("material.specular", glm::vec3(0.8f));
        
        glm::vec3 leftLightPos = position + glm::vec3(-scale.x * 0.35f, scale.y * 0.35f, scale.z * 0.1f);
        glm::mat4 leftLight = glm::translate(model, leftLightPos - position);
        leftLight = glm::scale(leftLight, glm::vec3(0.08f, 0.08f, 0.05f));
        shader.setMat4("model", leftLight);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        
        // Right indicator (status)
        glm::vec3 rightLightColor = isActive ? glm::vec3(1.0f, 0.7f, 0.0f) : glm::vec3(0.5f, 0.1f, 0.1f);
        shader.setVec3("material.ambient", rightLightColor * lightBright2);
        shader.setVec3("material.diffuse", rightLightColor);
        
        glm::vec3 rightLightPos = position + glm::vec3(scale.x * 0.35f, scale.y * 0.35f, scale.z * 0.1f);
        glm::mat4 rightLight = glm::translate(model, rightLightPos - position);
        rightLight = glm::scale(rightLight, glm::vec3(0.08f, 0.08f, 0.05f));
        shader.setMat4("model", rightLight);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        
        // Corner reinforcement brackets
        shader.setVec3("material.ambient", frameColor);
        shader.setVec3("material.diffuse", frameColor);
        
        glm::vec3 corners[] = {
            position + glm::vec3(-scale.x * 0.45f, -scale.y * 0.45f, 0),
            position + glm::vec3(scale.x * 0.45f, -scale.y * 0.45f, 0),
            position + glm::vec3(-scale.x * 0.45f, scale.y * 0.45f, 0),
            position + glm::vec3(scale.x * 0.45f, scale.y * 0.45f, 0)
        };
        
        for (int i = 0; i < 4; i++) {
            glm::mat4 bracket = glm::translate(model, corners[i] - position);
            bracket = glm::scale(bracket, glm::vec3(0.06f, 0.06f, 0.04f));
            shader.setMat4("model", bracket);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        }
    }
};

#endif // ENHANCED_SIGNBOARD_H
