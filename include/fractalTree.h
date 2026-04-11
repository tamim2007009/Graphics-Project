//
//  fractalTree.h
//  Procedural 3D Fractal Tree Generator
//
//  Generates recursive branching tree structures using cylinder primitives
//  with bark texture on branches and leaf sphere clusters at terminal branches
//

#ifndef fractalTree_h
#define fractalTree_h

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <cmath>
#include "shader.h"

#ifndef M_PI_FT
#define M_PI_FT 3.14159265358979323846
#endif

struct Branch {
    glm::vec3 start;
    glm::vec3 end;
    float radius;
    int depth;
};

struct LeafCluster {
    glm::vec3 position;
    float size;
};

class FractalTree {
public:
    unsigned int branchVAO = 0;
    int branchIndexCount = 0;
    unsigned int leafVAO = 0;
    int leafIndexCount = 0;
    std::vector<LeafCluster> leafClusters;
    unsigned int seed = 0;

    FractalTree() { seed = (unsigned int)rand(); }

    // Generate the fractal tree structure with enhanced realism
    // root: base position, height: trunk height, radius: trunk radius
    // depth: recursion depth (3-5 recommended), branchAngle: degrees
    void generate(glm::vec3 root, float height, float radius, int depth, float branchAngle = 30.0f) {
        std::vector<Branch> branches;
        leafClusters.clear();

        glm::vec3 direction(0, 1, 0);
        generateBranches(root, direction, height, radius, depth, branchAngle, branches, 0);

        buildBranchMesh(branches);
        buildLeafMesh();
    }

    void drawBranches(Shader& s, glm::mat4 model, unsigned int barkTex, bool useTextures) {
        if (branchVAO == 0) return;
        s.use();
        s.setVec3("material.ambient", glm::vec3(0.35f, 0.22f, 0.10f));
        s.setVec3("material.diffuse", glm::vec3(0.42f, 0.30f, 0.16f));
        s.setVec3("material.specular", glm::vec3(0.15f));
        s.setFloat("material.shininess", 16.0f);

        if (useTextures && barkTex != 0) {
            s.setBool("useTexture", true);
            s.setBool("blendWithColor", true);
            s.setFloat("texTiling", 2.0f);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, barkTex);
            s.setInt("texture1", 0);
        } else {
            s.setBool("useTexture", false);
        }

        s.setMat4("model", model);
        glBindVertexArray(branchVAO);
        glDrawElements(GL_TRIANGLES, branchIndexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        s.setBool("useTexture", false);
        s.setBool("blendWithColor", false);
    }

    void drawLeaves(Shader& s, glm::mat4 baseModel, unsigned int leafTex, bool useTextures) {
        if (leafVAO == 0 || leafClusters.empty()) return;

        for (auto& lc : leafClusters) {
            glm::mat4 m = glm::translate(baseModel, lc.position);
            m = glm::scale(m, glm::vec3(lc.size * 0.8f, lc.size * 1.2f, lc.size * 0.85f));

            s.use();
            float variation = fmod(lc.position.x + lc.position.y + lc.position.z, 1.0f);
            float g = 0.40f + variation * 0.25f;
            s.setVec3("material.ambient", glm::vec3(0.12f, g * 0.5f, 0.08f));
            s.setVec3("material.diffuse", glm::vec3(0.18f, g, 0.12f));
            s.setVec3("material.specular", glm::vec3(0.08f));
            s.setFloat("material.shininess", 6.0f);

            if (useTextures && leafTex != 0) {
                s.setBool("useTexture", true);
                s.setBool("blendWithColor", true);
                s.setFloat("texTiling", 1.5f);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, leafTex);
                s.setInt("texture1", 0);
            } else {
                s.setBool("useTexture", false);
            }

            s.setMat4("model", m);
            glBindVertexArray(leafVAO);
            glDrawElements(GL_TRIANGLES, leafIndexCount, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
            s.setBool("useTexture", false);
            s.setBool("blendWithColor", false);
        }
    }

private:
    // Seeded random number generator for consistent but varied results
    float randomFloat(float min, float max) {
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        return min + (seed % 10000) / 10000.0f * (max - min);
    }

    void generateBranches(glm::vec3 start, glm::vec3 dir, float length, float radius,
                          int depth, float branchAngle, std::vector<Branch>& branches, int currentDepth) {
        if (depth <= 0 || length < 0.03f) {
            float leafSize = radius * 4.5f + randomFloat(0.2f, 0.5f);
            leafClusters.push_back({start, leafSize});
            return;
        }

        glm::vec3 end = start + dir * length;
        branches.push_back({start, end, radius, depth});

        // Adaptive reduction: vary reduction factor for natural variation
        float reductionFactor = 0.65f + randomFloat(-0.08f, 0.08f);
        float childLength = length * reductionFactor;
        float childRadius = radius * (0.55f + randomFloat(-0.05f, 0.05f));

        float angleRad = branchAngle * (float)M_PI_FT / 180.0f;

        glm::vec3 up = glm::normalize(dir);
        glm::vec3 right;
        if (fabs(up.y) < 0.99f) {
            right = glm::normalize(glm::cross(up, glm::vec3(0, 1, 0)));
        } else {
            right = glm::normalize(glm::cross(up, glm::vec3(1, 0, 0)));
        }
        glm::vec3 forward = glm::normalize(glm::cross(right, up));

        // Variable number of children: 2-4 based on depth
        int numChildren = (depth > 2) ? (3 + (currentDepth % 2)) : 2;
        for (int i = 0; i < numChildren; i++) {
            float phi = 2.0f * (float)M_PI_FT * i / numChildren + randomFloat(-0.3f, 0.3f);
            float tiltAngle = angleRad + randomFloat(-0.15f, 0.15f);

            glm::vec3 childDir = up * cosf(tiltAngle)
                                 + (right * cosf(phi) + forward * sinf(phi)) * sinf(tiltAngle);
            childDir = glm::normalize(childDir);

            generateBranches(end, childDir, childLength, childRadius, depth - 1,
                           branchAngle + randomFloat(-5.0f, 5.0f), branches, currentDepth + 1);
        }
    }

