//
//  bezierCurve.h
//  Enhanced curve evaluation and mesh generation
//
//  Supports:
//  - Bezier curves (De Casteljau algorithm)
//  - Catmull-Rom spline curves (smooth interpolation)
//  - Revolved surfaces (round furniture, containers)
//  - Extruded profiles (architectural elements)
//  - Ruled surfaces (connecting curves in space)
//

#ifndef bezierCurve_h
#define bezierCurve_h

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <cmath>
#include "shader.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================================
// BEZIER CURVE EVALUATION (De Casteljau Algorithm)
// ============================================================================
class BezierCurve {
public:
    // Standard Bezier curve evaluation using recursive de Casteljau
    static glm::vec3 evaluate(const std::vector<glm::vec3>& controlPoints, float t) {
        std::vector<glm::vec3> pts = controlPoints;
        int n = (int)pts.size();
        for (int r = 1; r < n; r++) {
            for (int i = 0; i < n - r; i++) {
                pts[i] = (1.0f - t) * pts[i] + t * pts[i + 1];
            }
        }
        return pts[0];
    }

    // Evaluate tangent vector (first derivative) at parameter t
    static glm::vec3 evaluateTangent(const std::vector<glm::vec3>& controlPoints, float t) {
        int n = (int)controlPoints.size();
        if (n < 2) return glm::vec3(0, 0, 1);
        std::vector<glm::vec3> derived;
        for (int i = 0; i < n - 1; i++) {
            derived.push_back((float)(n - 1) * (controlPoints[i + 1] - controlPoints[i]));
        }
        return evaluate(derived, t);
    }

    // Evaluate curvature (second derivative) at parameter t
    static glm::vec3 evaluateCurvature(const std::vector<glm::vec3>& controlPoints, float t) {
        int n = (int)controlPoints.size();
        if (n < 3) return glm::vec3(0);
        
        // Build second-order derived control points
        std::vector<glm::vec3> firstDerived;
        for (int i = 0; i < n - 1; i++) {
            firstDerived.push_back((float)(n - 1) * (controlPoints[i + 1] - controlPoints[i]));
        }
        
        std::vector<glm::vec3> secondDerived;
        int m = (int)firstDerived.size();
        for (int i = 0; i < m - 1; i++) {
            secondDerived.push_back((float)(m - 1) * (firstDerived[i + 1] - firstDerived[i]));
        }
        
        return evaluate(secondDerived, t);
    }
};

// ============================================================================
// CATMULL-ROM SPLINE CURVE EVALUATION (Smooth Interpolation)
// ============================================================================
class CatmullRomSpline {
public:
    // Evaluate Catmull-Rom spline at parameter t
    // p0, p1, p2, p3: control points (curve passes through p1 and p2)
    // t: parameter [0, 1] for the segment from p1 to p2
    static glm::vec3 evaluate(const glm::vec3& p0, const glm::vec3& p1, 
                              const glm::vec3& p2, const glm::vec3& p3, float t) {
        float t2 = t * t;
        float t3 = t2 * t;
        
        // Catmull-Rom basis functions
        float f0 = -t3 + 2.0f * t2 - t;
        float f1 =  3.0f * t3 - 5.0f * t2 + 2.0f;
        float f2 = -3.0f * t3 + 4.0f * t2 + t;
        float f3 =  t3 - t2;
        
        return 0.5f * (f0 * p0 + f1 * p1 + f2 * p2 + f3 * p3);
    }

    // Create smooth curve through multiple control points
    static std::vector<glm::vec3> generateCurve(const std::vector<glm::vec3>& controlPoints, int samplesPerSegment) {
        std::vector<glm::vec3> curve;
        
        if (controlPoints.size() < 4) return curve;
        
        int numSegments = controlPoints.size() - 3;
        
        for (int seg = 0; seg < numSegments; seg++) {
            for (int s = 0; s < samplesPerSegment; s++) {
                float t = (float)s / samplesPerSegment;
                glm::vec3 point = evaluate(
                    controlPoints[seg],
                    controlPoints[seg + 1],
                    controlPoints[seg + 2],
                    controlPoints[seg + 3],
                    t
                );
                curve.push_back(point);
            }
        }
        
        curve.push_back(controlPoints[controlPoints.size() - 1]);
        return curve;
    }
};

