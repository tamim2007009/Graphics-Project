#ifndef COMPLEX_MACHINES_H
#define COMPLEX_MACHINES_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"

// Industrial machine drawing utilities
namespace ComplexMachine {

    // ===== ADVANCED STAMPING PRESS =====
    // REALISTIC INDUSTRIAL HYDRAULIC STAMPING MACHINE
    // Visual design: Large heavy-duty press with BRIGHT COLOR CHANGE feedback
    inline void drawAdvancedStampingPress(unsigned int &VAO, Shader &shader, glm::mat4 baseModel,
                                          glm::vec3 metalColor, float pressStroke, float globalTime)
    {
        shader.use();
        shader.setBool("useTexture", false);
        
        // ===== HEAVY BASE FOUNDATION =====
        // Large concrete-like base (prevents tipping from high forces)
        glm::mat4 baseFrame = glm::translate(baseModel, glm::vec3(0, 0.3f, 0));
        baseFrame = glm::scale(baseFrame, glm::vec3(5.0f, 0.6f, 3.5f));
        shader.setVec3("material.ambient", glm::vec3(0.3f, 0.3f, 0.3f));
        shader.setVec3("material.diffuse", glm::vec3(0.4f, 0.4f, 0.42f)); // Dark grey concrete
        shader.setVec3("material.specular", glm::vec3(0.2f));
        shader.setFloat("material.shininess", 16.0f);
        shader.setMat4("model", baseFrame);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        
        // ===== MAIN FRAME COLUMNS (4 heavy supports) =====
        glm::vec3 columnPositions[] = {
            {-1.8f, 2.2f, -1.2f},
            {1.8f, 2.2f, -1.2f},
            {-1.8f, 2.2f, 1.2f},
            {1.8f, 2.2f, 1.2f}
        };
        
        for (int i = 0; i < 4; i++) {
            glm::mat4 column = glm::translate(baseModel, columnPositions[i]);
            column = glm::scale(column, glm::vec3(0.4f, 4.5f, 0.4f));
            shader.setVec3("material.ambient", glm::vec3(0.2f, 0.2f, 0.25f));
            shader.setVec3("material.diffuse", glm::vec3(0.3f, 0.3f, 0.35f)); // Dark steel columns
            shader.setVec3("material.specular", glm::vec3(0.35f));
            shader.setFloat("material.shininess", 48.0f);
            shader.setMat4("model", column);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        }
        
        // ===== TOP CROSSBAR (connects all columns) =====
        glm::mat4 topBar = glm::translate(baseModel, glm::vec3(0, 4.4f, 0));
        topBar = glm::scale(topBar, glm::vec3(4.0f, 0.4f, 2.8f));
        shader.setVec3("material.ambient", glm::vec3(0.25f, 0.25f, 0.3f));
        shader.setVec3("material.diffuse", glm::vec3(0.35f, 0.35f, 0.4f)); // Reinforced top beam
        shader.setVec3("material.specular", glm::vec3(0.4f));
        shader.setFloat("material.shininess", 56.0f);
        shader.setMat4("model", topBar);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        
        // ===== HYDRAULIC CYLINDERS (LEFT & RIGHT) - MAIN ACTUATION =====
        // Left cylinder
        glm::mat4 cylLeft = glm::translate(baseModel, glm::vec3(-1.2f, 2.5f - pressStroke * 0.5f, -0.8f));
        cylLeft = glm::scale(cylLeft, glm::vec3(0.2f, 1.2f - pressStroke * 0.4f, 0.2f));
        shader.setVec3("material.ambient", glm::vec3(0.2f, 0.2f, 0.25f));
        shader.setVec3("material.diffuse", glm::vec3(0.45f, 0.45f, 0.5f)); // Bright cylinder walls
        shader.setVec3("material.specular", glm::vec3(0.5f));
        shader.setFloat("material.shininess", 72.0f);
        shader.setMat4("model", cylLeft);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        
        // Right cylinder
        glm::mat4 cylRight = glm::translate(baseModel, glm::vec3(1.2f, 2.5f - pressStroke * 0.5f, -0.8f));
        cylRight = glm::scale(cylRight, glm::vec3(0.2f, 1.2f - pressStroke * 0.4f, 0.2f));
        shader.setMat4("model", cylRight);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        
        // ===== PISTON RODS (extend/retract with press motion) =====
        // Left piston rod (shiny polished)
        glm::mat4 rodLeft = glm::translate(baseModel, glm::vec3(-1.2f, 1.8f + pressStroke * 0.5f, -0.8f));
        rodLeft = glm::scale(rodLeft, glm::vec3(0.1f, 0.8f, 0.1f));
        shader.setVec3("material.ambient", glm::vec3(0.4f, 0.4f, 0.45f));
        shader.setVec3("material.diffuse", glm::vec3(0.7f, 0.7f, 0.75f)); // Chrome/polished steel
        shader.setVec3("material.specular", glm::vec3(0.8f));
        shader.setFloat("material.shininess", 96.0f);
        shader.setMat4("model", rodLeft);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        
        // Right piston rod
        glm::mat4 rodRight = glm::translate(baseModel, glm::vec3(1.2f, 1.8f + pressStroke * 0.5f, -0.8f));
        rodRight = glm::scale(rodRight, glm::vec3(0.1f, 0.8f, 0.1f));
        shader.setMat4("model", rodRight);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        
        // ===== PRESS HEAD (UPPER DIE) - MOVES DOWN =====
        // This is the MAIN visual element that shows pressing action
        float headY = 3.8f - (pressStroke * 1.5f); // Moves down significantly
        glm::mat4 headModel = glm::translate(baseModel, glm::vec3(0, headY, 0));
        headModel = glm::scale(headModel, glm::vec3(3.2f, 0.5f, 2.4f));
        
        // COLOR CHANGES BASED ON PRESSURE - BRIGHT & VISIBLE!
        glm::vec3 headColor;
        if (pressStroke < 0.3f) {
            // Retracted: Cool blue-grey (idle state)
            headColor = glm::vec3(0.3f, 0.4f, 0.55f); // Cool blue-steel
        } else if (pressStroke < 0.7f) {
            // Approaching: Warm yellow (getting hot from friction)
            float blend = (pressStroke - 0.3f) / 0.4f;
            headColor = glm::mix(glm::vec3(0.3f, 0.4f, 0.55f), glm::vec3(1.0f, 0.85f, 0.2f), blend);
        } else {
            // Pressing: BRIGHT HOT ORANGE-RED (maximum pressure!)
            float blend = (pressStroke - 0.7f) / 0.3f;
            headColor = glm::mix(glm::vec3(1.0f, 0.85f, 0.2f), glm::vec3(1.0f, 0.35f, 0.1f), blend);
        }
        
        shader.setVec3("material.ambient", headColor * 0.7f);
        shader.setVec3("material.diffuse", headColor); // BRIGHT & VISIBLE COLOR
        shader.setVec3("material.specular", glm::vec3(0.6f + pressStroke * 0.3f)); // More reflective when hot
        shader.setFloat("material.shininess", 64.0f + pressStroke * 40.0f);
        shader.setMat4("model", headModel);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        
        // Add GLOW EFFECT on press head when under extreme pressure
        if (pressStroke > 0.8f) {
            glm::mat4 glowHead = glm::translate(baseModel, glm::vec3(0, headY + 0.1f, 0));
            glowHead = glm::scale(glowHead, glm::vec3(3.3f, 0.55f, 2.5f));
            float glowIntensity = (pressStroke - 0.8f) / 0.2f; // 0 to 1 at maximum pressure
            shader.setVec3("material.ambient", glm::vec3(1.0f, 0.5f, 0.1f) * glowIntensity * 0.5f);
            shader.setVec3("material.diffuse", glm::vec3(1.0f, 0.4f, 0.0f) * glowIntensity * 0.3f);
            shader.setMat4("model", glowHead);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        }
        
        // ===== BASE DIE PLATE (LOWER DIE) - STATIONARY =====
        glm::mat4 baseDie = glm::translate(baseModel, glm::vec3(0, 0.8f, 0));
        baseDie = glm::scale(baseDie, glm::vec3(3.0f, 0.4f, 2.2f));
        shader.setVec3("material.ambient", glm::vec3(0.2f, 0.2f, 0.25f));
        shader.setVec3("material.diffuse", glm::vec3(0.28f, 0.28f, 0.32f)); // Dark fixed base
        shader.setVec3("material.specular", glm::vec3(0.3f));
        shader.setFloat("material.shininess", 40.0f);
        shader.setMat4("model", baseDie);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        
        // ===== PRESSURE GAUGES (show visual feedback) =====
        for (int g = 0; g < 2; g++) {
            float gaugeX = g == 0 ? -1.5f : 1.5f;
            glm::mat4 gauge = glm::translate(baseModel, glm::vec3(gaugeX, 3.5f, 1.4f));
            gauge = glm::scale(gauge, glm::vec3(0.25f, 0.25f, 0.1f));
            
            // Gauge color represents pressure
            glm::vec3 gaugeColor;
            if (pressStroke < 0.3f) {
                gaugeColor = glm::vec3(0.2f, 0.8f, 0.2f); // GREEN = low pressure
            } else if (pressStroke < 0.7f) {
                gaugeColor = glm::vec3(1.0f, 0.9f, 0.1f); // YELLOW = medium pressure
            } else {
                gaugeColor = glm::vec3(1.0f, 0.2f, 0.2f); // RED = high pressure!
            }
            
            shader.setVec3("material.ambient", gaugeColor * 0.6f);
            shader.setVec3("material.diffuse", gaugeColor);
            shader.setVec3("material.specular", glm::vec3(0.7f));
            shader.setFloat("material.shininess", 80.0f);
            shader.setMat4("model", gauge);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        }
        
        // ===== CONNECTING RODS & LINKAGES =====
        // Mechanical linkage on left
        glm::mat4 linkLeft = glm::translate(baseModel, glm::vec3(-1.2f, 3.2f - pressStroke * 0.3f, -1.5f));
        linkLeft = glm::scale(linkLeft, glm::vec3(0.1f, 0.8f, 0.1f));
        shader.setVec3("material.diffuse", glm::vec3(0.5f, 0.5f, 0.55f)); // Medium steel
        shader.setMat4("model", linkLeft);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        
        // Mechanical linkage on right
        glm::mat4 linkRight = glm::translate(baseModel, glm::vec3(1.2f, 3.2f - pressStroke * 0.3f, -1.5f));
        linkRight = glm::scale(linkRight, glm::vec3(0.1f, 0.8f, 0.1f));
        shader.setMat4("model", linkRight);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        
        // ===== MOUNTING BOLTS (4 bolts at corners) =====
        glm::vec3 boltPositions[] = {
            {-1.6f, 0.6f, -1.0f},
            {1.6f, 0.6f, -1.0f},
            {-1.6f, 0.6f, 1.0f},
            {1.6f, 0.6f, 1.0f}
        };
        
        for (int b = 0; b < 4; b++) {
            glm::mat4 bolt = glm::translate(baseModel, boltPositions[b]);
            bolt = glm::scale(bolt, glm::vec3(0.15f, 0.15f, 0.15f));
            shader.setVec3("material.diffuse", glm::vec3(0.4f, 0.4f, 0.42f)); // Grey bolts
            shader.setMat4("model", bolt);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        }
    }