    void buildBranchMesh(const std::vector<Branch>& branches) {
        std::vector<float> verts;
        std::vector<unsigned int> inds;
        int segments = 8;
        int baseVert = 0;

        for (auto& b : branches) {
            glm::vec3 axis = b.end - b.start;
            float len = glm::length(axis);
            if (len < 0.001f) continue;
            glm::vec3 up = axis / len;

            glm::vec3 right;
            if (fabs(up.y) < 0.99f) right = glm::normalize(glm::cross(up, glm::vec3(0, 1, 0)));
            else right = glm::normalize(glm::cross(up, glm::vec3(1, 0, 0)));
            glm::vec3 fwd = glm::normalize(glm::cross(right, up));

            float topRadius = b.radius * 0.60f;

            for (int ring = 0; ring < 2; ring++) {
                float r = (ring == 0) ? b.radius : topRadius;
                glm::vec3 center = (ring == 0) ? b.start : b.end;
                float v = (float)ring;

                for (int s = 0; s <= segments; s++) {
                    float theta = 2.0f * (float)M_PI_FT * s / segments;
                    float cosT = cosf(theta);
                    float sinT = sinf(theta);
                    glm::vec3 normal = glm::normalize(right * cosT + fwd * sinT);
                    glm::vec3 pos = center + normal * r;
                    float u = (float)s / segments;

                    verts.push_back(pos.x); verts.push_back(pos.y); verts.push_back(pos.z);
                    verts.push_back(normal.x); verts.push_back(normal.y); verts.push_back(normal.z);
                    verts.push_back(u); verts.push_back(v);
                }
            }

            for (int s = 0; s < segments; s++) {
                int a = baseVert + s;
                int b2 = baseVert + s + 1;
                int c = baseVert + (segments + 1) + s;
                int d = baseVert + (segments + 1) + s + 1;
                inds.push_back(a); inds.push_back(c); inds.push_back(b2);
                inds.push_back(b2); inds.push_back(c); inds.push_back(d);
            }

            baseVert += 2 * (segments + 1);
        }

        branchIndexCount = (int)inds.size();
        if (branchIndexCount == 0) return;

        glGenVertexArrays(1, &branchVAO);
        unsigned int VBO, EBO;
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        glBindVertexArray(branchVAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size() * sizeof(unsigned int), inds.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 32, (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 32, (void*)12);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 32, (void*)24);
        glEnableVertexAttribArray(2);
        glBindVertexArray(0);
    }

    void buildLeafMesh() {
        std::vector<float> verts;
        std::vector<unsigned int> inds;
        int sectorCount = 12;
        int stackCount = 8;

        for (int i = 0; i <= stackCount; i++) {
            float stackAngle = (float)M_PI_FT / 2.0f - i * (float)M_PI_FT / stackCount;
            float xy = cosf(stackAngle);
            float y = sinf(stackAngle);

            for (int j = 0; j <= sectorCount; j++) {
                float sectorAngle = 2.0f * (float)M_PI_FT * j / sectorCount;
                float x = xy * cosf(sectorAngle);
                float z = xy * sinf(sectorAngle);

                verts.push_back(x); verts.push_back(y); verts.push_back(z);
                verts.push_back(x); verts.push_back(y); verts.push_back(z);
                verts.push_back((float)j / sectorCount); verts.push_back((float)i / stackCount);
            }
        }

        for (int i = 0; i < stackCount; i++) {
            int k1 = i * (sectorCount + 1);
            int k2 = k1 + sectorCount + 1;
            for (int j = 0; j < sectorCount; j++, k1++, k2++) {
                if (i != 0) {
                    inds.push_back(k1); inds.push_back(k2); inds.push_back(k1 + 1);
                }
                if (i != stackCount - 1) {
                    inds.push_back(k1 + 1); inds.push_back(k2); inds.push_back(k2 + 1);
                }
            }
        }

        leafIndexCount = (int)inds.size();
        if (leafIndexCount == 0) return;

        glGenVertexArrays(1, &leafVAO);
        unsigned int VBO, EBO;
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        glBindVertexArray(leafVAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size() * sizeof(unsigned int), inds.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 32, (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 32, (void*)12);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 32, (void*)24);
        glEnableVertexAttribArray(2);
        glBindVertexArray(0);
    }
};

#endif /* fractalTree_h */