// ============================================================================
// RULED SURFACE (Connects two curves in space)
// ============================================================================
class RuledSurface {
public:
    unsigned int VAO = 0;
    int indexCount = 0;

    RuledSurface() {}

    // Generate surface by linearly interpolating between two curves
    void generate(const std::vector<glm::vec3>& curve1, const std::vector<glm::vec3>& curve2, int samples = 20) {
        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        if (curve1.size() != curve2.size()) return;

        int numCurvePoints = curve1.size();

        // Create vertices by interpolating between curves
        for (int j = 0; j <= samples; j++) {
            float v = (float)j / samples;

            for (int i = 0; i < numCurvePoints; i++) {
                glm::vec3 p = glm::mix(curve1[i], curve2[i], v);

                // Calculate normal using adjacent points on surface
                glm::vec3 normal(0, 1, 0);
                
                if (i > 0 && i < numCurvePoints - 1) {
                    glm::vec3 tangent = glm::mix(curve1[i + 1], curve2[i + 1], v) - 
                                       glm::mix(curve1[i - 1], curve2[i - 1], v);
                    
                    if (j > 0 && j < samples) {
                        glm::vec3 vmove = glm::mix(curve1[i], curve2[i], (v + 0.05f)) - 
                                         glm::mix(curve1[i], curve2[i], (v - 0.05f));
                        normal = glm::normalize(glm::cross(tangent, vmove));
                    }
                }

                float u = (float)i / (numCurvePoints - 1);

                vertices.push_back(p.x); vertices.push_back(p.y); vertices.push_back(p.z);
                vertices.push_back(normal.x); vertices.push_back(normal.y); vertices.push_back(normal.z);
                vertices.push_back(u); vertices.push_back(v);
            }
        }

        // Generate quads
        for (int j = 0; j < samples; j++) {
            for (int i = 0; i < numCurvePoints - 1; i++) {
                int a = j * numCurvePoints + i;
                int b = a + 1;
                int c = (j + 1) * numCurvePoints + i;
                int d = c + 1;

                indices.push_back(a); indices.push_back(c); indices.push_back(b);
                indices.push_back(b); indices.push_back(c); indices.push_back(d);
            }
        }

        indexCount = (int)indices.size();

        glGenVertexArrays(1, &VAO);
        unsigned int VBO, EBO;
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 32, (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 32, (void*)12);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 32, (void*)24);
        glEnableVertexAttribArray(2);
        glBindVertexArray(0);
    }

    void draw(Shader& s, glm::mat4 model, glm::vec3 color, float shininess = 32.0f) {
        if (VAO == 0 || indexCount == 0) return;
        s.use();
        s.setVec3("material.ambient", color * 0.8f);
        s.setVec3("material.diffuse", color);
        s.setVec3("material.specular", glm::vec3(0.3f));
        s.setFloat("material.shininess", shininess);
        s.setBool("useTexture", false);
        s.setMat4("model", model);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void drawTextured(Shader& s, glm::mat4 model, unsigned int textureID, glm::vec3 color, float tiling = 1.0f, float shininess = 32.0f) {
        if (VAO == 0 || indexCount == 0) return;
        s.use();
        s.setVec3("material.ambient", color * 0.8f);
        s.setVec3("material.diffuse", color);
        s.setVec3("material.specular", glm::vec3(0.3f));
        s.setFloat("material.shininess", shininess);
        s.setBool("useTexture", true);
        s.setFloat("texTiling", tiling);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        s.setInt("texture1", 0);
        s.setMat4("model", model);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        s.setBool("useTexture", false);
    }
};

