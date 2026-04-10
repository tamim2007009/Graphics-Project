#ifndef COMPLEX_MACHINE_H
#define COMPLEX_MACHINE_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"
#include "sphere.h"

namespace ComplexMachine {

// STAMPING PRESS IMPLEMENTATION
void drawAdvancedStampingPress(unsigned int &cubeVAO, Shader &shader, 
                               glm::mat4 baseModel, glm::vec3 metalColor, 
                               float pressStroke, float globalTime) {
    shader.use();
    
    // Press frame (large steel frame)
    glm::mat4 frame = glm::scale(baseModel, glm::vec3(3.0f, 4.5f, 2.5f));
    shader.setVec3("material.ambient", metalColor * 0.7f);
    shader.setVec3("material.diffuse", metalColor);
    shader.setVec3("material.specular", glm::vec3(0.6f));
    shader.setFloat("material.shininess", 64.0f);
    shader.setMat4("model", frame);
    glBindVertexArray(cubeVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    
    // Hydraulic cylinders (left and right)
    for (int side = -1; side <= 1; side += 2) {
        glm::mat4 cylinder = glm::translate(baseModel, glm::vec3(side * 1.0f, 1.5f, 0));
        cylinder = glm::scale(cylinder, glm::vec3(0.15f, 2.5f, 0.15f));
        
        shader.setVec3("material.ambient", glm::vec3(0.3f, 0.3f, 0.35f));
        shader.setVec3("material.diffuse", glm::vec3(0.4f, 0.4f, 0.45f));
        shader.setMat4("model", cylinder);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        
        // Piston rod (retracting/extending)
        float rodExtension = pressStroke * 1.8f;
        glm::mat4 rod = glm::translate(baseModel, glm::vec3(side * 1.0f, 0.5f + rodExtension, 0));
        rod = glm::scale(rod, glm::vec3(0.08f, 1.2f, 0.08f));
        
        shader.setVec3("material.ambient", glm::vec3(0.6f, 0.6f, 0.62f));
        shader.setVec3("material.diffuse", glm::vec3(0.7f, 0.7f, 0.72f));
        shader.setMat4("model", rod);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    }
    
    // Press head (upper die)
    glm::mat4 pressHead = glm::translate(baseModel, glm::vec3(0, 1.5f + pressStroke * 1.8f, 0));
    pressHead = glm::scale(pressHead, glm::vec3(2.8f, 0.4f, 2.2f));
    
    shader.setVec3("material.ambient", metalColor * 0.6f);
    shader.setVec3("material.diffuse", metalColor * 1.1f);
    shader.setMat4("model", pressHead);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    
    // Base die (lower die)
    glm::mat4 baseDie = glm::translate(baseModel, glm::vec3(0, -1.5f, 0));
    baseDie = glm::scale(baseDie, glm::vec3(2.8f, 0.5f, 2.2f));
    
    shader.setVec3("material.ambient", metalColor * 0.5f);
    shader.setVec3("material.diffuse", metalColor * 0.9f);
    shader.setMat4("model", baseDie);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    
    // Pressure gauges
    for (int i = 0; i < 2; i++) {
        float gaugeX = -1.2f + i * 2.4f;
        glm::mat4 gauge = glm::translate(baseModel, glm::vec3(gaugeX, 2.5f, 1.3f));
        gauge = glm::scale(gauge, glm::vec3(0.2f, 0.2f, 0.1f));
        
        shader.setVec3("material.ambient", glm::vec3(0.8f, 0.3f, 0.3f));
        shader.setVec3("material.diffuse", glm::vec3(1.0f, 0.4f, 0.4f));
        shader.setMat4("model", gauge);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    }
    
    // Mounting bolts
    for (int x = -1; x <= 1; x++) {
        for (int z = -1; z <= 1; z++) {
            if (x == 0 && z == 0) continue;
            glm::mat4 bolt = glm::translate(baseModel, glm::vec3(x * 1.2f, -1.8f, z * 1.0f));
            bolt = glm::scale(bolt, glm::vec3(0.12f, 0.12f, 0.12f));
            
            shader.setVec3("material.ambient", glm::vec3(0.4f, 0.4f, 0.42f));
            shader.setVec3("material.diffuse", glm::vec3(0.5f, 0.5f, 0.52f));
            shader.setMat4("model", bolt);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        }
    }
}

// WELDING ROBOT IMPLEMENTATION
void drawAdvancedWeldingRobot(unsigned int &cubeVAO, Shader &shader, 
                              glm::mat4 baseModel, float j1_rot, float j2_rot,
                              float j3_rot, float j4_rot, float j5_rot, float j6_rot,
                              float torchGlow, float globalTime, Sphere* jointSphere = nullptr) {
    shader.use();
    
    // Robot base (heavy cast iron)
    glm::mat4 base = glm::scale(baseModel, glm::vec3(1.8f, 0.5f, 1.8f));
    shader.setVec3("material.ambient", glm::vec3(0.2f, 0.2f, 0.22f));
    shader.setVec3("material.diffuse", glm::vec3(0.3f, 0.3f, 0.32f));
    shader.setVec3("material.specular", glm::vec3(0.4f));
    shader.setFloat("material.shininess", 32.0f);
    shader.setMat4("model", base);
    glBindVertexArray(cubeVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    
    // Joint 1: Base rotation
    glm::mat4 joint1 = glm::translate(baseModel, glm::vec3(0, 0.6f, 0));
    joint1 = glm::rotate(joint1, glm::radians(j1_rot), glm::vec3(0, 1, 0));
    
    glm::mat4 bearing1 = glm::scale(joint1, glm::vec3(0.5f, 0.25f, 0.5f));
    shader.setVec3("material.ambient", glm::vec3(0.25f, 0.25f, 0.28f));
    shader.setVec3("material.diffuse", glm::vec3(0.35f, 0.35f, 0.38f));
    if (jointSphere) {
        jointSphere->ambient = glm::vec3(0.25f, 0.25f, 0.28f);
        jointSphere->diffuse = glm::vec3(0.35f, 0.35f, 0.38f);
        jointSphere->drawSphere(shader, bearing1);
    } else {
        shader.setMat4("model", bearing1);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    }
    
    // Segment 1: Base to shoulder
    glm::mat4 seg1 = glm::translate(joint1, glm::vec3(0, 0.3f, 0));
    seg1 = glm::rotate(seg1, glm::radians(j2_rot), glm::vec3(1, 0, 0));
    
    glm::mat4 seg1Body = glm::translate(seg1, glm::vec3(0, 0.5f, 0));
    seg1Body = glm::scale(seg1Body, glm::vec3(0.25f, 1.0f, 0.22f));
    
    shader.setVec3("material.ambient", glm::vec3(0.2f, 0.2f, 0.22f));
    shader.setVec3("material.diffuse", glm::vec3(0.28f, 0.28f, 0.3f));
    shader.setMat4("model", seg1Body);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    
    // Joint 2: Shoulder
    glm::mat4 joint2 = glm::translate(seg1, glm::vec3(0, 1.0f, 0));
    joint2 = glm::rotate(joint2, glm::radians(j3_rot), glm::vec3(1, 0, 0));
    
    glm::mat4 bearing2 = glm::scale(joint2, glm::vec3(0.35f, 0.2f, 0.35f));
    shader.setVec3("material.diffuse", glm::vec3(0.32f, 0.32f, 0.35f));
    if (jointSphere) {
        jointSphere->ambient = glm::vec3(0.22f, 0.22f, 0.25f);
        jointSphere->diffuse = glm::vec3(0.32f, 0.32f, 0.35f);
        jointSphere->drawSphere(shader, bearing2);
    } else {
        shader.setMat4("model", bearing2);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    }
    
    // Segment 2: Shoulder to elbow
    glm::mat4 seg2 = glm::translate(joint2, glm::vec3(0, 0.25f, 0));
    seg2 = glm::rotate(seg2, glm::radians(j4_rot), glm::vec3(1, 0, 0));
    
    glm::mat4 seg2Body = glm::translate(seg2, glm::vec3(0, 0.45f, 0));
    seg2Body = glm::scale(seg2Body, glm::vec3(0.2f, 0.9f, 0.18f));
    
    shader.setVec3("material.diffuse", glm::vec3(0.3f, 0.3f, 0.32f));
    shader.setMat4("model", seg2Body);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    
    // Joint 3: Wrist
    glm::mat4 joint3 = glm::translate(seg2, glm::vec3(0, 0.9f, 0));
    joint3 = glm::rotate(joint3, glm::radians(j5_rot), glm::vec3(0, 0, 1));
    
    glm::mat4 bearing3 = glm::scale(joint3, glm::vec3(0.22f, 0.18f, 0.22f));
    shader.setVec3("material.diffuse", glm::vec3(0.35f, 0.35f, 0.38f));
    if (jointSphere) {
        jointSphere->ambient = glm::vec3(0.25f, 0.25f, 0.28f);
        jointSphere->diffuse = glm::vec3(0.35f, 0.35f, 0.38f);
        jointSphere->drawSphere(shader, bearing3);
    } else {
        shader.setMat4("model", bearing3);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    }
    
    // Wrist assembly (tool flange)
    glm::mat4 wrist = glm::translate(joint3, glm::vec3(0, 0.0f, 0));
    wrist = glm::rotate(wrist, glm::radians(j6_rot), glm::vec3(0, 1, 0));
    
    glm::mat4 wristBody = glm::scale(wrist, glm::vec3(0.2f, 0.12f, 0.2f));
    shader.setVec3("material.ambient", glm::vec3(0.3f, 0.3f, 0.32f));
    shader.setVec3("material.diffuse", glm::vec3(0.4f, 0.4f, 0.42f));
    shader.setMat4("model", wristBody);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    
    // Welding torch
    glm::mat4 torch = glm::translate(wrist, glm::vec3(0, -0.15f, 0));
    torch = glm::scale(torch, glm::vec3(0.08f, 0.25f, 0.08f));
    
    shader.setVec3("material.ambient", glm::vec3(0.1f, 0.1f, 0.12f));
    shader.setVec3("material.diffuse", glm::vec3(0.15f, 0.15f, 0.18f));
    shader.setMat4("model", torch);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    
    // Welding arc glow
    if (torchGlow > 0.1f) {
        glm::mat4 arcGlow = glm::translate(wrist, glm::vec3(0, -0.25f, 0));
        arcGlow = glm::scale(arcGlow, glm::vec3(0.15f, 0.08f, 0.15f));
        
        float glowIntensity = torchGlow;
        shader.setVec3("material.ambient", glm::vec3(1.0f, 0.6f, 0.0f) * glowIntensity);
        shader.setVec3("material.diffuse", glm::vec3(1.0f, 0.7f, 0.2f) * glowIntensity);
        shader.setVec3("material.specular", glm::vec3(1.0f, 0.8f, 0.4f) * glowIntensity);
        shader.setFloat("material.shininess", 128.0f);
        shader.setMat4("model", arcGlow);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    }
    
    // Cable & hose routing
    glm::mat4 cable = glm::translate(seg1, glm::vec3(-0.15f, 0.3f, 0));
    cable = glm::scale(cable, glm::vec3(0.02f, 0.8f, 0.02f));
    
    shader.setVec3("material.ambient", glm::vec3(0.3f, 0.2f, 0.1f));
    shader.setVec3("material.diffuse", glm::vec3(0.4f, 0.25f, 0.15f));
    shader.setMat4("model", cable);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    
    glm::mat4 hose = glm::translate(seg1, glm::vec3(0.15f, 0.3f, 0));
    hose = glm::scale(hose, glm::vec3(0.025f, 0.8f, 0.025f));
    
    shader.setVec3("material.ambient", glm::vec3(0.2f, 0.5f, 0.2f));
    shader.setVec3("material.diffuse", glm::vec3(0.3f, 0.7f, 0.3f));
    shader.setMat4("model", hose);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

// ADVANCED STRETCH WRAPPING MACHINE IMPLEMENTATION
void drawAdvancedWrappingMachine(unsigned int &cubeVAO, Shader &shader, glm::mat4 baseModel,
                                 float wrapRotation, float filmHeight, float globalTime) {
    shader.use();
    
    // Machine Base & Turntable
    glm::mat4 base = glm::translate(baseModel, glm::vec3(0, 0.1f, 0));
    base = glm::scale(base, glm::vec3(3.2f, 0.2f, 3.2f));
    shader.setVec3("material.ambient", glm::vec3(0.15f, 0.15f, 0.18f));
    shader.setVec3("material.diffuse", glm::vec3(0.25f, 0.25f, 0.3f));
    shader.setVec3("material.specular", glm::vec3(0.2f));
    shader.setFloat("material.shininess", 16.0f);
    shader.setMat4("model", base);
    glBindVertexArray(cubeVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    
    // Rotating platform
    glm::mat4 platform = glm::translate(baseModel, glm::vec3(0, 0.22f, 0));
    platform = glm::rotate(platform, glm::radians(wrapRotation), glm::vec3(0, 1, 0));
    platform = glm::scale(platform, glm::vec3(2.6f, 0.05f, 2.6f));
    shader.setVec3("material.diffuse", glm::vec3(0.4f, 0.4f, 0.45f));
    shader.setMat4("model", platform);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    // Warning hash marks along base edge
    for (int i = 0; i < 8; i++) {
        glm::mat4 hash = glm::translate(baseModel, glm::vec3(0, 0.21f, 0));
        hash = glm::rotate(hash, glm::radians(i * 45.0f), glm::vec3(0, 1, 0));
        hash = glm::translate(hash, glm::vec3(1.5f, 0, 0));
        hash = glm::scale(hash, glm::vec3(0.2f, 0.05f, 0.4f));
        shader.setVec3("material.diffuse", glm::vec3(0.9f, 0.8f, 0.1f));
        shader.setMat4("model", hash);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    }

    // Main Mast
    glm::mat4 mastBase = glm::translate(baseModel, glm::vec3(-1.8f, 0.5f, 0));
    mastBase = glm::scale(mastBase, glm::vec3(0.8f, 0.6f, 0.8f));
    shader.setVec3("material.diffuse", glm::vec3(0.1f, 0.4f, 0.6f));
    shader.setMat4("model", mastBase);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    
    glm::mat4 mast = glm::translate(baseModel, glm::vec3(-1.8f, 2.4f, 0));
    mast = glm::scale(mast, glm::vec3(0.5f, 3.2f, 0.6f));
    shader.setMat4("model", mast);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    // Control Panel
    glm::mat4 panel = glm::translate(baseModel, glm::vec3(-1.5f, 1.5f, 0.35f));
    panel = glm::rotate(panel, glm::radians(15.0f), glm::vec3(0, 1, 0));
    panel = glm::scale(panel, glm::vec3(0.1f, 0.6f, 0.4f));
    shader.setVec3("material.diffuse", glm::vec3(0.2f, 0.2f, 0.2f));
    shader.setMat4("model", panel);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    // LED screen
    glm::mat4 screen = glm::translate(baseModel, glm::vec3(-1.44f, 1.6f, 0.35f));
    screen = glm::rotate(screen, glm::radians(15.0f), glm::vec3(0, 1, 0));
    screen = glm::scale(screen, glm::vec3(0.02f, 0.2f, 0.3f));
    shader.setVec3("material.ambient", glm::vec3(0.0f, 0.8f, 1.0f) * 0.8f);
    shader.setVec3("material.diffuse", glm::vec3(0.0f, 1.0f, 1.0f));
    shader.setMat4("model", screen);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    
    // Film Carriage
    glm::mat4 carriage = glm::translate(baseModel, glm::vec3(-1.4f, filmHeight, 0));
    carriage = glm::scale(carriage, glm::vec3(0.4f, 0.7f, 0.5f));
    shader.setVec3("material.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
    shader.setVec3("material.diffuse", glm::vec3(0.3f, 0.3f, 0.3f));
    shader.setMat4("model", carriage);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    // Film Roll
    float rollAngle = globalTime * 300.0f;
    glm::mat4 filmRoll = glm::translate(baseModel, glm::vec3(-1.15f, filmHeight, 0));
    filmRoll = glm::rotate(filmRoll, glm::radians(rollAngle), glm::vec3(0, 1, 0));
    filmRoll = glm::scale(filmRoll, glm::vec3(0.25f, 0.6f, 0.25f));
    shader.setVec3("material.ambient", glm::vec3(0.8f, 0.9f, 0.95f) * 0.5f);
    shader.setVec3("material.diffuse", glm::vec3(0.9f, 0.95f, 1.0f));
    shader.setVec3("material.specular", glm::vec3(0.6f));
    shader.setFloat("material.shininess", 64.0f);
    shader.setMat4("model", filmRoll);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    
    // Top stabilizer arm
    glm::mat4 stabArm = glm::translate(baseModel, glm::vec3(-0.8f, 3.8f, 0));
    stabArm = glm::scale(stabArm, glm::vec3(2.4f, 0.2f, 0.4f));
    shader.setVec3("material.ambient", glm::vec3(0.15f, 0.15f, 0.18f));
    shader.setVec3("material.diffuse", glm::vec3(0.2f, 0.2f, 0.25f));
    shader.setMat4("model", stabArm);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    
    // Stabilizer press pad
    glm::mat4 stabPad = glm::translate(baseModel, glm::vec3(0, 2.5f, 0));
    stabPad = glm::scale(stabPad, glm::vec3(1.2f, 0.1f, 1.2f));
    shader.setVec3("material.ambient", glm::vec3(0.3f, 0.1f, 0.1f));
    shader.setVec3("material.diffuse", glm::vec3(0.8f, 0.2f, 0.2f));
    shader.setMat4("model", stabPad);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    
    // Central rod
    glm::mat4 stabRod = glm::translate(baseModel, glm::vec3(0, 3.15f, 0));
    stabRod = glm::scale(stabRod, glm::vec3(0.15f, 1.3f, 0.15f));
    shader.setVec3("material.diffuse", glm::vec3(0.7f, 0.7f, 0.75f));
    shader.setMat4("model", stabRod);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

} // namespace ComplexMachine

#endif // COMPLEX_MACHINE_H
