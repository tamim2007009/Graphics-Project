//
//  curveApplications.h
//  Practical applications of curves in the graphics project
//
//  - Factory conveyor belt path (Catmull-Rom spline)
//  - Robotic arm trajectory (Bezier curve)
//  - Cable/rope between structures (Ruled surface)
//  - Escalator handrail (Spline curve)
//  - Welding seam path (Bezier curve)
//  - Ramp/slide surface (Ruled surface)
//

#ifndef curveApplications_h
#define curveApplications_h

#include "bezierCurve.h"
#include <vector>

// ============================================================================
// CONVEYOR PATH - Catmull-Rom spline for smooth factory workflow
// ============================================================================
class ConveyorPath {
private:
    std::vector<glm::vec3> pathPoints;
    std::vector<glm::vec3> smoothPath;

public:
    ConveyorPath() {
        // Define waypoints for conveyor path (input → stamping → welding → packing → output)
        pathPoints = {
            {-25.0f, 1.32f, 16.0f},   // Input start
            {-20.0f, 1.32f, 16.0f},   // Before stamping
            {-8.0f, 1.32f, 16.0f},    // After stamping
            {-4.0f, 1.32f, 16.0f},    // Before welding
            {0.0f, 1.32f, 16.0f},     // During welding
            {6.0f, 1.32f, 16.0f},     // After welding (transition)
            {12.0f, 1.32f, -6.0f},    // Boxing machine
            {20.0f, 1.32f, -6.0f}     // Output
        };

        // Generate smooth Catmull-Rom spline through waypoints
        if (pathPoints.size() >= 4) {
            int samplesPerSegment = 50;
            smoothPath = CatmullRomSpline::generateCurve(pathPoints, samplesPerSegment);
        }
    }

    glm::vec3 getPointOnPath(float normalizedPosition) {
        if (smoothPath.empty()) return glm::vec3(0);
        int index = (int)(normalizedPosition * (smoothPath.size() - 1));
        return smoothPath[glm::clamp(index, 0, (int)smoothPath.size() - 1)];
    }

    int getPathLength() { return smoothPath.size(); }
    
    glm::vec3 getTangentAtPath(float normalizedPosition) {
        if (smoothPath.size() < 2) return glm::vec3(0, 0, 1);
        int index = (int)(normalizedPosition * (smoothPath.size() - 1));
        index = glm::clamp(index, 0, (int)smoothPath.size() - 2);
        return glm::normalize(smoothPath[index + 1] - smoothPath[index]);
    }
};

// ============================================================================
// ROBOTIC ARM TRAJECTORY - Bezier curve for smooth arm movement
// ============================================================================
class RoboticArmTrajectory {
private:
    std::vector<glm::vec3> controlPoints;

public:
    RoboticArmTrajectory() {
        // Define welding robot arm path using Bezier control points
        controlPoints = {
            {-4.0f, 0.5f, -8.0f},     // Home position (low)
            {-2.0f, 1.2f, -6.0f},     // Approach (curved up)
            {0.0f, 2.5f, -4.0f},      // Peak (over workpiece)
            {2.0f, 1.2f, -6.0f},      // Descent (curved down)
            {4.0f, 0.5f, -8.0f}       // Return to home
        };
    }

    glm::vec3 getArmPosition(float phase) {
        return BezierCurve::evaluate(controlPoints, glm::clamp(phase, 0.0f, 1.0f));
    }

    glm::vec3 getArmVelocity(float phase) {
        return BezierCurve::evaluateTangent(controlPoints, glm::clamp(phase, 0.0f, 1.0f));
    }
};

// ============================================================================
// CABLE BETWEEN STRUCTURES - Ruled surface for realistic cable geometry
// ============================================================================
class CableBetweenStructures {
private:
    RuledSurface ruleSurface;
    bool isGenerated;

public:
    CableBetweenStructures() : isGenerated(false) {}

    void generateCableBetween(glm::vec3 pointA, glm::vec3 pointB, float sag = 0.3f) {
        // Create two curves: one straight, one sagging
        std::vector<glm::vec3> curve1, curve2;

        int samples = 30;
        for (int i = 0; i <= samples; i++) {
            float t = (float)i / samples;

            // Straight line (top of cable profile)
            glm::vec3 straight = glm::mix(pointA, pointB, t);
            curve1.push_back(straight);

            // Sagging curve (bottom of cable - forms catenary-like shape)
            glm::vec3 sagging = glm::mix(pointA, pointB, t);
            // Add downward sag based on distance from endpoints
            float sagAmount = sag * sin(t * M_PI);
            sagging.y -= sagAmount;
            curve2.push_back(sagging);
        }

        // Create ruled surface between the two curves
        ruleSurface.generate(curve1, curve2, 8);
        isGenerated = true;
    }

    void draw(Shader& s, glm::mat4 model, glm::vec3 color) {
        if (!isGenerated) return;
        ruleSurface.draw(s, model, color);
    }

    void drawTextured(Shader& s, glm::mat4 model, unsigned int texID, glm::vec3 color) {
        if (!isGenerated) return;
        ruleSurface.drawTextured(s, model, texID, color);
    }
};