// ============================================================================
// REVOLVED BEZIER SURFACE (for round table tops, vessels, furniture)
// ============================================================================
class BezierRevolvedSurface {
public:
    unsigned int VAO = 0;
    int indexCount = 0;

    BezierRevolvedSurface() {}

    // profile: control points in XY plane (X = radius, Y = height)
    // segments: curve tessellation, slices: revolution slices
    // Can use either Bezier or Catmull-Rom control points
    void generate(const std::vector<glm::vec3>& profile, int segments, int slices) {
        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        // Sample the profile curve (works with both Bezier and Catmull-Rom)
        std::vector<glm::vec3> profilePts;
        for (int i = 0; i <= segments; i++) {
            float t = (float)i / segments;
            profilePts.push_back(BezierCurve::evaluate(profile, t));
        }

        // Revolve around Y-axis
        for (int j = 0; j <= slices; j++) {
            float theta = 2.0f * (float)M_PI * j / slices;
            float cosT = cosf(theta);
            float sinT = sinf(theta);

            for (int i = 0; i <= segments; i++) {
                glm::vec3 p = profilePts[i];
                float x = p.x * cosT;
                float y = p.y;
                float z = p.x * sinT;

                // Normal approximation: tangent cross revolution direction
                float nx = cosT;
                float ny = 0.0f;
                float nz = sinT;
                if (i > 0 && i < segments) {
                    glm::vec3 tangent = profilePts[i + 1] - profilePts[i - 1];
                    // Revolution normal
                    glm::vec3 revDir(-sinT, 0, cosT);
                    glm::vec3 tanWorld(tangent.x * cosT, tangent.y, tangent.x * sinT);
                    glm::vec3 n = glm::cross(tanWorld, revDir);
                    float len = glm::length(n);
                    if (len > 0.001f) { nx = n.x / len; ny = n.y / len; nz = n.z / len; }
                }

                float u = (float)j / slices;
                float v = (float)i / segments;

                vertices.push_back(x); vertices.push_back(y); vertices.push_back(z);
                vertices.push_back(nx); vertices.push_back(ny); vertices.push_back(nz);
                vertices.push_back(u); vertices.push_back(v);
            }
        }

        // Indices
        for (int j = 0; j < slices; j++) {
            for (int i = 0; i < segments; i++) {
                int a = j * (segments + 1) + i;
                int b = a + 1;
                int c = (j + 1) * (segments + 1) + i;
                int d = c + 1;
                indices.push_back(a); indices.push_back(c); indices.push_back(b);
                indices.push_back(b); indices.push_back(c); indices.push_back(d);
            }
        }
        indexCount = (int)indices.size();

        glGenVertexArrays(1, &VAO);
        unsigned int VBO, EBO;
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 32, (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 32, (void*)12);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 32, (void*)24);
        glEnableVertexAttribArray(2);
        glBindVertexArray(0);
    }

    void draw(Shader& s, glm::mat4 model, glm::vec3 color, float shininess = 32.0f) {
        if (VAO == 0 || indexCount == 0) return;
        s.use();
        s.setVec3("material.ambient", color * 0.8f);
        s.setVec3("material.diffuse", color);
        s.setVec3("material.specular", glm::vec3(0.3f));
        s.setFloat("material.shininess", shininess);
        s.setBool("useTexture", false);
        s.setMat4("model", model);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void drawTextured(Shader& s, glm::mat4 model, unsigned int textureID, glm::vec3 color, float tiling = 1.0f, float shininess = 32.0f) {
        if (VAO == 0 || indexCount == 0) return;
        s.use();
        s.setVec3("material.ambient", color * 0.8f);
        s.setVec3("material.diffuse", color);
        s.setVec3("material.specular", glm::vec3(0.3f));
        s.setFloat("material.shininess", shininess);
        s.setBool("useTexture", true);
        s.setFloat("texTiling", tiling);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        s.setInt("texture1", 0);
        s.setMat4("model", model);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        s.setBool("useTexture", false);
    }
};

