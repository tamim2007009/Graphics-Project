#ifndef SCENE_RENDERER_H
#define SCENE_RENDERER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "shader.h"
#include "basic_camera.h"
#include "pointLight.h"
#include "spotLight.h"
#include "sphere.h"
#include "pyramid.h"
#include "bezierCurve.h"
#include "fractalTree.h"
#include "scene_state.h"
#include "scene_colors.h"
#include "render_utils.h"
#include "complex_machine.h"
#include "enhanced_signboard.h"

// External globals from main.cpp
extern BasicCamera basic_camera;
extern const float MH;
extern const float WT;

extern const int NUM_POINT_LIGHTS;
extern glm::vec3 plPos[];
extern PointLight *pointLights[];

extern const int NUM_SPOT_LIGHTS;
extern SpotLight* spotLights_arr[];

extern Sphere *sphWheel;
extern Sphere *sphHub;
extern Sphere *sphLeaf[];
extern Pyramid *treeCone;

extern const int NUM_TREES;
extern FractalTree* fractalTrees[];

extern BezierRevolvedSurface* bezierTable;
extern BezierArch* bezierArch;
extern BezierRevolvedSurface* bezierColorChamber;
extern BezierRevolvedSurface* mathCone;

extern VisualizableEscalatorHandrail* visibleHandrail;
extern VisualizableConveyorPath* visibleConveyorPath;

extern std::vector<EnhancedSignboard> signboards;