// ============================================================================
// ESCALATOR HANDRAIL - Spline curve following escalator path (REALISTIC)
// ============================================================================
class EscalatorHandrail {
private:
    std::vector<glm::vec3> handrailCurve;

public:
    EscalatorHandrail() {
        // ESCALATOR GEOMETRY (from scene_renderer.h)
        // Center: X = -20.0
        // Range:  Z = 0 (north/top) to Z = 8 (south/bottom)
        // Height: Y = 0 to Y = 4.5
        // Angle:  arctan(4.5 / 8.0) ≈ 29.05°
        
        // HANDRAIL PLACEMENT (realistic positioning)
        // Offset: +1.0m to the EAST (positive X direction)
        // This matches real escalators where handrail is 1m from center
        // Height offset: Handrail grips are 0.9m above step surface
        
        // Escalator steps are at Y = 0 to Y = 4.5
        // Handrail rises from 0.9m to 5.4m (1m higher than top step)
        
        // CONTROL POINTS - Derived from escalator geometry
        // Using Z parameter to track position along escalator (8 to 0)
        // X = -20 (center) + 1.0 (offset) = -19.0 ✓
        // Y calculated from Z position: Y = (8 - Z) / 8 * 4.5 + 0.9 (offset)
        
        std::vector<glm::vec3> controlPoints = {
            // Extrapolation point P0 (for Catmull-Rom continuity)
            {-20.5f, 0.2f, 9.0f},
            
            // P1: Bottom of handrail (at first step, Z=8)
            // Y = (8-8)/8 * 4.5 + 0.9 = 0.9
            {-20.5f, 0.9f, 8.0f},
            
            // P2: Middle of handrail (midway, Z=4)
            // Y = (8-4)/8 * 4.5 + 0.9 = 3.15
            {-20.5f, 3.15f, 4.0f},
            
            // P3: Top of handrail (at top step, Z=0)
            // Y = (8-0)/8 * 4.5 + 0.9 = 5.4
            {-20.5f, 5.4f, 0.0f},
            
            // Extrapolation point P4 (for Catmull-Rom continuity)
            {-20.5f, 6.1f, -1.0f}
        };

        // Generate smooth spline - high samples for smooth curve
        handrailCurve = CatmullRomSpline::generateCurve(controlPoints, 80);
    }

    glm::vec3 getHandrailPoint(float position) {
        int idx = (int)(glm::clamp(position, 0.0f, 1.0f) * (handrailCurve.size() - 1));
        return handrailCurve[idx];
    }
};

// ============================================================================
// WELDING SEAM PATH - Smooth Bezier path for robot torch
// ============================================================================
class WeldingSeamPath {
private:
    std::vector<glm::vec3> seamControlPoints;

public:
    WeldingSeamPath() {
        // Define a realistic welding seam (L-shaped, with curves at corners)
        seamControlPoints = {
            {0.0f, 0.0f, 0.0f},       // Start of seam
            {0.5f, 0.0f, 0.2f},       // Approaching first corner
            {1.0f, 0.0f, 0.5f},       // Corner (curve helps smooth transition)
            {1.0f, 0.5f, 1.0f},       // Along vertical edge
            {1.0f, 1.0f, 1.5f}        // End of seam
        };
    }

    glm::vec3 getTorchPosition(float seamProgress) {
        return BezierCurve::evaluate(seamControlPoints, glm::clamp(seamProgress, 0.0f, 1.0f));
    }

    glm::vec3 getTorchDirection(float seamProgress) {
        return glm::normalize(BezierCurve::evaluateTangent(seamControlPoints, glm::clamp(seamProgress, 0.0f, 1.0f)));
    }
};

// ============================================================================
// RAMP/SLIDE SURFACE - Ruled surface for industrial slides
// ============================================================================
class RampSurface {
private:
    RuledSurface ruleSurface;
    bool isGenerated;

public:
    RampSurface() : isGenerated(false) {}

    void generateRamp(glm::vec3 topStart, glm::vec3 topEnd, glm::vec3 bottomStart, glm::vec3 bottomEnd) {
        std::vector<glm::vec3> topEdge, bottomEdge;
        
        int samples = 40;
        for (int i = 0; i <= samples; i++) {
            float t = (float)i / samples;
            topEdge.push_back(glm::mix(topStart, topEnd, t));
            bottomEdge.push_back(glm::mix(bottomStart, bottomEnd, t));
        }
        
        ruleSurface.generate(topEdge, bottomEdge, 15);
        isGenerated = true;
    }

    void draw(Shader& s, glm::mat4 model, glm::vec3 color) {
        if (!isGenerated) return;
        ruleSurface.draw(s, model, color);
    }
};

// ============================================================================
// VISUALIZABLE ESCALATOR HANDRAIL - Tube mesh from curve
// ============================================================================
class VisualizableEscalatorHandrail {
public:
    unsigned int VAO = 0, VBO = 0, EBO = 0;
    int indexCount = 0;
    