// ---- Extruded Bezier Arch (for doorways) ----
// Takes a 2D Bezier profile (arch shape in XY) and extrudes it along Z
class BezierArch {
public:
    unsigned int VAO = 0;
    int indexCount = 0;

    void generate(const std::vector<glm::vec3>& archProfile, int segments, float depth) {
        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        std::vector<glm::vec3> pts;
        for (int i = 0; i <= segments; i++) {
            float t = (float)i / segments;
            pts.push_back(BezierCurve::evaluate(archProfile, t));
        }

        float halfD = depth / 2.0f;
        // Front face (z = +halfD) and back face (z = -halfD)
        for (int face = 0; face < 2; face++) {
            float z = (face == 0) ? halfD : -halfD;
            float nz = (face == 0) ? 1.0f : -1.0f;
            for (int i = 0; i <= segments; i++) {
                float u = (float)i / segments;
                vertices.push_back(pts[i].x); vertices.push_back(pts[i].y); vertices.push_back(z);
                vertices.push_back(0); vertices.push_back(0); vertices.push_back(nz);
                vertices.push_back(u); vertices.push_back((face == 0) ? 0.0f : 1.0f);
            }
        }
        // Top surface connecting front and back
        for (int i = 0; i <= segments; i++) {
            glm::vec3 normal(0, 1, 0);
            if (i > 0 && i < segments) {
                glm::vec3 tang = pts[i + 1] - pts[i - 1];
                normal = glm::normalize(glm::vec3(-tang.y, tang.x, 0));
            }
            float u = (float)i / segments;
            // Front edge
            vertices.push_back(pts[i].x); vertices.push_back(pts[i].y); vertices.push_back(halfD);
            vertices.push_back(normal.x); vertices.push_back(normal.y); vertices.push_back(0);
            vertices.push_back(u); vertices.push_back(0);
            // Back edge
            vertices.push_back(pts[i].x); vertices.push_back(pts[i].y); vertices.push_back(-halfD);
            vertices.push_back(normal.x); vertices.push_back(normal.y); vertices.push_back(0);
            vertices.push_back(u); vertices.push_back(1);
        }

        int frontBase = 0;
        int backBase = segments + 1;
        int topBase = 2 * (segments + 1);

        // Front/back triangle strips
        for (int i = 0; i < segments; i++) {
            // Front face triangles (as a fan from center bottom)
            if (i < segments / 2) {
                indices.push_back(frontBase); indices.push_back(frontBase + i + 1); indices.push_back(frontBase + i + 2);
            }
        }
        for (int i = 0; i < segments; i++) {
            if (i < segments / 2) {
                indices.push_back(backBase + segments); indices.push_back(backBase + segments - i - 1); indices.push_back(backBase + segments - i - 2);
            }
        }

        // Top surface quads
        for (int i = 0; i < segments; i++) {
            int a = topBase + i * 2;
            int b = a + 1;
            int c = a + 2;
            int d = a + 3;
            indices.push_back(a); indices.push_back(c); indices.push_back(b);
            indices.push_back(b); indices.push_back(c); indices.push_back(d);
        }

        indexCount = (int)indices.size();

        glGenVertexArrays(1, &VAO);
        unsigned int VBO, EBO;
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 32, (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 32, (void*)12);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 32, (void*)24);
        glEnableVertexAttribArray(2);
        glBindVertexArray(0);
    }

    void draw(Shader& s, glm::mat4 model, glm::vec3 color, float shininess = 32.0f) {
        if (VAO == 0 || indexCount == 0) return;
        s.use();
        s.setVec3("material.ambient", color * 0.8f);
        s.setVec3("material.diffuse", color);
        s.setVec3("material.specular", glm::vec3(0.3f));
        s.setFloat("material.shininess", shininess);
        s.setBool("useTexture", false);
        s.setMat4("model", model);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
};

#endif /* bezierCurve_h */