inline void drawScene(unsigned int &V, unsigned int &LV, Shader &ls, Shader &fs, glm::mat4 view, glm::mat4 proj)
{
    ls.use();
    ls.setMat4("view", view);
    ls.setMat4("projection", proj);
    ls.setVec3("viewPos", basic_camera.eye);
    glm::mat4 I(1);
    float WH = MH / 2;

    auto drawOutdoor = [&]() {
    // OUTDOOR ENVIRONMENT

    // Grass ground (blended texture with surface color)
    ls.setBool("blendWithColor", true);
    drawCubeTextured(texCubeVAO, ls, I, C_GRASS, {0, -.1f, 15}, {200, .12f, 160}, texGrass, 12, 8);
    drawCubeTextured(texCubeVAO, ls, I, C_GRASS2, {-40, -.08f, 20}, {25, .1f, 20}, texGrass, 4, 8);
    drawCubeTextured(texCubeVAO, ls, I, C_GRASS2, {42, -.08f, -5}, {18, .1f, 14}, texGrass, 4, 8);
    ls.setBool("blendWithColor", false);

    // Main E-W road south of parking - with asphalt texture
    drawCubeTextured(texCubeVAO, ls, I, C_ROAD, {0, -.04f, 60}, {130, .06f, 8}, texAsphalt, 16.0f, 16, 0.95f);
    for (int i = 0; i < 18; i++)
        drawCube(V, ls, I, C_ROAD_YEL, {-64 + i * 7.5f, -.01f, 60}, {4.5f, .02f, .12f});
    drawCube(V, ls, I, C_SIDEWALK, {0, .05f, 65}, {130, .12f, 2});
    drawCube(V, ls, I, C_SIDEWALK, {0, .05f, 55}, {130, .12f, 2});
    // Access road into parking
    drawCube(V, ls, I, C_ROAD, {0, -.04f, 52}, {8, .06f, 16});
    // West side road
    drawCube(V, ls, I, C_ROAD, {-30, -.04f, 5}, {6, .06f, 60});
    drawCube(V, ls, I, C_SIDEWALK, {-26.5f, .05f, 5}, {1.5f, .12f, 60});

    // FRACTAL TREES (procedurally generated, replacing old pyramid cones)
    {
        glm::vec3 treeP[] = {{-20, 0, 28}, {22, 0, 28}, {-36, 0, 15}, {-36, 0, -10}, {36, 0, 12}, {36, 0, -15}, {-16, 0, 62}, {16, 0, 62}, {-45, 0, 40}, {45, 0, -18}};
        for (int ti = 0; ti < NUM_TREES; ti++) {
            glm::mat4 treeModel = glm::translate(I, treeP[ti]);
            fractalTrees[ti]->drawBranches(ls, treeModel, texTreeBark, texturesEnabled);
            fractalTrees[ti]->drawLeaves(ls, treeModel, texTreeLeaf, texturesEnabled);
        }
    }

    };
    drawOutdoor();

    auto drawParkingLot = [&]() {
    // PARKING LOT (Z: 27 to 55)

    // ======== ASSIGNMENT SHOWCASE EXHIBITION (Inside SE Corner) ========
    // Perfectly hits all criteria: multiple textured objects, texture types, surface blending, vertex vs fragment, curvy sphere/cone.
    if (exShowcase) {
        float exX = 21.0f;
        float exZ = 21.0f;
        
        // Base Stage Platform
        drawCubeTextured(texCubeVAO, ls, I, C_METAL, {exX, 0.2f, exZ}, {6.5f, 0.4f, 6.5f}, texWood, 4.0f, 64, 1.0f);
        drawCube(V, ls, I, C_CYAN_ACCENT, {exX, 0.25f, exZ}, {6.3f, 0.35f, 6.3f}); // Neon trim
        
        // Dynamic informative Signboards above the stand
        drawCube(V, ls, I, C_DARK_METAL, {exX, 3.5f, exZ - 2.8f}, {0.2f, 7.0f, 0.2f}); // Post
        drawCube(V, ls, I, C_MACHINE, {exX, 6.5f, exZ - 2.6f}, {6.0f, 1.5f, 0.3f}); // Screen border
        drawCube(V, ls, I, C_CYAN_ACCENT * 0.8f, {exX, 6.5f, exZ - 2.45f}, {5.8f, 1.3f, 0.1f}, 128, 0.8f); // Screen
        
        // -------------------------------------------------------------
        // Setup Exhibition Object Properties Based on the user's Toggles
        // -------------------------------------------------------------
        
        // Save global states to perfectly demonstrate toggles exclusively on the exhibition
        int prevShadingMode = shadingMode;
        
        // We will force the object's shading mode (Vertex Gouraud = 1, Fragment Phong = 0)
        ls.use();
        ls.setInt("shadingMode", exVertexShade ? 1 : 0);
        
        // If blended, the object color is strongly yellow. If simple texture, we make it white to verify it works without surface color influence.
        glm::vec3 objColor;
        bool doBlend = false;
        
        if (exBlendMode == 0) {
            // State 0: Simple Surface Color (Yellow)
            objColor = glm::vec3(1.0f, 1.0f, 0.0f);
            doBlend = false; // We won't use texture at all
        } else if (exBlendMode == 1) {
            // State 1: Texture Only
            objColor = glm::vec3(1.0f, 1.0f, 1.0f);
            doBlend = false;
        } else {
            // State 2: Blended (Yellow + Texture)
            objColor = glm::vec3(1.0f, 1.0f, 0.0f);
            doBlend = true;
        }
        
        // Force blend mode for the fragment shader inside the exhibition call
        ls.setBool("blendWithColor", doBlend);
        
        // Rotate the display object
        glm::mat4 objModel = glm::translate(I, {exX, 1.5f, exZ});
        objModel = glm::rotate(objModel, glm::radians(globalTime * 30.0f), {0, 1, 0});
        
        unsigned int exTex = texContainer2; // Try to use the container! Fallbacks applied in load

        // We draw the "textured curvy surfaced object (at least sphere and cone)"
        if (exBlendMode == 0) {
            // Draw without texture
            ls.setBool("useTexture", false);
            if (exObjectType == 0) {
                // Draw Sphere
                objModel = glm::scale(objModel, {1.6f, 1.6f, 1.6f});
                sphHub->ambient = objColor;
                sphHub->diffuse = objColor;
                sphHub->drawSphere(ls, objModel); 
            } else {
                // Draw Mathematical Curvy Cone
                objModel = glm::translate(objModel, {0.0f, -0.8f, 0.0f}); // Base center alignment
                objModel = glm::scale(objModel, {1.5f, 1.5f, 1.5f});
                mathCone->draw(ls, objModel, objColor, 64.0f);
            }
        } else {
            // Draw with Texture
            if (exObjectType == 0) {
                // Draw Sphere
                objModel = glm::scale(objModel, {1.6f, 1.6f, 1.6f});
                sphHub->ambient = objColor;
                sphHub->diffuse = objColor;
                sphHub->drawSphereTextured(ls, objModel, exTex, 2.0f); // Tiled sphere
            } else {
                // Draw Mathematical Curvy Cone
                objModel = glm::translate(objModel, {0.0f, -0.8f, 0.0f}); // Base center alignment
                objModel = glm::scale(objModel, {1.5f, 1.5f, 1.5f});
                mathCone->drawTextured(ls, objModel, exTex, objColor, 3.0f, 64.0f);
            }
        }
        
        // Cleanup and Restore standard shader states so the rest of the world remains normal
        ls.setBool("blendWithColor", false);
        ls.setBool("useTexture", false);
        ls.setInt("shadingMode", prevShadingMode);
    }

    drawCubeTextured(texCubeVAO, ls, I, C_PARKING, {0, .01f, 41}, {50, .05f, 28}, texAsphalt, 12.0f, 16, 0.9f);
    // Parking lines
    for (int i = 0; i < 9; i++)
    {
        float z = 28 + i * 3.f;
        drawCube(V, ls, I, C_PARK_LINE, {-15, .04f, z}, {.1f, .02f, 2.5f});
        drawCube(V, ls, I, C_PARK_LINE, {15, .04f, z}, {.1f, .02f, 2.5f});
    }
    drawCube(V, ls, I, C_PARK_LINE, {-4, .04f, 41}, {.1f, .02f, 26});
    drawCube(V, ls, I, C_PARK_LINE, {4, .04f, 41}, {.1f, .02f, 26});
    // Arrows
    drawCube(V, ls, I, C_ROAD_WHT, {0, .04f, 36}, {.15f, .02f, 2});
    drawCube(V, ls, I, C_ROAD_WHT, {-.5f, .04f, 37}, {1, .02f, .15f});
    
    // Boom barrier (animated rotation)
    drawCube(V, ls, I, C_LAMP, {3, .5f, 54}, {.2f, 1, .2f}); // post
    {
        // Pivot at post top, rotate the arm around Z-axis at pivot point
        glm::mat4 bm = glm::translate(I, {3.0f, 1.1f, 54.0f}); // move to pivot
        bm = glm::rotate(bm, glm::radians(-barrierAngle), {0, 0, 1}); // rotate
        bm = glm::translate(bm, {-3.0f, 0, 0}); // offset arm from pivot
        drawCube(V, ls, bm, C_BARRIER_R, {0, 0, 0}, {6, .12f, .12f});
    }
    // Parking lamp posts
    drawCube(V, ls, I, C_LAMP, {-10, 3, 42}, {.15f, 6, .15f});
    drawCube(V, ls, I, C_LAMP, {-10, 6.2f, 42}, {.8f, .15f, .6f});
    drawCube(V, ls, I, C_LAMP, {10, 3, 42}, {.15f, 6, .15f});
    drawCube(V, ls, I, C_LAMP, {10, 6.2f, 42}, {.8f, .15f, .6f});

    };
    drawParkingLot();

    auto drawMallExterior = [&]() {
    // MALL EXTERIOR (X:-25 to 25, Z:-25 to 25)

    // North wall - with brick texture for realism
    drawCubeTextured(texCubeVAO, ls, I, C_EXT, {0, WH, -25}, {50, MH, WT}, texBrick, 25.0f, 32, 0.9f);
    // South wall (gap X:-4 to 4 for entrance) - with brick texture
    drawCubeTextured(texCubeVAO, ls, I, C_EXT, {-14.5f, WH, 25}, {21, MH, WT}, texBrick, 12.0f, 32, 0.9f);
    drawCubeTextured(texCubeVAO, ls, I, C_EXT, {14.5f, WH, 25}, {21, MH, WT}, texBrick, 12.0f, 32, 0.9f);
    drawCubeTextured(texCubeVAO, ls, I, C_EXT, {0, MH - .5f, 25}, {8, 1, WT}, texBrick, 4.0f, 32, 0.9f); // strip above entrance
    // West wall - with brick texture
    drawCubeTextured(texCubeVAO, ls, I, C_EXT, {-25, WH, 0}, {WT, MH, 50}, texBrick, 25.0f, 32, 0.9f);
    // East wall - with brick texture
    drawCubeTextured(texCubeVAO, ls, I, C_EXT, {25, WH, 0}, {WT, MH, 50}, texBrick, 25.0f, 32, 0.9f);
    // Entrance awning
    drawCube(V, ls, I, C_AWNING, {0, MH + .1f, 26.5f}, {12, .2f, 3.5f});
    // Glass panels beside entrance
    drawCube(V, ls, I, C_GLASS, {-3.5f, WH - .5f, 25.01f}, {1.5f, MH - 1, .08f}, 128, .3f);
    drawCube(V, ls, I, C_GLASS, {3.5f, WH - .5f, 25.01f}, {1.5f, MH - 1, .08f}, 128, .3f);
    // GLASS ENTRANCE DOORS (sliding, press G to open/close)
    {
        float edX = 0.0f, edZ = 25.0f;
        float edHalfW = 3.8f; // total opening width
        float slideAmt = entranceDoorOffset * edHalfW * 0.5f; // how far each door slides
        // Left door panel (slides left when open)
        float ldX = edX - edHalfW * 0.25f - slideAmt;
        drawCube(V, ls, I, C_GLASS * 1.05f, {ldX, WH * 0.9f, edZ + 0.02f}, {edHalfW * 0.48f, MH * 0.85f, .1f}, 128, .2f);
        // Right door panel (slides right when open)
        float rdX = edX + edHalfW * 0.25f + slideAmt;
        drawCube(V, ls, I, C_GLASS * 1.05f, {rdX, WH * 0.9f, edZ + 0.02f}, {edHalfW * 0.48f, MH * 0.85f, .1f}, 128, .2f);
        // Door frame (metal surround)
        drawCube(V, ls, I, C_LAMP, {edX, MH - 0.1f, edZ + 0.02f}, {edHalfW + 0.4f, .2f, .15f}); // top bar
        drawCube(V, ls, I, C_LAMP, {edX - edHalfW * 0.5f - 0.1f, WH * 0.9f, edZ + 0.02f}, {.15f, MH * 0.85f, .15f}); // left post
        drawCube(V, ls, I, C_LAMP, {edX + edHalfW * 0.5f + 0.1f, WH * 0.9f, edZ + 0.02f}, {.15f, MH * 0.85f, .15f}); // right post
        // Door handle marks (faint)
        drawCube(V, ls, I, C_LAMP * 1.3f, {ldX + edHalfW * 0.15f, 1.2f, edZ + 0.08f}, {.04f, .35f, .04f}, 64);
        drawCube(V, ls, I, C_LAMP * 1.3f, {rdX - edHalfW * 0.15f, 1.2f, edZ + 0.08f}, {.04f, .35f, .04f}, 64);
        // Button panel beside entrance (outside)
        drawCube(V, ls, I, C_LAMP, {edX + edHalfW * 0.5f + 0.8f, 1.2f, edZ + 0.3f}, {.2f, .3f, .1f});
        drawCube(V, ls, I, {0.1f, 0.9f, 0.3f}, {edX + edHalfW * 0.5f + 0.8f, 1.2f, edZ + 0.36f}, {.1f, .1f, .03f}); // green button
    }
    // Windows on east/west
    for (int i = 0; i < 4; i++)
    {
        float wz = -20 + i * 12.f;
        drawCube(V, ls, I, C_GLASS, {-25.01f, 2.5f, wz}, {.08f, 2, 4}, 128, .3f);
        drawCube(V, ls, I, C_GLASS, {25.01f, 2.5f, wz}, {.08f, 2, 4}, 128, .3f);
    }

    };
    drawMallExterior();

    auto drawFactoryWarehouse = [&]() {
    // FACTORY FLOOR & CEILING (industrial look)
    ls.setBool("blendWithColor", true);
    drawCubeTextured(texCubeVAO, ls, I, C_FACTORY_FLOOR, {-12.375f, -.02f, 0}, {24.75f, .08f, 49.5f}, texIndustryFloor, 10.0f, 8, .95f);
    drawCubeTextured(texCubeVAO, ls, I, C_FACTORY_FLOOR, {12.375f, -.02f, 0}, {24.75f, .08f, 49.5f}, texConcrete, 10.0f, 8, .95f);
    ls.setBool("blendWithColor", false);
    drawCube(V, ls, I, C_CEILING * 0.9f, {0, MH, 0}, {49.5f, .15f, 49.5f});
    drawCube(V, ls, I, C_PARKING, {0, MH + .05f, 0}, {49.5f, .1f, 49.5f}, 8, .8f);

    // ================================================================
    // HUGE INDUSTRIAL WAREHOUSE - NESTED CONVEYOR SYSTEMS
    // Large open floor with multiple nested conveyor belts
    // Heavy support columns, cyan accent lighting
    // ================================================================

    // ---- INDUSTRIAL SUPPORT COLUMNS (massive warehouse pillars) ----
    {    
        glm::vec3 columnPos[] = {
            {-20, WH, 12}, {20, WH, 12},
            {-20, WH, -12}, {20, WH, -12},
            {-18, WH, 0}, {18, WH, 0}
        };
        for (int i = 0; i < 6; i++) {
            // Large cylindrical columns (approximated with tall boxes)
            drawCubeTextured(texCubeVAO, ls, I, C_DARK_METAL, columnPos[i], {1.2f, MH + 1.0f, 1.2f}, texConcrete, 2.0f, 16, 1.0f);
            // Reinforced base
            drawCubeTextured(texCubeVAO, ls, I, C_METAL, {columnPos[i].x, 0.2f, columnPos[i].z}, {1.6f, 0.4f, 1.6f}, texMetal, 2.0f, 64, 1.0f);
            // Top cap with cyan accent ring
            drawCube(V, ls, I, C_CYAN_ACCENT, {columnPos[i].x, MH + 0.95f, columnPos[i].z}, {1.3f, 0.15f, 1.3f});
            // Safety markings on columns (yellow-black stripes)
            for (int s = 0; s < 6; s++) {
                float sy = 0.6f + s * 0.7f;
                glm::vec3 stripeColor = (s % 2 == 0) ? glm::vec3(0.9f, 0.9f, 0.0f) : glm::vec3(0.1f, 0.1f, 0.1f);
                drawCube(V, ls, I, stripeColor, {columnPos[i].x, sy, columnPos[i].z}, {1.35f, 0.1f, 1.35f});
            }
        }
    }

    // ---- FLOOR MARKINGS & SAFETY ZONES ----
    {
        // Yellow safety lines around conveyor areas
        // Outer loop boundary markings
        glm::vec3 yellowLine(0.9f, 0.9f, 0.0f);
        
        // West perimeter
        drawCube(V, ls, I, yellowLine, {-20.8f, 0.02f, 0}, {0.2f, 0.01f, 37});
        drawCube(V, ls, I, yellowLine, {20.8f, 0.02f, 0}, {0.2f, 0.01f, 37});
        // North perimeter
        drawCube(V, ls, I, yellowLine, {0, 0.02f, 18.8f}, {41, 0.01f, 0.2f});
        // South perimeter
        drawCube(V, ls, I, yellowLine, {0, 0.02f, -18.8f}, {41, 0.01f, 0.2f});
        
        // Corner warning markers
        glm::vec3 cornerPos[] = {{-20, 0, 18}, {20, 0, 18}, {-20, 0, -18}, {20, 0, -18}};
        for (int c = 0; c < 4; c++) {
            drawCube(V, ls, I, glm::vec3(1.0f, 0.0f, 0.0f), cornerPos[c], {0.5f, 0.02f, 0.5f});
        }
    }

    // ---- MASSIVE NESTED CONVEYOR SYSTEMS (like the reference image) ----
    // Three nested rectangular conveyor loops at different levels
    {    
        // OUTER CONVEYOR LOOP (largest, X: -20 to 20, Z: -18 to 18)
        {
            float outerY = 0.15f;
            
            // West side belt (X = -20, Z: -18 to 18)
            drawCube(V, ls, I, C_CONVEYOR, {-20, outerY, 0}, {0.6f, 0.2f, 36});
            drawCube(V, ls, I, C_CYAN_ACCENT, {-20, outerY + 0.01f, fmod(conveyorOffset * 2.0f, 36.0f) - 18.0f}, {0.59f, 0.01f, 2.0f});
            
            // East side belt (X = 20, Z: -18 to 18)
            drawCube(V, ls, I, C_CONVEYOR, {20, outerY, 0}, {0.6f, 0.2f, 36});
            drawCube(V, ls, I, C_CYAN_ACCENT, {20, outerY + 0.01f, fmod(conveyorOffset * 2.0f, 36.0f) - 18.0f}, {0.59f, 0.01f, 2.0f});
            
            // North side belt (Z = 18, X: -20 to 20)
            drawCube(V, ls, I, C_CONVEYOR, {0, outerY, 18}, {40, 0.2f, 0.6f});
            drawCube(V, ls, I, C_CYAN_ACCENT, {fmod(conveyorOffset * 2.0f, 40.0f) - 20.0f, outerY + 0.01f, 18}, {2.0f, 0.01f, 0.59f});
            
            // South side belt (Z = -18, X: -20 to 20)
            drawCube(V, ls, I, C_CONVEYOR, {0, outerY, -18}, {40, 0.2f, 0.6f});
            drawCube(V, ls, I, C_CYAN_ACCENT, {fmod(conveyorOffset * 2.0f, 40.0f) - 20.0f, outerY + 0.01f, -18}, {2.0f, 0.01f, 0.59f});
        }
        
        // MIDDLE CONVEYOR LOOP (X: -12 to 12, Z: -12 to 12, higher level)
        {
            float midY = 0.50f;
            
            // West side
            drawCube(V, ls, I, C_CONVEYOR, {-12, midY, 0}, {0.5f, 0.2f, 24});
            drawCube(V, ls, I, C_CYAN_ACCENT, {-12, midY + 0.01f, fmod(conveyorOffset * 2.3f, 24.0f) - 12.0f}, {0.48f, 0.01f, 1.5f});
            
            // East side
            drawCube(V, ls, I, C_CONVEYOR, {12, midY, 0}, {0.5f, 0.2f, 24});
            drawCube(V, ls, I, C_CYAN_ACCENT, {12, midY + 0.01f, fmod(conveyorOffset * 2.3f, 24.0f) - 12.0f}, {0.48f, 0.01f, 1.5f});
            
            // North side
            drawCube(V, ls, I, C_CONVEYOR, {0, midY, 12}, {24, 0.2f, 0.5f});
            drawCube(V, ls, I, C_CYAN_ACCENT, {fmod(conveyorOffset * 2.3f, 24.0f) - 12.0f, midY + 0.01f, 12}, {1.5f, 0.01f, 0.48f});
            
            // South side
            drawCube(V, ls, I, C_CONVEYOR, {0, midY, -12}, {24, 0.2f, 0.5f});
            drawCube(V, ls, I, C_CYAN_ACCENT, {fmod(conveyorOffset * 2.3f, 24.0f) - 12.0f, midY + 0.01f, -12}, {1.5f, 0.01f, 0.48f});
        }
        
        // INNER CONVEYOR LOOP (X: -6 to 6, Z: -6 to 6, highest level)
        {
            float innerY = 0.85f;
            
            // West side
            drawCube(V, ls, I, C_CONVEYOR, {-6, innerY, 0}, {0.4f, 0.2f, 12});
            drawCube(V, ls, I, C_CYAN_ACCENT, {-6, innerY + 0.01f, fmod(conveyorOffset * 2.5f, 12.0f) - 6.0f}, {0.38f, 0.01f, 1.0f});
            
            // East side
            drawCube(V, ls, I, C_CONVEYOR, {6, innerY, 0}, {0.4f, 0.2f, 12});
            drawCube(V, ls, I, C_CYAN_ACCENT, {6, innerY + 0.01f, fmod(conveyorOffset * 2.5f, 12.0f) - 6.0f}, {0.38f, 0.01f, 1.0f});
            
            // North side
            drawCube(V, ls, I, C_CONVEYOR, {0, innerY, 6}, {12, 0.2f, 0.4f});
            drawCube(V, ls, I, C_CYAN_ACCENT, {fmod(conveyorOffset * 2.5f, 12.0f) - 6.0f, innerY + 0.01f, 6}, {1.0f, 0.01f, 0.38f});
            
            // South side
            drawCube(V, ls, I, C_CONVEYOR, {0, innerY, -6}, {12, 0.2f, 0.4f});
            drawCube(V, ls, I, C_CYAN_ACCENT, {fmod(conveyorOffset * 2.5f, 12.0f) - 6.0f, innerY + 0.01f, -6}, {1.0f, 0.01f, 0.38f});
        }
    }

    // ---- REALISTIC BOX HANDLING (Individual boxes with tracking on conveyor path) ----
    {
        // Draw all active boxes on their realistic conveyor positions
        for (int i = 0; i < MAX_BOXES; i++) {
            if (boxes[i].active) {
                float pos = boxes[i].position;
                float bx = 0, bz = 0, by = 1.32f;  // Y correctly tuned to rest exactly ON the visually animated moving belt
                bool drawn = false;
                bool takesLiftBranch = (i % 3 == 0);

                // ===== MAIN CONVEYOR PATH (Input → Stamping → Color → Welding) =====
                if (pos >= -25.0f && pos <= -5.0f) {
                    bx = -24.0f + (pos + 25.0f);  // Moves purely along X
                    bz = 16.0f;  
                    by = 1.32f; // Perfectly sitting on elevated structure
                    drawn = true;
                }
                // ===== LIFT BRANCH PATH =====
                else if (takesLiftBranch && pos > -5.0f) {
                    if (pos <= 8.0f) {
                        bx = -4.0f + (pos + 5.0f); 
                        bz = 16.0f;
                        by = 1.32f;
                        drawn = true;
                    } else if (pos <= 10.0f) {
                        bx = 9.0f;
                        bz = 16.0f;
                        by = 1.32f;
                        drawn = true;
                    } else if (pos <= 40.0f) {
                        bx = 9.0f;
                        bz = 16.0f - (pos - 10.0f);
                        by = 1.32f;
                        if (pos >= 27.0f && pos <= 29.5f) {
                            by += factoryLiftY; // moves with lift height
                        } else if (pos > 29.5f) {
                            by += 3.0f; // flat upper floor level
                        }
                        drawn = true;
                    } else {
                        drawn = false;
                    }
                }
                // ===== TRANSITION CONVEYOR CHUTE (Drop down) =====
                else if (!takesLiftBranch && pos > -5.0f && pos <= 0.0f) {
                    float transitionFactor = (pos - (-5.0f)) / 5.0f;
                    bx = glm::mix(-4.0f, -18.0f, transitionFactor);
                    bz = glm::mix(16.0f, -6.0f, transitionFactor);
                    by = glm::mix(1.32f, 0.525f, transitionFactor); // Sloping down
                    drawn = true;
                }
                // ===== INSPECTION CONVEYOR (Moving Right towards Box Machine) =====
                else if (pos > 0.0f && pos <= 6.0f) {
                    float inspectionPos = pos / 6.0f;
                    bx = -18.0f + inspectionPos * 6.0f;  // -18 to -12
                    bz = -6.0f;
                    by = 0.525f; // perfectly flat on lower conveyor
                    drawn = true;
                }
                // ===== BOXING MACHINE CONVEYOR (Passing through box plant) =====
                else if (pos > 6.0f && pos <= 12.0f) {
                    float boxingPos = (pos - 6.0f) / 6.0f;
                    bx = -12.0f + boxingPos * 6.0f;  // -12 to -6
                    bz = -6.0f;
                    by = 0.525f; // perfectly flat on lower conveyor
                    drawn = true;
                }
                // At pos > 12.0f, the individual loose box is conceptually swept away by the robot or vanishes logically.
                // We stop drawing the single floating unit because the palletizer renders stacks statically.
                else {
                    drawn = false;
                }

                // Draw box only if positioned within valid area and marked for drawing.
                if (drawn && bx >= -25 && bx <= 25 && bz >= -20 && bz <= 20) {
                    glm::mat4 boxModel = glm::translate(I, {bx, by, bz});
                    
                    // Realistic Box Behavior: Sit flat on conveyor surfaces! 
                    // No more chaotic spinning in the air. 
                    // Add tiny vibration when inside the box packing sequence only.
                    if (pos > 6.0f && pos <= 12.0f && !takesLiftBranch) {
                        boxModel = glm::rotate(boxModel, glm::radians(sin(globalTime * 15.0f) * 4.0f), {0, 1, 0});
                    }
                    else if (takesLiftBranch && pos > 8.0f && pos <= 10.0f) {
                        boxModel = glm::rotate(boxModel, glm::radians(globalTime * 90.0f), {0, 1, 0});
                    }
                    
                    // Box should be perfectly SQUARE (0.55 on all axes)
                    glm::vec3 boxScale = {0.55f, 0.55f, 0.55f};
                    
                    // Draw the box with its workflow-stage color and texture
                    if (boxes[i].textureID != 0) {
                        drawCubeTextured(texCubeVAO, ls, boxModel, boxes[i].color, {0, 0, 0}, boxScale, boxes[i].textureID, 1.0f, 64, 1.0f);
                    } else {
                        drawCube(V, ls, boxModel, boxes[i].color, {0, 0, 0}, boxScale, 64, 1.0f);
                    }
                }
            }
        }
    }

    // ---- REALISTIC MULTI-STAGE PACKAGING WORKFLOW ----
    {
        // STAGE 1: QUALITY INSPECTION AREA (X: -18, Z: -6)
        {
            float inspectX = -18.0f;
            float inspectZ = -6.0f;
            
            // Inspection conveyor belt
            drawCube(V, ls, I, C_CONVEYOR, {inspectX, 0.15f, inspectZ}, {6.0f, 0.2f, 1.5f});
            drawCube(V, ls, I, C_CYAN_ACCENT, {inspectX + fmod(conveyorOffset * 2.5f, 6.0f) - 3.0f, 0.16f, inspectZ}, {0.4f, 0.01f, 1.4f});
            
            // Inspection gantry system
            glm::mat4 inspectGantry = glm::translate(I, {inspectX, 1.2f, inspectZ});
            drawCube(V, ls, inspectGantry, C_DARK_METAL, {0, 0, 0}, {6.5f, 0.3f, 0.3f});
            
            // Automated vision camera (moving probe for quality check)
            float cameraPos = sin(globalTime * 1.5f) * 2.5f;
            drawCube(V, ls, I, C_CYAN_ACCENT, {inspectX + cameraPos, 1.35f, inspectZ - 1.8f}, {0.25f, 0.15f, 0.15f});
            
            // Quality indicator lights
            float qualityPass = sin(globalTime * 2.0f) * 0.5f + 0.5f;
            drawCube(V, ls, I, glm::vec3(0.1f, qualityPass, 0.1f), {inspectX + 2.5f, 1.8f, inspectZ + 0.9f}, {0.15f, 0.08f, 0.08f});
        }
        
        // STAGE 2: AUTOMATED BOXING MACHINE (X: -12, Z: -6)
        {
            float boxingX = -12.0f;
            float boxingZ = -6.0f;
            
            // Large boxing machine (carton erector & filler)
            drawCube(V, ls, I, C_MACHINE, {boxingX, 0.8f, boxingZ}, {3.2f, 1.8f, 2.8f});
            // Machine window
            drawCube(V, ls, I, C_GLASS, {boxingX, 1.0f, boxingZ - 1.5f}, {3.0f, 1.6f, 0.1f}, 128, 0.3f);
            
            // Input conveyor into boxing machine
            drawCube(V, ls, I, C_CONVEYOR, {boxingX - 3.0f, 0.15f, boxingZ}, {2.5f, 0.2f, 1.5f});
            
            // Items being boxed - realistic filling cycle showing boxes inside machine
            float fillingCycle = fmod(globalTime * 0.6f, 1.0f);
            for (int boxIdx = 0; boxIdx < 2; boxIdx++) {
                float inMachinePhase = fmod(fillingCycle * 2.0f + boxIdx, 2.0f) / 2.0f;
                
                float itemY;
                if (inMachinePhase < 0.3f) {
                    // Item dropping into box
                    itemY = 0.7f - (inMachinePhase / 0.3f) * 0.4f;
                } else if (inMachinePhase < 0.7f) {
                    // In box being packed
                    itemY = 0.3f + sin((inMachinePhase - 0.3f) / 0.4f * 3.14159f) * 0.1f;
                } else {
                    // Box complete, rising to exit
                    itemY = 0.3f + ((inMachinePhase - 0.7f) / 0.3f) * 0.5f;
                }
                
                drawCube(V, ls, I, glm::vec3(0.25f, 0.95f, 0.25f), {boxingX, itemY, boxingZ}, {0.50f, 0.45f, 0.50f}, 64, 0.9f);
            }
            
            // Output conveyor (packed boxes leaving)
            drawCube(V, ls, I, C_CONVEYOR, {boxingX + 3.0f, 0.15f, boxingZ}, {2.5f, 0.2f, 1.5f});
            drawCube(V, ls, I, C_CYAN_ACCENT, {boxingX + 3.0f + fmod(conveyorOffset * 2.0f, 2.5f) - 1.25f, 0.16f, boxingZ}, {0.3f, 0.01f, 1.4f});
        }
        
        // STAGE 3: PALLETIZING ROBOT STATION (X: -4 to 0, Z: -6 to -4)
        {
            float paletizeX = -2.0f;
            float paletizeZ = -5.0f;
            
            // Robot mounting base
            drawCube(V, ls, I, C_METAL, {paletizeX, 0.5f, paletizeZ}, {1.8f, 0.8f, 1.8f});
            
            // Palletizing robotic arm (pick -> transfer -> place -> return)
            float armRotPal = 0.0f;
            float armReach = 0.55f;
            float armY = 1.1f;
            float t = palletizerCycle;
            if (t < 0.18f) {
                float p = t / 0.18f; // move to pickup
                armRotPal = glm::mix(20.0f, 62.0f, p);
                armReach = glm::mix(0.45f, 0.88f, p);
                armY = glm::mix(1.15f, 0.95f, p);
            } else if (t < 0.38f) {
                float p = (t - 0.18f) / 0.20f; // lift after pick
                armRotPal = 62.0f;
                armReach = 0.88f;
                armY = glm::mix(0.95f, 1.35f, p);
            } else if (t < 0.68f) {
                float p = (t - 0.38f) / 0.30f; // transfer toward pallet
                armRotPal = glm::mix(62.0f, -18.0f, p);
                armReach = glm::mix(0.88f, 0.68f, p);
                armY = 1.35f;
            } else if (t < 0.80f) {
                float p = (t - 0.68f) / 0.12f; // place down
                armRotPal = -18.0f;
                armReach = 0.68f;
                armY = glm::mix(1.35f, 0.98f, p);
            } else {
                float p = (t - 0.80f) / 0.20f; // return home
                armRotPal = glm::mix(-18.0f, 20.0f, p);
                armReach = glm::mix(0.68f, 0.55f, p);
                armY = glm::mix(0.98f, 1.1f, p);
            }

            glm::mat4 palArm = glm::translate(I, {paletizeX, 1.2f, paletizeZ});
            palArm = glm::rotate(palArm, glm::radians(armRotPal), {0, 1, 0});
            
            // Upper arm segment
            drawCube(V, ls, palArm, C_MACHINE, {armReach, armY - 1.2f, 0}, {0.35f, 0.18f, 1.0f});
            
            // Joint bearing
            drawCube(V, ls, palArm, C_CYAN_ACCENT, {0, 0.05f, 0}, {0.4f, 0.15f, 0.4f});
            
            // Suction cup gripper
            glm::mat4 suction = glm::translate(palArm, {armReach + 0.15f, armY - 1.35f, 0});
            float gripperActive = (t > 0.18f && t < 0.72f) ? 1.0f : 0.35f;
            drawCube(V, ls, suction, C_CYAN_ACCENT * (0.7f + gripperActive * 0.3f), {0, 0, 0}, {0.28f, 0.12f, 0.28f});
            
            // Wooden pallet base
            drawCube(V, ls, I, C_DARK_METAL, {paletizeX + 1.8f, 0.08f, paletizeZ}, {1.4f, 0.05f, 1.4f});
            
            // Real stacked pattern row-by-row from palletizer output
            for (int idx = 0; idx < palletizerStackCount; ++idx) {
                int layer = idx / 4;
                int rem = idx % 4;
                int row = rem / 2;
                int col = rem % 2;

                float sx = paletizeX + 1.8f - 0.40f + col * 0.80f;
                float sy = 0.40f + layer * 0.58f;
                float sz = paletizeZ - 0.40f + row * 0.80f;
                drawCube(V, ls, I, glm::vec3(0.92f, 0.92f, 0.90f), {sx, sy, sz}, {0.50f, 0.55f, 0.50f}, 64, 0.95f);
            }

            // Picked box visual (held by gripper while transferring)
            if (palletizerCurrentSource >= 0 && t >= 0.18f && t < 0.80f) {
                drawCube(V, ls, suction, glm::vec3(0.92f, 0.92f, 0.90f), {0, -0.22f, 0}, {0.50f, 0.50f, 0.50f}, 64, 0.95f);
            }
        }
        
        // STAGE 4: STRETCH WRAPPING STATION (X: 15, Z: -6)
        {
            float wrapX = 15.0f;
            float wrapZ = -6.0f;
            
            // Realistic machine cycle based on general packaging run
            float wrappingCycle = fmod(globalTime * 0.4f, 1.0f);
            float wrapRotation = wrappingCycle * 2160.0f; // 6 rotations per cycle
            float filmHeight = 0.8f + sin(wrappingCycle * 3.14159f) * 1.3f; // Moves up and down
            
            glm::mat4 wrapperModel = glm::translate(I, {wrapX, 0.0f, wrapZ});
            
            // Draw Advanced Wrapping Machine
            ComplexMachine::drawAdvancedWrappingMachine(V, ls, wrapperModel, wrapRotation, filmHeight, globalTime);
            
            // Input conveyor to wrapper
            drawCube(V, ls, I, C_CONVEYOR, {wrapX - 3.0f, 0.15f, wrapZ}, {2.5f, 0.2f, 1.5f});
            
            // Pallet rotating inside wrapper
            glm::mat4 wrappingPallet = glm::translate(I, {wrapX, 0.25f, wrapZ});
            wrappingPallet = glm::rotate(wrappingPallet, glm::radians(wrapRotation), {0, 1, 0});
            
            // Wooden pallet base
            drawCube(V, ls, wrappingPallet, C_DARK_METAL, {0, 0, 0}, {1.4f, 0.1f, 1.4f});
            
            // Complex structure of stacked objects being wrapped
            for (int layer = 0; layer < 4; layer++) {
                for(int r = 0; r < 2; r++) {
                    for(int c = 0; c < 2; c++) {
                        float bx = -0.35f + c * 0.7f;
                        float bz = -0.35f + r * 0.7f;
                        float by = 0.35f + layer * 0.55f;
                        // Use slightly different colors to look like varied product
                        glm::vec3 boxColor = glm::mix(glm::vec3(0.92f, 0.92f, 0.9f), glm::vec3(0.85f, 0.88f, 0.85f), (r+c)*0.5f);
                        drawCube(V, ls, wrappingPallet, boxColor, {bx, by, bz}, {0.68f, 0.53f, 0.68f}, 64, 1.0f);
                    }
                }
            }
            
            // Active film stretching from carriage to the load
            glm::mat4 activeFilm = glm::translate(I, {wrapX - 0.7f, filmHeight, wrapZ});
            activeFilm = glm::rotate(activeFilm, glm::radians(wrapRotation), {0, 1, 0});
            drawCube(V, ls, activeFilm, glm::vec3(0.85f, 0.93f, 0.98f), {0.5f, 0, 0.5f}, {1.0f, 0.58f, 0.02f}, 128, 0.3f);

            // Plastic wrap buildup effect globally on the load
            float buildupAlpha = wrappingCycle * 0.45f; // Gets thicker over time
            drawCube(V, ls, wrappingPallet, glm::vec3(0.85f, 0.95f, 1.0f), 
                     {0, 1.15f, 0}, {1.45f, 2.2f, 1.45f}, 128, buildupAlpha);
            
            // Output conveyor (wrapped pallets leaving)
            drawCube(V, ls, I, C_CONVEYOR, {wrapX + 3.0f, 0.15f, wrapZ}, {2.5f, 0.2f, 1.5f});
        }
        
        // STAGE 5: FINISHED GOODS STAGING AREA (X: 18-22, Z: -6 to -4)
        {
            // Staging floor
            drawCube(V, ls, I, C_FACTORY_FLOOR * 1.15f, {20, 0.02f, -5}, {6.0f, 0.04f, 3.5f});
            
            // Wall shelving for finished goods
            drawCube(V, ls, I, C_DARK_METAL, {23.5f, 0.5f, -5}, {0.3f, 2.5f, 3.5f});  // Vertical rack
            drawCube(V, ls, I, C_METAL, {23.5f, 0.5f, -5}, {1.5f, 0.08f, 3.5f});      // Base shelf
            drawCube(V, ls, I, C_METAL, {23.5f, 1.3f, -5}, {1.5f, 0.08f, 3.5f});      // Middle shelf
            drawCube(V, ls, I, C_METAL, {23.5f, 2.1f, -5}, {1.5f, 0.08f, 3.5f});      // Top shelf
            
            // Wrapped and ready-to-ship pallets stacked with realistic appearance
            int remainingGathered = gatheredBoxes;
            for (int pIdx = 0; pIdx < 3; pIdx++) {
                float px = 18.5f + pIdx * 1.5f;
                float pz = -5.0f;
                
                // Pallet base
                drawCube(V, ls, I, C_DARK_METAL, {px, 0.08f, pz}, {1.3f, 0.05f, 1.3f});
                
                // Wrapped boxes stacked on pallet based on actual gathered inventory
                int boxesOnPallet = glm::clamp(remainingGathered, 0, 8);
                remainingGathered -= boxesOnPallet;
                for (int idx = 0; idx < boxesOnPallet; idx++) {
                    int layer = idx / 4;
                    int rem = idx % 4;
                    int row = rem / 2;
                    int col = rem % 2;

                    float bx = px - 0.35f + col * 0.70f;
                    float by = 0.40f + layer * 0.58f;
                    float bz = pz - 0.35f + row * 0.70f;

                    drawCube(V, ls, I, glm::vec3(0.92f, 0.92f, 0.90f), {bx, by, bz}, {0.50f, 0.55f, 0.50f}, 128, 0.92f);
                }
                
                // Plastic wrap effect on finished pallet
                float wrapGlow = sin(globalTime * 1.5f + pIdx * 0.6f) * 0.15f + 0.3f;
                drawCube(V, ls, I, glm::vec3(0.93f, 0.96f, 1.0f) * wrapGlow, 
                         {px, 1.0f, pz}, {1.4f, 1.8f, 1.4f}, 64, 0.08f);
            }
        }
    }

    // ---- ROBOTIC ARMS AT INTERSECTIONS (processing stations) ----
    {
        // North intersection arm (X=0, Z=12)
        {
            glm::mat4 armN = glm::translate(I, {0, 1.2f, 12});
            
            // Task timing: pick, move, place, return
            float taskPhaseN = fmod(globalTime, 5.0f) / 5.0f;
            float rotN, armExtensionN, heightN;
            
            if (taskPhaseN < 0.25f) {
                // Pick phase: rotate to input position
                float pickTime = taskPhaseN / 0.25f;
                rotN = -45.0f + pickTime * 45.0f;
                armExtensionN = 0.5f + sin(pickTime * 3.14159f) * 0.2f;
                heightN = 0.8f;
            } else if (taskPhaseN < 0.5f) {
                // Move phase: rotate across
                float moveTime = (taskPhaseN - 0.25f) / 0.25f;
                rotN = 0.0f + moveTime * 90.0f;
                armExtensionN = 0.7f;
                heightN = 0.8f + sin(moveTime * 3.14159f) * 0.3f;
            } else if (taskPhaseN < 0.75f) {
                // Place phase: rotate to output
                float placeTime = (taskPhaseN - 0.5f) / 0.25f;
                rotN = 90.0f + placeTime * 45.0f;
                armExtensionN = 0.7f - sin(placeTime * 3.14159f) * 0.2f;
                heightN = 0.8f + sin(placeTime * 3.14159f) * 0.3f;
            } else {
                // Return phase
                float returnTime = (taskPhaseN - 0.75f) / 0.25f;
                rotN = 135.0f - returnTime * 135.0f;
                armExtensionN = 0.5f;
                heightN = 0.8f;
            }
            
            drawCube(V, ls, I, C_METAL, {0, 0.5f, 12}, {1.5f, 0.8f, 1.5f}); // base
            
            glm::mat4 armNRot = glm::translate(armN, {0, 0.4f, 0});
            armNRot = glm::rotate(armNRot, glm::radians(rotN), {0, 1, 0});
            
            glm::mat4 armSegN = glm::translate(armNRot, {armExtensionN * 0.5f, heightN - 0.6f, 0});
            armSegN = glm::translate(armSegN, {0, 0, 0}); // nested transform
            drawCube(V, ls, armSegN, C_MACHINE, {0, 0, 0}, {0.3f, 0.15f, 0.8f});
            
            glm::mat4 gripN = glm::translate(armSegN, {0, -0.15f, 0.3f});
            drawCube(V, ls, gripN, C_CYAN_ACCENT, {0, 0, 0}, {0.2f, 0.12f, 0.4f});
        }
        
        // South intersection arm (X=0, Z=-12)
        {
            glm::mat4 armS = glm::translate(I, {0, 1.2f, -12});
            
            // Task timing: offset by 2.5 seconds (half cycle)
            float taskPhaseS = fmod(globalTime + 2.5f, 5.0f) / 5.0f;
            float rotS, armExtensionS, heightS;
            
            if (taskPhaseS < 0.25f) {
                float pickTime = taskPhaseS / 0.25f;
                rotS = 45.0f - pickTime * 45.0f;
                armExtensionS = 0.5f + sin(pickTime * 3.14159f) * 0.2f;
                heightS = 0.8f;
            } else if (taskPhaseS < 0.5f) {
                float moveTime = (taskPhaseS - 0.25f) / 0.25f;
                rotS = 0.0f - moveTime * 90.0f;
                armExtensionS = 0.7f;
                heightS = 0.8f + sin(moveTime * 3.14159f) * 0.3f;
            } else if (taskPhaseS < 0.75f) {
                float placeTime = (taskPhaseS - 0.5f) / 0.25f;
                rotS = -90.0f - placeTime * 45.0f;
                armExtensionS = 0.7f - sin(placeTime * 3.14159f) * 0.2f;
                heightS = 0.8f + sin(placeTime * 3.14159f) * 0.3f;
            } else {
                float returnTime = (taskPhaseS - 0.75f) / 0.25f;
                rotS = -135.0f + returnTime * 135.0f;
                armExtensionS = 0.5f;
                heightS = 0.8f;
            }
            
            drawCube(V, ls, I, C_METAL, {0, 0.5f, -12}, {1.5f, 0.8f, 1.5f}); // base
            
            glm::mat4 armSRot = glm::translate(armS, {0, 0.4f, 0});
            armSRot = glm::rotate(armSRot, glm::radians(rotS), {0, 1, 0});
            
            glm::mat4 armSegS = glm::translate(armSRot, {armExtensionS * 0.5f, heightS - 0.6f, 0});
            drawCube(V, ls, armSegS, C_MACHINE, {0, 0, 0}, {0.3f, 0.15f, 0.8f});
            
            glm::mat4 gripS = glm::translate(armSegS, {0, -0.15f, 0.3f});
            drawCube(V, ls, gripS, C_CYAN_ACCENT, {0, 0, 0}, {0.2f, 0.12f, 0.4f});
        }
    }
    
    // ---- OVERHEAD LIGHTING RIGS (cyan accent) ----
    {
        // Main crossbeams with integrated lighting
        drawCube(V, ls, I, C_DARK_METAL, {0, MH - 0.3f, 0}, {40, 0.15f, 1.0f});
        drawCube(V, ls, I, C_DARK_METAL, {0, MH - 0.3f, 0}, {1.0f, 0.15f, 36});
        
        // Secondary support beams (adds realism)
        for (int b = 0; b < 4; b++) {
            float bx = -12 + b * 8.0f;
            drawCube(V, ls, I, C_DARK_METAL, {bx, MH - 0.25f, 0}, {0.3f, 0.1f, 36});
        }
        
        // Pulsing cyan lights along crossbeams
        for (int i = 0; i < 8; i++) {
            float lightBright = sin(globalTime * 2.0f + i * 0.4f) * 0.5f + 0.5f;
            drawCube(V, ls, I, C_CYAN_ACCENT * (0.3f + lightBright * 0.7f), 
                     {-16.0f + i * 4.5f, MH - 0.2f, 0}, {0.3f, 0.08f, 0.3f});
        }
        
        // Additional industrial lighting elements (hanging lights)
        for (int ly = 0; ly < 3; ly++) {
            float lx = -12 + ly * 12.0f;
            for (int lz = 0; lz < 3; lz++) {
                float lz_pos = -12 + lz * 12.0f;
                float lightIntensity = sin(globalTime * 1.5f + ly * 0.3f + lz * 0.2f) * 0.3f + 0.7f;
                drawCube(V, ls, I, glm::vec3(0.9f, 0.9f, 0.85f) * lightIntensity, 
                         {lx, MH - 0.5f, lz_pos}, {0.15f, 0.15f, 0.15f});
            }
        }
    }

    // ======== INDUSTRIAL MANUFACTURING ZONE (X:-24.5 to -4, Z:8 to 24) — Southwest ========
    {
        // Complete industrial manufacturing facility smoothly mapping industry floor texture
        drawCubeTextured(texCubeVAO, ls, I, C_FACTORY_FLOOR, {-14, .03f, 16}, {21, .04f, 16}, texIndustryFloor, 6.0f, 16, 1.0f);
        drawCubeTextured(texCubeVAO, ls, I, C_MACHINE * 0.8f, {-24.5f, WH, 16}, {WT, MH, 16}, texBrick, 8.0f, 32, 1.0f); // brick industrial wall
        
        // ===== STAMPING PRESS STATION (Left side) =====
        {
            glm::mat4 pressModel = glm::translate(I, {-22, 0.85f, 10});
            ComplexMachine::drawAdvancedStampingPress(V, ls, pressModel, C_METAL, machineState.pressStroke, globalTime);
        }
        
    // ===== MAIN ASSEMBLY CONVEYOR =====
        {
            float mainConvY = 1.0f; // High from the ground
            
            // Conveyor frame (heavy steel) - lowered slightly so belt is exactly on top
            drawCubeTextured(texCubeVAO, ls, I, C_METAL * 0.85f, {-14, mainConvY - 0.08f, 16}, {20.2f, 0.12f, 5.2f}, texMetal, 16.0f, 32, 0.8f);
            
            // Primary animated moving conveyor belt.
            drawCubeTextured(texCubeVAO, ls, I, C_CONVEYOR, {-14, mainConvY + 0.02f, 16}, {20.0f, 0.05f, 5.0f}, texMetal, 16.0f, 16, 1.0f);
            
            // Visible moving belt segments (thick yellow/black safety stripes so movement is extremely obvious)
            float beltAnimX = fmod(conveyorOffset * 2.0f, 3.0f);
            for(int s = 0; s < 7; s++) {
                float segmentX = -24.0f + fmod(beltAnimX + s * 3.0f, 20.0f);
                drawCube(V, ls, I, glm::vec3(0.9f, 0.8f, 0.1f), {segmentX, mainConvY + 0.046f, 16}, {0.3f, 0.01f, 4.8f}, 16, 1.0f);
                drawCube(V, ls, I, glm::vec3(0.1f, 0.1f, 0.1f), {segmentX - 0.3f, mainConvY + 0.046f, 16}, {0.3f, 0.01f, 4.8f}, 16, 1.0f);
            }

            // Outer guiding rails for objects
            drawCube(V, ls, I, C_DARK_METAL, {-14, mainConvY + 0.15f, 13.6f}, {20.0f, 0.15f, 0.1f});
            drawCube(V, ls, I, C_DARK_METAL, {-14, mainConvY + 0.15f, 18.4f}, {20.0f, 0.15f, 0.1f});

            // Cyan stripe animation with enhanced brightness along the center
            float stripeZ = 16 + fmod(conveyorOffset * 2.2f, 5.0f) - 2.5f;
            glm::vec3 enhancedCyan = C_CYAN_ACCENT * 1.2f;
            drawCube(V, ls, I, enhancedCyan, {-14, mainConvY + 0.045f, stripeZ}, {19.5f, 0.01f, 0.35f});
            
            // Support legs - with concrete texture
            drawCubeTextured(texCubeVAO, ls, I, C_DARK_METAL, {-23, mainConvY * 0.5f, 14}, {.3f, mainConvY, .3f}, texConcrete, 4.0f, 32, 0.85f);
            drawCubeTextured(texCubeVAO, ls, I, C_DARK_METAL, {-23, mainConvY * 0.5f, 18}, {.3f, mainConvY, .3f}, texConcrete, 4.0f, 32, 0.85f);
            drawCubeTextured(texCubeVAO, ls, I, C_DARK_METAL, {-5, mainConvY * 0.5f, 14}, {.3f, mainConvY, .3f}, texConcrete, 4.0f, 32, 0.85f);
            drawCubeTextured(texCubeVAO, ls, I, C_DARK_METAL, {-5, mainConvY * 0.5f, 18}, {.3f, mainConvY, .3f}, texConcrete, 4.0f, 32, 0.85f);
            
            // Drive motor at one end directly attached beneath
            float rollerRot = fmod(conveyorOffset * 200.0f, 360.0f);
            drawCube(V, ls, I, C_MACHINE, {-23.5f, mainConvY - 0.25f, 16}, {.6f, .4f, .6f});
            glm::mat4 coupling = glm::translate(I, {-23.5f, mainConvY - 0.08f, 16});
            coupling = glm::rotate(coupling, glm::radians(rollerRot * 0.5f), {1, 0, 0});
            drawCube(V, ls, coupling, C_METAL, {0, 0, 0}, {.25f, .15f, .25f});
        }

        // ===== LIFT BRANCH CONVEYOR & BELT ROTATOR =====
        {
            float branchY = 1.0f; 
            
            // Belt 1: X=-4 to X=8.4. Center = 2.2, Length = 12.4
            drawCube(V, ls, I, C_METAL * 0.85f, {2.2f, branchY - 0.08f, 16}, {12.4f, 0.12f, 1.5f});
            drawCube(V, ls, I, C_CONVEYOR, {2.2f, branchY + 0.02f, 16}, {12.4f, 0.05f, 1.3f});
            
            float beltAnimX = fmod(conveyorOffset * 2.0f, 3.0f);
            for(int s = 0; s < 4; s++) {
                float segmentX = -4.0f + fmod(beltAnimX + s * 3.0f, 12.4f);
                drawCube(V, ls, I, glm::vec3(0.9f, 0.8f, 0.1f), {segmentX, branchY + 0.046f, 16}, {0.3f, 0.01f, 1.2f});
            }
            
            // Belt 2: X=9, Z=15.4 to Z=-2.0. Center = 6.7, Length = 17.4
            drawCube(V, ls, I, C_METAL * 0.85f, {9.0f, branchY - 0.08f, 6.7f}, {1.5f, 0.12f, 17.4f});
            drawCube(V, ls, I, C_CONVEYOR, {9.0f, branchY + 0.02f, 6.7f}, {1.3f, 0.05f, 17.4f});
            
            for(int s = 0; s < 6; s++) {
                float segmentZ = 15.4f - fmod(beltAnimX + s * 3.0f, 17.4f);
                drawCube(V, ls, I, glm::vec3(0.9f, 0.8f, 0.1f), {9.0f, branchY + 0.046f, segmentZ}, {1.2f, 0.01f, 0.3f});
            }
            
            // Support legs
            drawCube(V, ls, I, C_DARK_METAL, {2.2f, branchY * 0.5f, 16}, {0.3f, branchY, 0.3f});
            drawCube(V, ls, I, C_DARK_METAL, {9.0f, branchY * 0.5f, 16}, {0.4f, branchY, 0.4f});
            drawCube(V, ls, I, C_DARK_METAL, {9.0f, branchY * 0.5f, 6.7f}, {0.3f, branchY, 0.3f});
            
            // Visible Belt Rotator / Turntable at X=9, Z=16
            float turnRot = globalTime * 90.0f; 
            glm::mat4 turntable = glm::translate(I, {9.0f, branchY - 0.08f, 16.0f});
            drawCube(V, ls, turntable, C_DARK_METAL, {0, 0, 0}, {1.6f, 0.12f, 1.6f}); // Base
            glm::mat4 turnTop = glm::translate(turntable, {0, 0.12f, 0});
            turnTop = glm::rotate(turnTop, glm::radians(turnRot), {0, 1, 0});
            drawCube(V, ls, turnTop, C_CYAN_ACCENT, {0, 0, 0}, {1.4f, 0.06f, 1.4f}); // Spinning top
            
            // Lift structure rails at X=9, Z=-1.5
            drawCube(V, ls, I, C_DARK_METAL, {8.0f, WH, -1.5f}, {0.2f, MH, 0.2f});  // Left rail
            drawCube(V, ls, I, C_DARK_METAL, {10.0f, WH, -1.5f}, {0.2f, MH, 0.2f}); // Right rail
            drawCube(V, ls, I, C_DARK_METAL, {9.0f, MH - 0.1f, -1.5f}, {2.2f, 0.2f, 0.2f}); // Top machinery
            
            // Dynamic lift platform spanning the queueing branch
            float platY = 1.0f + factoryLiftY;
            drawCube(V, ls, I, C_DARK_METAL, {9.0f, platY, -1.5f}, {1.8f, 0.12f, 2.8f});
            
            // UPPER UNLOAD CONVEYOR FLOOR AND PORTAL
            float upperY = 4.08f; 
            drawCube(V, ls, I, C_METAL * 0.85f, {9.0f, upperY - 0.08f, -7.5f}, {1.5f, 0.12f, 9.0f});
            drawCube(V, ls, I, C_CONVEYOR, {9.0f, upperY + 0.02f, -7.5f}, {1.3f, 0.05f, 9.0f});
            // Glowing exit chute destination wall
            drawCube(V, ls, I, C_DARK_METAL, {9.0f, upperY + 0.5f, -12.0f}, {2.0f, 1.5f, 0.5f}); 
            drawCube(V, ls, I, C_CYAN_ACCENT * 0.8f, {9.0f, upperY + 0.5f, -11.8f}, {1.2f, 1.0f, 0.2f});
            
            // Optional cyan pulse on the lift motor
            float liftPulse = sin(globalTime * 3.0f) * 0.5f + 0.5f;
            drawCube(V, ls, I, C_CYAN_ACCENT * liftPulse, {9.0f, MH - 0.3f, -1.5f}, {0.4f, 0.4f, 0.4f});
        }

        // ===== COLOR APPLICATION CHAMBER (objects pass through and get recolored) =====
        {
            float chamberX = -8.0f;
            float chamberZ = 16.0f;
            float chamberY = 1.75f; // Raised height for main conv

            // Calculate dynamic door opening based on box proximity
            float minDistToEntry = 100.0f;
            float minDistToExit = 100.0f;
            bool objectInside = false;
            
            // To apply the color on the chamber, we grab the color of a box moving inside it
            glm::vec3 activeColor = C_MACHINE; // default

            for (int b = 0; b < MAX_BOXES; b++) {
                if (boxes[b].active && boxes[b].position > -18.0f && boxes[b].position < 0.0f) {
                    float bx = -24.0f + (boxes[b].position + 25.0f); // Map pos to bx on main conveyor
                    
                    float distEntry = (chamberX - 2.0f) - bx; // Distance to front door
                    float distExit = bx - (chamberX + 2.0f);  // Distance from back door
                    
                    if (distEntry > -0.5f && distEntry < minDistToEntry) minDistToEntry = distEntry;
                    if (distExit > -0.5f && distExit < minDistToExit) minDistToExit = distExit;
                    
                    if (bx > chamberX - 2.0f && bx < chamberX + 2.0f) {
                        objectInside = true;
                        activeColor = boxes[b].color;
                    }
                }
            }
            
            // Proximity sliding doors (open seamlessly as packages approach)
            float entryDoorOpen = glm::clamp(1.0f - (minDistToEntry - 0.2f), 0.0f, 1.0f);
            float exitDoorOpen = glm::clamp(1.0f - (minDistToExit - 0.2f), 0.0f, 1.0f);

            // Exterior High-Tech Chamber Housing 
            drawCube(V, ls, I, C_DARK_METAL, {chamberX, chamberY, chamberZ - 1.8f}, {4.2f, 1.4f, 0.3f}); // Front
            drawCube(V, ls, I, C_DARK_METAL, {chamberX, chamberY, chamberZ + 1.8f}, {4.2f, 1.4f, 0.3f}); // Back
            drawCube(V, ls, I, C_MACHINE, {chamberX, chamberY + 0.8f, chamberZ}, {4.2f, 0.2f, 4.0f}); // Roof
            
            // Core processing area heavily glows with the specific color of the package inside
            glm::vec3 glowColor = objectInside ? activeColor * 1.5f : C_CYAN_ACCENT * 0.3f;
            drawCube(V, ls, I, glowColor, {chamberX, chamberY, chamberZ}, {3.8f, 1.3f, 3.2f}, 128, 0.6f); // Inner volume glow

            // Transparent side observation windows
            drawCube(V, ls, I, C_GLASS, {chamberX, chamberY, chamberZ - 1.9f}, {2.5f, 0.8f, 0.1f}, 128, 0.2f);

            // Solid sliding doors that retract vertically into the roof
            drawCube(V, ls, I, C_METAL, {chamberX - 2.1f, chamberY - 0.1f + entryDoorOpen * 1.5f, chamberZ}, {0.1f, 1.4f, 1.8f}); // Entry
            drawCube(V, ls, I, C_METAL, {chamberX + 2.1f, chamberY - 0.1f + exitDoorOpen * 1.5f, chamberZ}, {0.1f, 1.4f, 1.8f}); // Exit

            // Pulsing door frame rings
            glm::vec3 ringColorEntry = entryDoorOpen > 0.1f ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
            glm::vec3 ringColorExit = exitDoorOpen > 0.1f ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
            drawCube(V, ls, I, ringColorEntry, {chamberX - 2.1f, chamberY + 1.0f, chamberZ}, {0.15f, 0.1f, 2.0f}, 64, 1.0f);
            drawCube(V, ls, I, ringColorExit, {chamberX + 2.1f, chamberY + 1.0f, chamberZ}, {0.15f, 0.1f, 2.0f}, 64, 1.0f);

            // Internal rotating applicator nodes painting the color
            float sprayRot = fmod(globalTime * 400.0f, 360.0f);
            for (int r = -1; r <= 1; r+=2) {
                glm::mat4 sprayMount = glm::translate(I, {chamberX + r * 0.8f, chamberY, chamberZ});
                sprayMount = glm::rotate(sprayMount, glm::radians(sprayRot), glm::vec3(1, 0, 0));
                for(int s = 0; s < 4; s++) { 
                    glm::mat4 sprayArm = glm::rotate(sprayMount, glm::radians(s * 90.0f), glm::vec3(1, 0, 0));
                    drawCube(V, ls, sprayArm, glowColor * 1.2f, {0, 0.6f, 0}, {0.1f, 0.3f, 0.1f});
                }
            }
            
            // Roof indicator beacon
            glm::vec3 beaconColor = objectInside ? activeColor : glm::vec3(1.0f, 0.5f, 0.0f); // Flashes the active color
            drawCube(V, ls, I, beaconColor * (0.8f + sin(globalTime*12.0f)*0.3f), {chamberX, chamberY + 1.0f, chamberZ}, {0.4f, 0.3f, 0.4f}, 64, 1.0f);
        }
        
        // ===== WELDING ROBOT STATION (Center) =====
        {
            glm::mat4 welderBase = glm::translate(I, {-10, 1.85f, 16});
            
            // Prepare kinematics joint angles
            float j1_baseRotation = fmod(globalTime * 20.0f, 360.0f);
            float j2_shoulderRot = 30.0f + sin(machineState.weldArmAngle2) * 40.0f;
            float j3_elbowRot = 60.0f - sin(globalTime * 0.8f) * 50.0f;
            float j4_wristPitch = sin(globalTime * 1.5f) * 45.0f;
            float j5_wristRoll = sin(globalTime * 0.7f) * 30.0f;
            float j6_toolRot = fmod(globalTime * 60.0f, 360.0f);
            
            ComplexMachine::drawAdvancedWeldingRobot(V, ls, welderBase, j1_baseRotation, j2_shoulderRot,
                                                     j3_elbowRot, j4_wristPitch, j5_wristRoll, j6_toolRot,
                                                     machineState.weldTorchGlow, globalTime, sphHub);
        }
        
        // ===== PACKAGING & INSPECTION AREA (Right side) =====
        {
            // Inspection table
            drawCubeTextured(texCubeVAO, ls, I, C_METAL, {-6, .5f, 12}, {3, .15f, 2}, texMetal, 2.0f, 64, 1.0f); // top surface
            drawCubeTextured(texCubeVAO, ls, I, C_DARK_METAL, {-6, .15f, 12}, {3.2f, .3f, 2.2f}, texMetal, 2.0f, 64, 1.0f); // base
            // Support legs
            drawCube(V, ls, I, C_MACHINE, {-7.5f, .25f, 11}, {.2f, .25f, .2f});
            drawCube(V, ls, I, C_MACHINE, {-4.5f, .25f, 11}, {.2f, .25f, .2f});
            drawCube(V, ls, I, C_MACHINE, {-7.5f, .25f, 13}, {.2f, .25f, .2f});
            drawCube(V, ls, I, C_MACHINE, {-4.5f, .25f, 13}, {.2f, .25f, .2f});
            
            // Quality control camera mount (animated pan)
            float cameraRot = sin(globalTime * 1.0f) * 30.0f;
            glm::mat4 cameraMount = glm::translate(I, {-6, 1.2f, 12});
            cameraMount = glm::rotate(cameraMount, glm::radians(cameraRot), {0, 1, 0});
            drawCube(V, ls, cameraMount, C_DARK_METAL, {0, 0, 0}, {.3f, .2f, .1f});
            drawCube(V, ls, cameraMount, C_CYAN_ACCENT, {0, 0, -.15f}, {.15f, .1f, .1f}); // lens (cyan)
        }
        
        // ===== TRANSITION CONVEYOR CHUTE (Connects High Assembly Conveyor to Low Inspection) =====
        {
            glm::vec3 startP(-4.0f, 1.05f, 16.0f); // High start point from main conveyor
            glm::vec3 endP(-18.0f, 0.15f, -6.0f);  // Low endpoint near packaging
            glm::vec3 midP = (startP + endP) * 0.5f;
            float distXZ = glm::length(glm::vec2(endP.x - startP.x, endP.z - startP.z));
            float dist = glm::length(endP - startP);
            
            float angleY = atan2(endP.x - startP.x, endP.z - startP.z);
            float anglePitch = atan2(startP.y - endP.y, distXZ); // Downward slope
            
            glm::mat4 transModel = glm::translate(I, midP);
            transModel = glm::rotate(transModel, angleY, glm::vec3(0, 1, 0));
            transModel = glm::rotate(transModel, anglePitch, glm::vec3(1, 0, 0));
            
            // Draw a straight sloped transition chute aligned exactly
            drawCube(V, ls, transModel, C_CONVEYOR, {0, 0.0f, 0}, {1.5f, 0.05f, dist});
            drawCube(V, ls, transModel, C_DARK_METAL, {-0.8f, 0.05f, 0}, {0.1f, 0.15f, dist});
            drawCube(V, ls, transModel, C_DARK_METAL, {0.8f, 0.05f, 0}, {0.1f, 0.15f, dist});
            
            // Animated cyan strip along the chute moving with objects
            float offset = fmod(conveyorOffset * 3.0f, dist) - dist*0.5f;
            drawCube(V, ls, transModel, C_CYAN_ACCENT, {0, 0.025f, offset}, {0.4f, 0.01f, 1.5f});
        }
        
        // ===== INDUSTRIAL EQUIPMENT OVERHEAD =====
        {
            // Cable/hose runs along ceiling
            drawCubeTextured(texCubeVAO, ls, I, C_DARK_METAL, {-14, MH - 0.2f, 16}, {20, .1f, .3f}, texMetal, 4.0f, 16, 1.0f);
            // Electrical conduit (animated)
            drawCubeTextured(texCubeVAO, ls, I, C_MACHINE, {-14, MH - 0.4f, 16}, {20, .06f, .2f}, texMetal, 4.0f, 32, 1.0f);
            
            // Safety light beacon (pulsing)
            float beaconIntensity = sin(globalTime * 3.0f) * 0.5f + 0.5f;
            drawCube(V, ls, I, C_CYAN_ACCENT * (0.4f + beaconIntensity * 0.6f), 
                     {-4, MH - 0.3f, 16}, {.2f, .2f, .2f});
        }
        
        // ===== EMERGENCY STOP STATION & CONTROL PANEL =====
        {
            // Control console mounted on wall
            drawCubeTextured(texCubeVAO, ls, I, C_DARK_METAL, {-24.3f, 1.5f, 18}, {.3f, 1.2f, 1.5f}, texMetal, 1.0f, 32, 1.0f); // panel mount
            drawCubeTextured(texCubeVAO, ls, I, C_DARK_METAL, {-24.2f, 1.5f, 18}, {.15f, 1.0f, 1.2f}, texMetal, 1.0f, 32, 1.0f); // panel face
            
            // E-stop button (red, pulsing)
            float estopGlow = sin(globalTime * 2.5f) * 0.3f + 0.7f;
            drawCube(V, ls, I, glm::vec3(1.0f, 0.1f, 0.1f) * estopGlow, 
                     {-24.1f, 1.8f, 18.4f}, {.15f, .15f, .05f});
            
            // LED indicators
            for (int led = 0; led < 4; led++) {
                float ledBright = sin(globalTime * 1.5f + led * 0.7f) * 0.5f + 0.5f;
                glm::vec3 ledColors[] = {glm::vec3(0.1f, 1.0f, 0.1f), C_CYAN_ACCENT, glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.5f, 0.0f)};
                drawCube(V, ls, I, ledColors[led] * (0.4f + ledBright * 0.6f), 
                         {-24.1f, 1.2f - led * 0.25f, 18.3f}, {.08f, .08f, .03f});
            }
        }
    }

    };
    drawFactoryWarehouse();

    auto drawClothingShop = [&]() {
    // ======== CLOTHING SHOP (X:4 to 24.5, Z:8 to 24) — Southeast ========
    {
        // Convert clothing shop into advanced assembly area
        drawCubeTextured(texCubeVAO, ls, I, C_FACTORY_FLOOR, {14, .03f, 16}, {21, .04f, 16}, texConcrete, 6.0f, 16, 1.0f);
        drawCubeTextured(texCubeVAO, ls, I, C_FASH_WALL * 0.6f, {24.5f, WH, 16}, {WT, MH, 16}, texBrick, 8.0f, 32, 1.0f);
        
        // Assembly line machines
        drawCubeTextured(texCubeVAO, ls, I, C_MACHINE, {18, .5f, 14}, {4, 1.0f, 6}, texMetal, 2.0f, 32, 1.0f);
        drawCubeTextured(texCubeVAO, ls, I, C_METAL, {22, .5f, 8}, {6, 1.0f, 4}, texMetal, 2.0f, 64, 1.0f);
        
        // Secondary conveyor with cyan accent
        drawCube(V, ls, I, C_CONVEYOR, {14, .15f, 18}, {8, .2f, 4});
        // Cyan animated stripe
        float stripePos = 18 + fmod(conveyorOffset * 2.5f, 4.0f) - 2.0f;
        drawCube(V, ls, I, C_CYAN_ACCENT, {14, .16f, stripePos}, {8 * 0.95f, .01f, 0.5f});
        // Rollers for secondary conveyor
        for (int r = 0; r < 3; r++) {
            float rz = 16 + r * 2.0f;
            drawCube(V, ls, I, C_ROLLER, {14 - 4.0f + 0.1f, .07f, rz}, {.08f, .08f, 7.8f});
            drawCube(V, ls, I, C_ROLLER, {14 + 4.0f - 0.1f, .07f, rz}, {.08f, .08f, 7.8f});
        }
        
        // Heavy-duty 6-axis robotic arm (main assembly)
        {
            glm::mat4 baseArm = glm::translate(I, {14, 1.0f, 12});
            
            // Realistic joint angle calculations based on task cycle
            float taskCycle = fmod(globalTime, 4.0f) / 4.0f; // 4-second cycle
            
            // ===== 6-AXIS ARM KINEMATICS CHAIN =====
            // J1: Base rotation (Z-axis) - full 360° capability
            float j1_angle = fmod(globalTime * 45.0f, 360.0f);
            
            // J2: Shoulder joint (Y-axis) - reaches and retracts
            float j2_angle = taskCycle < 0.5f 
                ? 45.0f + sin(taskCycle * 3.14159f * 4.0f) * 25.0f 
                : 45.0f - sin((taskCycle - 0.5f) * 3.14159f * 4.0f) * 25.0f;
            
            // J3: Elbow joint (Y-axis) - follows shoulder motion for reach
            float j3_angle = 60.0f - sin(taskCycle * 3.14159f * 2.0f) * 40.0f;
            
            // J4: Wrist pitch (X-axis) - tool approach angle
            float j4_angle = sin(globalTime * 2.0f) * 30.0f;
            
            // J5: Wrist roll (Z-axis) - rotation around tool axis
            float j5_angle = fmod(globalTime * 60.0f, 360.0f);
            
            // J6: Tool rotation (Y-axis) - fine orientation control
            float j6_angle = sin(globalTime * 1.3f) * 45.0f;
            
            // Robust base
            drawCube(V, ls, I, C_METAL, {14, .5f, 12}, {1.5f, .7f, 1.5f});
            
            // ===== BUILD KINEMATICS CHAIN =====
            // Joint 1: Base joint (vertical axis rotation)
            glm::mat4 joint1 = glm::translate(baseArm, {0, .6f, 0});
            joint1 = glm::rotate(joint1, glm::radians(j1_angle), {0, 1, 0});
            drawCube(V, ls, joint1, C_DARK_METAL, {0, 0, 0}, {.4f, .3f, .4f});
            
            // Segment 1: Base to shoulder (upper arm)
            glm::mat4 seg1 = glm::translate(joint1, {0, 0.2f, 0});
            seg1 = glm::rotate(seg1, glm::radians(j2_angle), {1, 0, 0});
            drawCube(V, ls, seg1, C_MACHINE, {0, .35f, .4f}, {.3f, .15f, .8f});
            
            // Joint 2: Shoulder joint
            glm::mat4 joint2 = glm::translate(seg1, {0, .4f, .6f});
            joint2 = glm::rotate(joint2, glm::radians(j3_angle), {1, 0, 0});
            drawCube(V, ls, joint2, C_DARK_METAL, {0, 0, 0}, {.35f, .25f, .35f});
            
            // Segment 2: Shoulder to elbow (forearm)
            glm::mat4 seg2 = glm::translate(joint2, {0, .25f, .5f});
            seg2 = glm::rotate(seg2, glm::radians(j4_angle), {0, 1, 0});
            drawCube(V, ls, seg2, C_MACHINE, {0, .3f, .35f}, {.25f, .12f, .7f});
            
            // Joint 3: Wrist pitch joint (3-axis wrist)
            glm::mat4 joint3 = glm::translate(seg2, {0, .25f, .45f});
            joint3 = glm::rotate(joint3, glm::radians(j5_angle), {0, 0, 1});
            drawCube(V, ls, joint3, C_DARK_METAL, {0, 0, 0}, {.2f, .15f, .2f});
            
            // Wrist assembly (compact 3-axis tool interface)
            glm::mat4 wristAssy = glm::translate(joint3, {0, 0, .15f});
            wristAssy = glm::rotate(wristAssy, glm::radians(j6_angle), {0, 1, 0});
            drawCube(V, ls, wristAssy, C_METAL, {0, 0, 0}, {.18f, .12f, .25f});
            
            // End effector: 4-finger precision gripper with servo actuators
            glm::mat4 gripper = glm::translate(wristAssy, {0, 0, .13f});
            
            // Main gripper palm
            drawCube(V, ls, gripper, C_CYAN_ACCENT, {0, 0, 0}, {.2f, .25f, .15f});
            
            // Servo actuators (show gripper actuation)
            float gripperOpening = machineState.robotGripperOpen;
            
            // Finger 1 (left) - servo driven
            drawCube(V, ls, gripper, C_CYAN_ACCENT, 
                     {-0.08f - gripperOpening * 0.05f, -.15f, 0}, {.04f, .15f, .1f});
            // Finger 1 servo (small motor)
            drawCube(V, ls, gripper, C_DARK_METAL, 
                     {-0.08f - gripperOpening * 0.05f, -.22f, -0.02f}, {.06f, .04f, .06f});
            
            // Finger 2 (right) - servo driven
            drawCube(V, ls, gripper, C_CYAN_ACCENT, 
                     {0.08f + gripperOpening * 0.05f, -.15f, 0}, {.04f, .15f, .1f});
            // Finger 2 servo
            drawCube(V, ls, gripper, C_DARK_METAL, 
                     {0.08f + gripperOpening * 0.05f, -.22f, -0.02f}, {.06f, .04f, .06f});
            
            // Finger 3 (front) - servo driven
            drawCube(V, ls, gripper, C_CYAN_ACCENT, 
                     {0, -.15f, -0.06f - gripperOpening * 0.03f}, {.1f, .15f, .04f});
            // Finger 3 servo
            drawCube(V, ls, gripper, C_DARK_METAL, 
                     {0, -.22f, -0.08f - gripperOpening * 0.03f}, {.08f, .04f, .06f});
            
            // Finger 4 (back) - servo driven
            drawCube(V, ls, gripper, C_CYAN_ACCENT, 
                     {0, -.15f, 0.06f + gripperOpening * 0.03f}, {.1f, .15f, .04f});
            // Finger 4 servo
            drawCube(V, ls, gripper, C_DARK_METAL, 
                     {0, -.22f, 0.08f + gripperOpening * 0.03f}, {.08f, .04f, .06f});
            
            // ===== CABLE & HOSE MANAGEMENT =====
            // Power cable running along arm
            drawCube(V, ls, seg1, C_DARK_METAL * 0.6f, {-.12f, .35f, .4f}, {.03f, .15f, .6f});
            drawCube(V, ls, seg2, C_DARK_METAL * 0.6f, {.12f, .3f, .35f}, {.03f, .12f, .5f});
            
            // Pneumatic line (for gripper actuation)
            drawCube(V, ls, gripper, glm::vec3(0.8f, 0.2f, 0.8f), {0, -.08f, -.06f}, {.025f, .08f, .1f});
            
            // ===== JOINT INDICATOR LIGHTS (status feedback) =====
            // Show current task phase
            glm::vec3 taskColor;
            if (taskCycle < 0.25f) {
                taskColor = glm::vec3(0.1f, 1.0f, 0.1f); // Picking (green)
            } else if (taskCycle < 0.5f) {
                taskColor = glm::vec3(1.0f, 0.9f, 0.1f); // Moving (yellow)
            } else if (taskCycle < 0.75f) {
                taskColor = glm::vec3(0.1f, 0.8f, 1.0f); // Placing (cyan)
            } else {
                taskColor = glm::vec3(1.0f, 0.4f, 0.1f); // Returning (orange)
            }
            
            float brightness = sin(globalTime * 2.0f) * 0.2f + 0.8f;
            drawCube(V, ls, baseArm, taskColor * brightness, {0, .8f, 0}, {.15f, .1f, .1f});
            
            // ===== WORKPIECE / COMPONENT BEING ASSEMBLED =====
            // Show the part the robot is handling
            glm::mat4 workPieceTransform = glm::translate(I, {14, 0.4f, 14.5f});
            
            // Part body
            drawCube(V, ls, workPieceTransform, glm::vec3(0.7f, 0.7f, 0.75f), {0, 0, 0}, {.6f, .5f, .6f});
            
            // Assembly indicators (simulated screws/fasteners)
            for (int p = 0; p < 3; p++) {
                float px = -0.15f + p * 0.15f;
                drawCube(V, ls, workPieceTransform, glm::vec3(0.4f, 0.4f, 0.45f), {px, .15f, 0}, {.08f, .08f, .08f});
            }
        }
        
        // Secondary arm (SCARA-style - Selective Compliance Articulated Robot Arm)
        {
            glm::mat4 scaraBase = glm::translate(I, {18, 1.0f, 16});
            
            // SCARA kinematics: parallel linked arms
            // Link 1 rotation
            float link1Angle = globalTime * 60.0f + sin(globalTime * 1.8f) * 60.0f;
            // Link 2 rotation (constrained parallel motion)
            float link2Angle = -sin(globalTime * 1.8f) * 60.0f;
            
            // Vertical motion (Z axis) - up/down reaching
            float scaraZ = 0.4f + sin(globalTime * 0.9f) * 0.2f;
            
            drawCube(V, ls, I, C_METAL, {18, .5f, 16}, {1.2f, .6f, 1.2f}); // base
            
            // Horizontal arm 1 (first link)
            glm::mat4 scaraArm1 = glm::translate(scaraBase, {0, scaraZ, 0});
            scaraArm1 = glm::rotate(scaraArm1, glm::radians(link1Angle), {0, 1, 0});
            drawCube(V, ls, scaraArm1, C_MACHINE, {.6f, .3f, 16}, {.3f, .12f, .35f});
            
            // Joint 1 bearing
            drawCube(V, ls, scaraArm1, C_CYAN_ACCENT, {0, 0, 0}, {.35f, .2f, .35f});
            
            // Horizontal arm 2 (second link) - parallel linkage
            glm::mat4 scaraArm2 = glm::translate(scaraArm1, {.6f, 0, 0});
            scaraArm2 = glm::rotate(scaraArm2, glm::radians(link2Angle), {0, 1, 0});
            drawCube(V, ls, scaraArm2, C_MACHINE, {.6f, .3f, 16}, {.3f, .12f, .35f});
            
            // Joint 2 bearing
            drawCube(V, ls, scaraArm2, C_CYAN_ACCENT, {0, 0, 0}, {.3f, .18f, .3f});
            
            // End effector with vacuum cup (animated grip)
            glm::mat4 vacuum = glm::translate(scaraArm2, {.4f, -.15f, 0});
            float vacuumPower = 0.3f + sin(globalTime * 2.0f) * 0.2f; // pulsing suction
            glm::vec3 vacuumColor = C_CYAN_ACCENT * (0.5f + vacuumPower * 0.5f);
            drawCube(V, ls, vacuum, vacuumColor, {0, 0, 0}, {.18f, .08f, .18f}); // vacuum cup
            
            // Suction force indicator (small pulsing sphere effect)
            drawCube(V, ls, vacuum, glm::vec3(0.0f, 0.8f, 1.0f) * vacuumPower * 0.3f, 
                     {0, -.12f, 0}, {.12f, .04f, .12f});
        }
        
        // Control panel with lights
        drawCube(V, ls, I, C_DARK_METAL, {12, 1.5f, 22}, {2, 1.2f, 0.3f}); // panel
        // LED lights (simulate with small cyan cubes)
        for (int led = 0; led < 6; led++) {
            float ledBrightness = sin(globalTime * 2.0f + led * 0.5f) * 0.5f + 0.5f;
            glm::vec3 ledColor = C_CYAN_ACCENT * (0.5f + ledBrightness * 0.5f);
            drawCube(V, ls, I, ledColor, {12 - 0.6f + led * 0.25f, 1.8f, 22.15f}, {.08f, .08f, .05f});
        }
    }

    };
    drawClothingShop();

    auto drawMallAmenities = [&]() {
    // ======== ESCALATOR (West wall, X:-22 to -18, Z:0 to 8, facing EAST into corridor) ========
    {
        float esX = -20.f, esZ1 = 8.0f, esZ2 = 0.0f;
        float esLen = esZ1 - esZ2;
        float esH = 4.5f;
        
        // WEST SIDE WALL (Brick texture for industrial look)
        drawCubeTextured(texCubeVAO, ls, I, C_EXT, {-24.5f, WH, 4}, {WT, MH, 8}, texBrick, 5.0f, 32, 0.9f);
        
        // NO EAST WALL IN ESCALATOR AREA - Open corridor for clear handrail visibility
        // Handrail can be seen from the corridor
        
        // Side rails (run N-S) - positioned accurately on west side
        drawCube(V, ls, I, C_LAMP, {esX-0.7f, esH/2, (esZ1+esZ2)/2}, {.05f,esH+.5f,esLen});
        drawCube(V, ls, I, C_LAMP, {esX+0.7f, esH/2, (esZ1+esZ2)/2}, {.05f,esH+.5f,esLen});
        
        // Moving steps (go from Z=8 bottom to Z=0 top, rising northward)
        int numSteps = 16;
        float stepLen = esLen / numSteps;
        for (int i = 0; i < numSteps; i++) {
            float rawZ = esZ2 + (i + escalatorOffset/(esLen/numSteps)) * stepLen;
            float frac = fmod(rawZ - esZ2, esLen);
            if (frac < 0) frac += esLen;
            float sZ = esZ2 + frac;
            float t = 1.0f - (sZ - esZ2) / esLen; // rises as Z decreases (north)
            float sY = t * esH;
            drawCube(V, ls, I, C_STAIR*1.05f, {esX, sY+.05f, sZ}, {1.2f,.1f,stepLen*.85f});
        }
        
        // Floor area around escalator - accurately positioned on west side
        drawCube(V, ls, I, C_STAIR*.9f, {-20, .03f, 4}, {8, .04f, 8});
        
        // ===== VISIBLE ESCALATOR HANDRAIL CURVE (Brass tube) - NOW CLEARLY VISIBLE =====
        // Positioned accurately following the escalator from (-19.3, 0.9, 8) to (-18.9, 3.8, 0)
        if (visibleHandrail != nullptr) {
            glm::mat4 railModel = glm::mat4(1.0f); // Identity - curve is already in correct coordinates
            // Enhanced brass color - more saturated for prominence
            visibleHandrail->draw(ls, railModel, glm::vec3(0.95f, 0.75f, 0.20f)); // Brighter brass
        }
    }

    // ======== STAIRCASE (East side, X:16 to 24, Z:0 to 8, facing EAST → steps go along X) ========
    {
        float stDepth = 8.f; // Z extent (Z:0 to 8)
        int numSteps = 10;
        float stepW = 8.f / numSteps; // step width along X
        for (int i = 0; i < numSteps; i++) {
            float sx = 16.f + i * stepW + stepW / 2;
            float sy = i * .5f + .25f;
            drawCube(V, ls, I, C_STAIR, {sx, sy, 4}, {stepW, .5f, stDepth});
        }
        // Railings (run along X on both Z sides)
        drawCube(V, ls, I, C_LAMP, {20, 2.5f, 0.2f}, {8, MH, .08f});
        drawCube(V, ls, I, C_LAMP, {20, 2.5f, 7.8f}, {8, MH, .08f});
        // Floor
        drawCube(V, ls, I, C_STAIR*.9f, {20, .03f, 4}, {8, .04f, 8});
    }

    // ======== PRAYER ROOM (X:-24.5 to -4, Z:-14 to -7) — Northwest upper ========
    {
        drawCubeTextured(texCubeVAO, ls, I, C_PRAY_FLOOR, {-14, .03f, -10.5f}, {21, .04f, 7}, texFloor, 6.0f, 16, 1.0f);
        drawCubeTextured(texCubeVAO, ls, I, C_PRAY_WALL, {-24.5f, WH, -10.5f}, {WT, MH, 7}, texBrick, 5.0f, 32, 1.0f);
        // Prayer mat
        drawCube(V, ls, I, C_PRAY_MAT, {-16, .06f, -10}, {5, .04f, 3});
        drawCube(V, ls, I, C_PRAY_MAT*.9f, {-16, .07f, -10}, {4, .02f, 2.2f});
        // Standing lamp
        drawCube(V, ls, I, C_PRAY_LAMP*.6f, {-22, .05f, -12}, {.5f,.1f,.5f});
        drawCube(V, ls, I, C_PRAY_LAMP*.5f, {-22, 1.2f, -12}, {.08f,2.3f,.08f});
        drawCube(V, ls, I, C_PRAY_LAMP, {-22, 2.6f, -12}, {.5f,.8f,.5f});
        // Bookshelf
        drawCube(V, ls, I, C_PRAY_SHELF, {-8, .5f, -12}, {1.5f,1,.8f});
        drawCube(V, ls, I, C_PRAY_SHELF*.8f, {-8, .7f, -12}, {1.2f,.3f,.6f});
    }

    // ======== WASH ROOM (X:-24.5 to -4, Z:-7 to 0) — West middle ========
    {
        drawCubeTextured(texCubeVAO, ls, I, C_WASH, {-14, .03f, -3.5f}, {21, .04f, 7}, texFloor, 6.0f, 32, 1.0f);
        // Stall partitions
        for (int i = 0; i < 4; i++)
            drawCube(V, ls, I, C_WASH*.9f, {-22 + i * 4.f, 1.2f, -3.5f}, {.08f,2.2f,5});
    }

    // ======== GEMS SHOP (X:4 to 24.5, Z:-14 to -7) — Northeast upper ========
    {
        drawCubeTextured(texCubeVAO, ls, I, C_GEM_WALL, {14, .03f, -10.5f}, {21, .04f, 7}, texFloor, 6.0f, 32, 1.0f);
        drawCubeTextured(texCubeVAO, ls, I, C_GEM_WALL, {24.5f, WH, -10.5f}, {WT, MH, 7}, texBrick, 5.0f, 32, 1.0f);
        // Glass display cases
        for (int i = 0; i < 3; i++) {
            float gx = 9 + i * 6.f;
            drawCube(V, ls, I, C_GEM_CASE, {gx, .5f, -10}, {3, 1, 2});
            drawCube(V, ls, I, C_GLASS, {gx, 1.15f, -10}, {3.1f,.3f,2.1f}, 128, .3f);
            drawCube(V, ls, I, {.9f,.2f,.2f}, {gx-.5f,.75f,-10}, {.25f,.18f,.25f}, 128);
            drawCube(V, ls, I, {.2f,.3f,.9f}, {gx+.5f,.75f,-10}, {.25f,.18f,.25f}, 128);
            drawCube(V, ls, I, {.2f,.8f,.3f}, {gx,.75f,-10.5f}, {.2f,.15f,.2f}, 128);
        }
        // Wall display
        drawCube(V, ls, I, C_GEM_CASE*.9f, {23, 1.5f, -9}, {.6f,2.8f,4});
        drawCube(V, ls, I, C_COUNTER, {7, .55f, -12}, {3, 1.1f, 2});
    }

    // ======== LIFT (X:4 to 14, Z:-7 to 0) — East middle ========
    {
        float eX = 9.f, eZ = -3.5f;
        // Shaft (glass walls)
        drawCube(V, ls, I, C_GLASS, {eX-1.5f, WH, eZ}, {.08f, MH, 3}, 128, .2f);
        drawCube(V, ls, I, C_GLASS, {eX+1.5f, WH, eZ}, {.08f, MH, 3}, 128, .2f);
        drawCube(V, ls, I, C_GLASS, {eX, WH, eZ-1.5f}, {3, MH, .08f}, 128, .2f);
        // Shaft rails
        drawCube(V, ls, I, C_LAMP, {eX-1.45f, WH, eZ-1.45f}, {.05f,MH,.05f});
        drawCube(V, ls, I, C_LAMP, {eX+1.45f, WH, eZ-1.45f}, {.05f,MH,.05f});
        drawCube(V, ls, I, C_LAMP, {eX-1.45f, WH, eZ+1.45f}, {.05f,MH,.05f});
        drawCube(V, ls, I, C_LAMP, {eX+1.45f, WH, eZ+1.45f}, {.05f,MH,.05f});
        // Moving platform
        drawCube(V, ls, I, C_STAIR, {eX, elevatorY+.05f, eZ}, {2.8f,.1f,2.8f});
        // Sliding doors (face WEST toward corridor, at X=eX-1.5)
        float doorHalf = 1.4f * (1.f - elevatorDoorOffset);
        if (doorHalf > 0.05f) {
            drawCube(V, ls, I, C_GLASS, {eX-1.5f, elevatorY+.8f, eZ-doorHalf*.5f-.05f}, {.06f,1.4f,doorHalf}, 128, .25f);
            drawCube(V, ls, I, C_GLASS, {eX-1.5f, elevatorY+.8f, eZ+doorHalf*.5f+.05f}, {.06f,1.4f,doorHalf}, 128, .25f);
        }
        // Button panel
        drawCube(V, ls, I, C_LAMP, {eX-1.8f, 1.2f, eZ+1.2f}, {.15f,.25f,.1f});
        drawCube(V, ls, I, {.2f,.8f,.2f}, {eX-1.82f, 1.28f, eZ+1.2f}, {.03f,.08f,.08f});
        drawCube(V, ls, I, {.8f,.2f,.2f}, {eX-1.82f, 1.12f, eZ+1.2f}, {.03f,.08f,.08f});
    }

    // ======== FOOD COURT (Z:-25 to -14, full width) — North ========
    {
        drawCubeTextured(texCubeVAO, ls, I, C_FOOD_FLOOR, {0, .03f, -19.5f}, {49.5f, .04f, 11}, texFloor, 12.0f, 16, 1.0f);
        // Food stalls along north wall
        drawCube(V, ls, I, C_STALL_RED, {-22, WH, -24.5f}, {5, MH, 1});
        drawCube(V, ls, I, C_COUNTER, {-22, .55f, -23}, {4, 1.1f, 2});
        drawCube(V, ls, I, C_STALL_YEL, {-12, WH, -24.5f}, {5, MH, 1});
        drawCube(V, ls, I, C_COUNTER, {-12, .55f, -23}, {4, 1.1f, 2});
        drawCube(V, ls, I, C_STALL_GRN, {5, WH, -24.5f}, {5, MH, 1});
        drawCube(V, ls, I, C_COUNTER, {5, .55f, -23}, {4, 1.1f, 2});
        drawCube(V, ls, I, C_STALL_ORG, {18, WH, -24.5f}, {5, MH, 1});
        drawCube(V, ls, I, C_COUNTER, {18, .55f, -23}, {4, 1.1f, 2});
        // Seating (tables + chairs)
        for (int r = 0; r < 2; r++)
            for (int c = 0; c < 4; c++) {
                float tx = -14+c*9.f, tz = -16-r*4.f;
                glm::mat4 ftm = glm::translate(I, {tx, 0.75f, tz});
                // Draw table with wood texture for realism
                if (texturesEnabled && texWood != 0) {
                    ls.use();
                    ls.setBool("useTexture", true);
                    ls.setBool("blendWithColor", true);
                    ls.setFloat("texTiling", 2.0f);
                    ls.setVec3("material.ambient", C_TABLE * 0.8f);
                    ls.setVec3("material.diffuse", C_TABLE);
                    ls.setVec3("material.specular", glm::vec3(0.2f));
                    ls.setFloat("material.shininess", 32.0f);
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, texWood);
                    ls.setInt("texture1", 0);
                    ls.setMat4("model", ftm);
                    bezierTable->draw(ls, ftm, C_TABLE, 64.0f);
                    ls.setBool("useTexture", false);
                    ls.setBool("blendWithColor", false);
                } else {
                    bezierTable->draw(ls, ftm, C_TABLE, 64.0f);
                }
                drawCube(V, ls, I, C_TABLE*.7f, {tx,.37f,tz}, {.08f,.72f,.08f});
                drawCube(V, ls, I, C_CHAIR, {tx-.8f,.45f,tz}, {.4f,.05f,.4f});
                drawCube(V, ls, I, C_CHAIR, {tx-.8f,.7f,tz-.18f}, {.4f,.45f,.05f});
                drawCube(V, ls, I, C_CHAIR, {tx+.8f,.45f,tz}, {.4f,.05f,.4f});
                drawCube(V, ls, I, C_CHAIR, {tx+.8f,.7f,tz+.18f}, {.4f,.45f,.05f});
            }
    }

    // ROOF PARAPET
    float pY = MH + .5f;
    drawCubeTextured(texCubeVAO, ls, I, C_PARAPET, {0, pY, -25}, {50, 1, .3f}, texBrick, 25.0f, 32, 0.9f);
    drawCubeTextured(texCubeVAO, ls, I, C_PARAPET, {0, pY, 25}, {50, 1, .3f}, texBrick, 25.0f, 32, 0.9f);
    drawCubeTextured(texCubeVAO, ls, I, C_PARAPET, {-25, pY, 0}, {.3f, 1, 50}, texBrick, 25.0f, 32, 0.9f);
    drawCubeTextured(texCubeVAO, ls, I, C_PARAPET, {25, pY, 0}, {.3f, 1, 50}, texBrick, 25.0f, 32, 0.9f);

    // ===== DRAW ALL SIGNBOARDS =====
    // Update signboard status dynamically based on system states (Reversed Indices)
    if (signboards.size() >= 10) {
        signboards[9].isActive = conveyorRunning && (machineState.pressVelocity != 0.0f); // STAMPING
        signboards[8].isActive = conveyorRunning && (machineState.weldTorchGlow > 0.15f); // WELDING
        signboards[7].isActive = conveyorRunning; // INSPECT
        signboards[6].isActive = packagingRunning && (palletizerCurrentSource >= 0); // PALLET
        signboards[5].isActive = packagingRunning; // WRAP
        signboards[4].isActive = packagingRunning; // SHIP
        signboards[3].isActive = conveyorRunning; // ASSEMBLY
        signboards[2].isActive = conveyorRunning; // COLOR APPLIED
        signboards[1].isActive = packagingRunning; // PACKED
        signboards[0].isActive = gatheredBoxes > 0; // GATHERED
    }
    
    for (auto& board : signboards) {
        board.draw(V, texCubeVAO, ls, I, globalTime);
    }

    };
    drawMallAmenities();

    // LIGHT FIXTURE GEOMETRY (visual only, lights off)
    fs.use();
    fs.setMat4("projection", proj);
    fs.setMat4("view", view);
    glBindVertexArray(LV);
    for (int i = 0; i < NUM_POINT_LIGHTS; i++)
    {
        glm::mat4 lm = glm::translate(I, plPos[i]);
        lm = glm::scale(lm, i >= 12 ? glm::vec3(.8f, .15f, .6f) : glm::vec3(1, .08f, .4f));
        fs.setVec3("color", glm::vec3(.9f, .9f, .85f));
        fs.setMat4("model", lm);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    }
}


#endif