    VisualizableEscalatorHandrail() {}
    
    void generateMesh(EscalatorHandrail* rail, int samples = 60, float tubeRadius = 0.08f) {
        std::vector<float> vertices;
        std::vector<unsigned int> indices;
        
        // Sample curve at many points
        std::vector<glm::vec3> curvePoints;
        for (int s = 0; s <= samples; s++) {
            float t = (float)s / samples;
            curvePoints.push_back(rail->getHandrailPoint(t));
        }
        
        // Create tube geometry around curve
        int circleRes = 20;  // 20-sided circle for smoother tube appearance
        
        for (size_t i = 0; i < curvePoints.size(); i++) {
            glm::vec3 p = curvePoints[i];
            
            // Calculate tangent (curve direction)
            glm::vec3 tangent;
            if (i == 0) {
                tangent = curvePoints[1] - p;
            } else if (i == curvePoints.size() - 1) {
                tangent = p - curvePoints[i-1];
            } else {
                tangent = curvePoints[i+1] - curvePoints[i-1];
            }
            tangent = glm::normalize(tangent);
            
            // Create perpendicular frame (Frenet frame)
            glm::vec3 up(0, 1, 0);
            glm::vec3 right = glm::cross(tangent, up);
            
            if (glm::length(right) < 0.001f) {
                right = glm::vec3(1, 0, 0);
            }
            right = glm::normalize(right);
            up = glm::normalize(glm::cross(right, tangent));
            
            // Create circle of vertices around this curve point
            for (int j = 0; j < circleRes; j++) {
                float angle = 2.0f * 3.14159f * j / circleRes;
                
                // Position on circle
                glm::vec3 offset = right * cos(angle) * tubeRadius + 
                                  up * sin(angle) * tubeRadius;
                glm::vec3 vertex = p + offset;
                
                // Position
                vertices.push_back(vertex.x);
                vertices.push_back(vertex.y);
                vertices.push_back(vertex.z);
                
                // Normal (pointing outward)
                vertices.push_back(offset.x / tubeRadius);
                vertices.push_back(offset.y / tubeRadius);
                vertices.push_back(offset.z / tubeRadius);
            }
        }
        
        // Connect circles with triangles
        for (size_t i = 0; i < curvePoints.size() - 1; i++) {
            for (int j = 0; j < circleRes; j++) {
                int v0 = i * circleRes + j;
                int v1 = i * circleRes + (j + 1) % circleRes;
                int v2 = (i + 1) * circleRes + j;
                int v3 = (i + 1) * circleRes + (j + 1) % circleRes;
                
                // Two triangles per quad
                indices.push_back(v0);
                indices.push_back(v2);
                indices.push_back(v1);
                
                indices.push_back(v1);
                indices.push_back(v2);
                indices.push_back(v3);
            }
        }
        
        indexCount = (int)indices.size();
        
        // Upload to GPU
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        
        glBindVertexArray(VAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        
        // Vertex attributes (24 bytes per vertex: pos(12) + normal(12))
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 24, (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 24, (void*)12);
        glEnableVertexAttribArray(1);
        
        glBindVertexArray(0);
    }
    
    void draw(Shader& s, glm::mat4 model, glm::vec3 color) {
        if (VAO == 0 || indexCount == 0) return;
        
        s.use();
        s.setMat4("model", model);
        s.setVec3("material.ambient", color * 0.9f);      // Stronger ambient
        s.setVec3("material.diffuse", color);
        s.setVec3("material.specular", glm::vec3(0.8f));  // Brighter specular (was 0.5f)
        s.setFloat("material.shininess", 128.0f);         // Shinier (was 64.0f)
        s.setBool("useTexture", false);
        
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
};

// ============================================================================
// VISUALIZABLE CONVEYOR PATH - Line visualization with spheres
// ============================================================================
class VisualizableConveyorPath {
public:
    unsigned int VAO = 0, VBO = 0;
    int vertexCount = 0;
    
    VisualizableConveyorPath() {}
    
    void generateVisualization(ConveyorPath* path, int samples = 100, float sphereRadius = 0.05f) {
        std::vector<float> vertices;
        
        // Sample path points
        for (int i = 0; i <= samples; i++) {
            float t = (float)i / samples;
            glm::vec3 p = path->getPointOnPath(t);
            
            // Create vertex for this point
            vertices.push_back(p.x);
            vertices.push_back(p.y);
            vertices.push_back(p.z);
        }
        
        vertexCount = (int)vertices.size() / 3;
        
        // Upload to GPU
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12, (void*)0);
        glEnableVertexAttribArray(0);
        
        glBindVertexArray(0);
    }
    
    void drawLine(Shader& s, glm::mat4 model, glm::vec3 color) {
        if (VAO == 0) return;
        
        s.use();
        s.setMat4("model", model);
        s.setVec3("material.diffuse", color);
        
        glBindVertexArray(VAO);
        glDrawArrays(GL_LINE_STRIP, 0, vertexCount);
        glBindVertexArray(0);
    }
};

#endif /* curveApplications_h */