    // ===== ADVANCED WELDING ROBOT (6-AXIS ARTICULATED ARM) =====
    // REALISTIC INDUSTRIAL ROBOTIC ARM with IMPRESSIVE VISUAL FEEDBACK
    inline void drawAdvancedWeldingRobot(unsigned int &VAO, Shader &shader, glm::mat4 baseModel,
                                         float j1_baseRotation, float j2_shoulderRot,
                                         float j3_elbowRot, float j4_wristPitch, float j5_wristRoll,
                                         float j6_toolRot, float weldGlow, float globalTime)
    {
        shader.use();
        shader.setBool("useTexture", false);
        
        // ===== MASSIVE ROBOT BASE (Heavy mounting for stability) =====
        glm::mat4 massiveBase = glm::translate(baseModel, glm::vec3(0, 0.4f, 0));
        massiveBase = glm::scale(massiveBase, glm::vec3(2.2f, 0.8f, 2.2f));
        shader.setVec3("material.ambient", glm::vec3(0.15f, 0.15f, 0.17f));
        shader.setVec3("material.diffuse", glm::vec3(0.25f, 0.25f, 0.28f)); // Heavy dark grey
        shader.setVec3("material.specular", glm::vec3(0.3f));
        shader.setFloat("material.shininess", 32.0f);
        shader.setMat4("model", massiveBase);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        
        // Base mounting bolts (4 corners for visual detail)
        glm::vec3 baseBoltPositions[] = {
            {-1.0f, 0.2f, -1.0f},
            {1.0f, 0.2f, -1.0f},
            {-1.0f, 0.2f, 1.0f},
            {1.0f, 0.2f, 1.0f}
        };
        for (int i = 0; i < 4; i++) {
            glm::mat4 boltBase = glm::translate(baseModel, baseBoltPositions[i]);
            boltBase = glm::scale(boltBase, glm::vec3(0.18f, 0.18f, 0.18f));
            shader.setVec3("material.diffuse", glm::vec3(0.4f, 0.4f, 0.42f));
            shader.setMat4("model", boltBase);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        }
        
        // ===== JOINT 1: BASE ROTATION (Large bearing) =====
        glm::mat4 j1 = glm::translate(baseModel, glm::vec3(0, 1.0f, 0));
        j1 = glm::rotate(j1, glm::radians(j1_baseRotation), glm::vec3(0, 1, 0));
        
        glm::mat4 bearing1 = glm::translate(j1, glm::vec3(0, 0, 0));
        bearing1 = glm::scale(bearing1, glm::vec3(0.6f, 0.35f, 0.6f));
        shader.setVec3("material.ambient", glm::vec3(0.2f, 0.2f, 0.25f));
        shader.setVec3("material.diffuse", glm::vec3(0.35f, 0.35f, 0.4f)); // Joint colour
        shader.setVec3("material.specular", glm::vec3(0.4f));
        shader.setFloat("material.shininess", 48.0f);
        shader.setMat4("model", bearing1);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        
        // ===== SEGMENT 1: UPPER ARM (Large articulated section) =====
        glm::mat4 seg1 = glm::translate(j1, glm::vec3(0, 0.25f, 0));
        seg1 = glm::rotate(seg1, glm::radians(j2_shoulderRot), glm::vec3(1, 0, 0));
        
        glm::mat4 seg1Body = glm::translate(seg1, glm::vec3(0, 0.45f, 0.45f));
        seg1Body = glm::scale(seg1Body, glm::vec3(0.32f, 0.18f, 1.0f));
        shader.setVec3("material.ambient", glm::vec3(0.2f, 0.2f, 0.22f));
        shader.setVec3("material.diffuse", glm::vec3(0.32f, 0.32f, 0.36f)); // Upper arm
        shader.setVec3("material.specular", glm::vec3(0.35f));
        shader.setFloat("material.shininess", 40.0f);
        shader.setMat4("model", seg1Body);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        
        // ===== JOINT 2: SHOULDER BEARING =====
        glm::mat4 j2 = glm::translate(seg1, glm::vec3(0, 0.35f, 0.75f));
        j2 = glm::rotate(j2, glm::radians(j3_elbowRot), glm::vec3(1, 0, 0));
        
        glm::mat4 bearing2 = glm::translate(j2, glm::vec3(0, 0, 0));
        bearing2 = glm::scale(bearing2, glm::vec3(0.45f, 0.28f, 0.45f));
        shader.setVec3("material.diffuse", glm::vec3(0.3f, 0.3f, 0.35f)); // Joint
        shader.setMat4("model", bearing2);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        
        // ===== SEGMENT 2: FOREARM (Reaches toward workpiece) =====
        glm::mat4 seg2 = glm::translate(j2, glm::vec3(0, 0.2f, 0.4f));
        seg2 = glm::rotate(seg2, glm::radians(j4_wristPitch), glm::vec3(0, 1, 0));
        
        glm::mat4 seg2Body = glm::translate(seg2, glm::vec3(0, 0.25f, 0.4f));
        seg2Body = glm::scale(seg2Body, glm::vec3(0.28f, 0.16f, 0.85f));
        shader.setVec3("material.diffuse", glm::vec3(0.38f, 0.38f, 0.42f)); // Forearm
        shader.setMat4("model", seg2Body);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        
        // ===== JOINT 3: WRIST ASSEMBLY (precision interface) =====
        glm::mat4 j3 = glm::translate(seg2, glm::vec3(0, 0.18f, 0.6f));
        j3 = glm::rotate(j3, glm::radians(j5_wristRoll), glm::vec3(0, 0, 1));
        
        glm::mat4 bearing3 = glm::translate(j3, glm::vec3(0, 0, 0));
        bearing3 = glm::scale(bearing3, glm::vec3(0.32f, 0.24f, 0.32f));
        shader.setVec3("material.diffuse", glm::vec3(0.35f, 0.35f, 0.4f)); // Wrist bearing
        shader.setMat4("model", bearing3);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        
        // ===== END EFFECTOR: WELDING TORCH (the actual tool) =====
        glm::mat4 torch = glm::translate(j3, glm::vec3(0, 0, 0.2f));
        torch = glm::rotate(torch, glm::radians(j6_toolRot), glm::vec3(0, 1, 0));
        
        // Torch nozzle (copper-gold color)
        glm::mat4 nozzle = glm::translate(torch, glm::vec3(0, -0.18f, 0.25f));
        nozzle = glm::scale(nozzle, glm::vec3(0.12f, 0.1f, 0.2f));
        shader.setVec3("material.ambient", glm::vec3(0.4f, 0.3f, 0.1f));
        shader.setVec3("material.diffuse", glm::vec3(0.85f, 0.65f, 0.25f)); // Copper torch nozzle
        shader.setVec3("material.specular", glm::vec3(0.7f, 0.6f, 0.3f));
        shader.setFloat("material.shininess", 88.0f);
        shader.setMat4("model", nozzle);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        
        // ===== WELDING ARC GLOW (IMPRESSIVE VISUAL EFFECT!) =====
        if (weldGlow > 0.15f) {
            // Main arc glow
            glm::mat4 arcGlow = glm::translate(torch, glm::vec3(0, -0.28f, 0.35f));
            arcGlow = glm::scale(arcGlow, glm::vec3(0.2f, 0.12f, 0.2f));
            
            // BRIGHT CYAN GLOW - very visible!
            float glowIntensity = weldGlow * (0.5f + sin(globalTime * 8.0f) * 0.3f); // Pulsing effect
            shader.setVec3("material.ambient", glm::vec3(0.0f, 0.95f, 1.0f) * glowIntensity);
            shader.setVec3("material.diffuse", glm::vec3(0.1f, 1.0f, 1.0f) * glowIntensity);
            shader.setVec3("material.specular", glm::vec3(1.0f, 1.0f, 1.0f) * glowIntensity);
            shader.setFloat("material.shininess", 128.0f);
            shader.setMat4("model", arcGlow);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            
            // Secondary hot glow (orange-red underneath)
            glm::mat4 heatGlow = glm::translate(torch, glm::vec3(0, -0.32f, 0.38f));
            heatGlow = glm::scale(heatGlow, glm::vec3(0.25f, 0.08f, 0.25f));
            float heatIntensity = glowIntensity * 0.7f;
            shader.setVec3("material.ambient", glm::vec3(1.0f, 0.6f, 0.0f) * heatIntensity);
            shader.setVec3("material.diffuse", glm::vec3(1.0f, 0.5f, 0.0f) * heatIntensity);
            shader.setVec3("material.specular", glm::vec3(1.0f, 0.7f, 0.2f) * heatIntensity);
            shader.setMat4("model", heatGlow);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        }
        
        // ===== CABLE MANAGEMENT (Power cables running along arm) =====
        // Power cable (dark rubber)
        glm::mat4 powerCable = glm::translate(seg1, glm::vec3(-0.18f, 0.35f, 0.3f));
        powerCable = glm::scale(powerCable, glm::vec3(0.05f, 1.0f, 0.05f));
        shader.setVec3("material.ambient", glm::vec3(0.15f, 0.1f, 0.05f));
        shader.setVec3("material.diffuse", glm::vec3(0.2f, 0.15f, 0.08f)); // Dark rubber cable
        shader.setVec3("material.specular", glm::vec3(0.1f));
        shader.setFloat("material.shininess", 8.0f);
        shader.setMat4("model", powerCable);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        
        // Pneumatic hose (bright green for visibility)
        glm::mat4 pneumaticHose = glm::translate(seg1, glm::vec3(0.18f, 0.35f, 0.3f));
        pneumaticHose = glm::scale(pneumaticHose, glm::vec3(0.06f, 1.0f, 0.06f));
        shader.setVec3("material.ambient", glm::vec3(0.1f, 0.4f, 0.1f));
        shader.setVec3("material.diffuse", glm::vec3(0.3f, 0.85f, 0.3f)); // Bright green hose
        shader.setVec3("material.specular", glm::vec3(0.2f));
        shader.setFloat("material.shininess", 16.0f);
        shader.setMat4("model", pneumaticHose);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        
        // ===== JOINT DECORATIVE BOLTS (Adds realism and detail) =====
        // Bolts around each joint for visual detail
        glm::vec3 jointPositions[] = {bearing1[3], bearing2[3], bearing3[3]};
        for (int j = 0; j < 3; j++) {
            for (int b = 0; b < 2; b++) {
                glm::mat4 jointBolt = glm::translate(baseModel, 
                    glm::vec3(jointPositions[j].x + (b == 0 ? -0.12f : 0.12f), 
                              jointPositions[j].y, 
                              jointPositions[j].z));
                jointBolt = glm::scale(jointBolt, glm::vec3(0.08f, 0.08f, 0.08f));
                shader.setVec3("material.diffuse", glm::vec3(0.35f, 0.35f, 0.38f));
                shader.setMat4("model", jointBolt);
                glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            }
        }
    }

