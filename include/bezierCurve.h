//
//  bezierCurve.h
//  Bezier Curve evaluation and mesh generation for furniture and architecture
//
//  Uses de Casteljau algorithm for curve evaluation
//  Supports: revolved surfaces (tables), extruded profiles (arches, booths)
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

class BezierCurve {
public:
    // De Casteljau algorithm: evaluate a Bezier curve at parameter t
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

    // Evaluate tangent (first derivative) via de Casteljau on derived control points
    static glm::vec3 evaluateTangent(const std::vector<glm::vec3>& controlPoints, float t) {
        int n = (int)controlPoints.size();
        if (n < 2) return glm::vec3(0, 0, 1);
        std::vector<glm::vec3> derived;
        for (int i = 0; i < n - 1; i++) {
            derived.push_back((float)(n - 1) * (controlPoints[i + 1] - controlPoints[i]));
        }
        return evaluate(derived, t);
    }
};

// ---- Revolved Bezier Surface (for round table tops) ----
// Takes a 2D profile curve (in XY plane) and revolves it around the Y-axis
class BezierRevolvedSurface {
public:
    unsigned int VAO = 0;
    int indexCount = 0;

    BezierRevolvedSurface() {}

    // profile: control points in XY plane (X = radius, Y = height)
    // segments: curve tessellation, slices: revolution slices
    void generate(const std::vector<glm::vec3>& profile, int segments, int slices) {
        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        // Sample the profile curve
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