    // ===== ADVANCED STRETCH WRAPPING MACHINE =====
    // REALISTIC ROTARY ARM WRAPPER WITH FILM DISPENSER
    inline void drawAdvancedWrappingMachine(unsigned int &VAO, Shader &shader, glm::mat4 baseModel,
                                            float wrapRotation, float filmHeight, float globalTime)
    {
        shader.use();
        shader.setBool("useTexture", false);
        
        // ===== MACHINE BASE & TURNTABLE =====
        glm::mat4 base = glm::translate(baseModel, glm::vec3(0, 0.1f, 0));
        base = glm::scale(base, glm::vec3(3.2f, 0.2f, 3.2f));
        shader.setVec3("material.ambient", glm::vec3(0.15f, 0.15f, 0.18f));
        shader.setVec3("material.diffuse", glm::vec3(0.25f, 0.25f, 0.3f)); // Heavy dark grey
        shader.setVec3("material.specular", glm::vec3(0.2f));
        shader.setFloat("material.shininess", 16.0f);
        shader.setMat4("model", base);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        
        // Rotating platform (center)
        glm::mat4 platform = glm::translate(baseModel, glm::vec3(0, 0.22f, 0));
        platform = glm::rotate(platform, glm::radians(wrapRotation), glm::vec3(0, 1, 0));
        platform = glm::scale(platform, glm::vec3(2.6f, 0.05f, 2.6f));
        shader.setVec3("material.diffuse", glm::vec3(0.4f, 0.4f, 0.45f)); // Lighter metal
        shader.setMat4("model", platform);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        // Warning hash marks along base edge
        for (int i = 0; i < 8; i++) {
            glm::mat4 hash = glm::translate(baseModel, glm::vec3(0, 0.21f, 0));
            hash = glm::rotate(hash, glm::radians(i * 45.0f), glm::vec3(0, 1, 0));
            hash = glm::translate(hash, glm::vec3(1.5f, 0, 0));
            hash = glm::scale(hash, glm::vec3(0.2f, 0.05f, 0.4f));
            shader.setVec3("material.diffuse", glm::vec3(0.9f, 0.8f, 0.1f)); // Yellow
            shader.setMat4("model", hash);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        }

        // ===== MAIN MAST (TOWER) =====
        glm::mat4 mastBase = glm::translate(baseModel, glm::vec3(-1.8f, 0.5f, 0));
        mastBase = glm::scale(mastBase, glm::vec3(0.8f, 0.6f, 0.8f));
        shader.setVec3("material.diffuse", glm::vec3(0.1f, 0.4f, 0.6f)); // Industrial Blue
        shader.setMat4("model", mastBase);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        
        glm::mat4 mast = glm::translate(baseModel, glm::vec3(-1.8f, 2.4f, 0));
        mast = glm::scale(mast, glm::vec3(0.5f, 3.2f, 0.6f));
        shader.setMat4("model", mast);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        // Control Panel on mast
        glm::mat4 panel = glm::translate(baseModel, glm::vec3(-1.5f, 1.5f, 0.35f));
        panel = glm::rotate(panel, glm::radians(15.0f), glm::vec3(0, 1, 0));
        panel = glm::scale(panel, glm::vec3(0.1f, 0.6f, 0.4f));
        shader.setVec3("material.diffuse", glm::vec3(0.2f, 0.2f, 0.2f));
        shader.setMat4("model", panel);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        // LED screen on panel
        glm::mat4 screen = glm::translate(baseModel, glm::vec3(-1.44f, 1.6f, 0.35f));
        screen = glm::rotate(screen, glm::radians(15.0f), glm::vec3(0, 1, 0));
        screen = glm::scale(screen, glm::vec3(0.02f, 0.2f, 0.3f));
        shader.setVec3("material.ambient", glm::vec3(0.0f, 0.8f, 1.0f) * 0.8f);
        shader.setVec3("material.diffuse", glm::vec3(0.0f, 1.0f, 1.0f));
        shader.setMat4("model", screen);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        
        // Buttons
        for (int i = 0; i < 3; i++) {
            glm::mat4 btn = glm::translate(baseModel, glm::vec3(-1.44f, 1.4f - i*0.08f, 0.35f));
            btn = glm::rotate(btn, glm::radians(15.0f), glm::vec3(0, 1, 0));
            btn = glm::scale(btn, glm::vec3(0.03f, 0.05f, 0.05f));
            glm::vec3 btnColor = i == 0 ? glm::vec3(1,0,0) : i == 1 ? glm::vec3(0,1,0) : glm::vec3(1,1,0);
            shader.setVec3("material.ambient", btnColor);
            shader.setVec3("material.diffuse", btnColor);
            shader.setMat4("model", btn);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        }
        
        // ===== FILM CARRIAGE (MOVES UP AND DOWN) =====
        // Carriage body
        glm::mat4 carriage = glm::translate(baseModel, glm::vec3(-1.4f, filmHeight, 0));
        carriage = glm::scale(carriage, glm::vec3(0.4f, 0.7f, 0.5f));
        shader.setVec3("material.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
        shader.setVec3("material.diffuse", glm::vec3(0.3f, 0.3f, 0.3f));
        shader.setMat4("model", carriage);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        // Film Roll (white/transparent cylinder approximated by scaled cube)
        // Spin it as wrap progresses
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
        
        // Top stabilizer arm (holds pallet down)
        glm::mat4 stabArm = glm::translate(baseModel, glm::vec3(-0.8f, 3.8f, 0));
        stabArm = glm::scale(stabArm, glm::vec3(2.4f, 0.2f, 0.4f));
        shader.setVec3("material.ambient", glm::vec3(0.15f, 0.15f, 0.18f));
        shader.setVec3("material.diffuse", glm::vec3(0.2f, 0.2f, 0.25f));
        shader.setMat4("model", stabArm);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        
        // Stabilizer press pad (moves down to lock load)
        glm::mat4 stabPad = glm::translate(baseModel, glm::vec3(0, 2.5f, 0));
        stabPad = glm::scale(stabPad, glm::vec3(1.2f, 0.1f, 1.2f));
        shader.setVec3("material.ambient", glm::vec3(0.3f, 0.1f, 0.1f));
        shader.setVec3("material.diffuse", glm::vec3(0.8f, 0.2f, 0.2f));  // Red safety pad
        shader.setMat4("model", stabPad);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        
        // Central rod for stabilizer
        glm::mat4 stabRod = glm::translate(baseModel, glm::vec3(0, 3.15f, 0));
        stabRod = glm::scale(stabRod, glm::vec3(0.15f, 1.3f, 0.15f));
        shader.setVec3("material.diffuse", glm::vec3(0.7f, 0.7f, 0.75f)); // Chrome
        shader.setMat4("model", stabRod);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    }

} // namespace ComplexMachine

#endif // COMPLEX_MACHINES_H
