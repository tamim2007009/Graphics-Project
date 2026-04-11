#ifndef RENDER_UTILS_H
#define RENDER_UTILS_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"

extern bool texturesEnabled;

inline void drawCube(unsigned int &v, Shader &s, glm::mat4 pm, glm::vec3 c, glm::vec3 p, glm::vec3 sc, float sh = 32, float a = 1.0f)
{
    s.use();
    glm::mat4 m = glm::translate(pm, p);
    m = glm::scale(m, sc);
    s.setVec3("material.ambient", c * a);
    s.setVec3("material.diffuse", c);
    s.setVec3("material.specular", glm::vec3(0.8f));  // Increased from 0.3f to 0.8f for better specular visibility
    s.setFloat("material.shininess", sh);
    s.setBool("useTexture", false);
    s.setMat4("model", m);
    glBindVertexArray(v);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

inline void setMaterial(Shader &s, glm::vec3 color, float shininess, float specIntensity, float ambientIntensity = 0.6f) {
    s.setVec3("material.ambient", color * ambientIntensity);
    s.setVec3("material.diffuse", color);
    s.setVec3("material.specular", glm::vec3(specIntensity * 2.0f));  // Double the spec intensity for better visibility
    s.setFloat("material.shininess", shininess);
}

inline void drawCubeTextured(unsigned int &v, Shader &s, glm::mat4 pm, glm::vec3 c, glm::vec3 p, glm::vec3 sc, unsigned int tex, float tiling = 1.0f, float sh = 32, float a = 1.0f)
{
    s.use();
    glm::mat4 m = glm::translate(pm, p);
    m = glm::scale(m, sc);
    s.setVec3("material.ambient", c * a);
    s.setVec3("material.diffuse", c);
    s.setVec3("material.specular", glm::vec3(0.8f));  // Increased from 0.3f for better visibility
    s.setFloat("material.shininess", sh);
    if (texturesEnabled && tex != 0)
    {
        s.setBool("useTexture", true);
        s.setFloat("texTiling", tiling);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);
        s.setInt("texture1", 0);
    }
    else
    {
        s.setBool("useTexture", false);
    }
    s.setMat4("model", m);
    glBindVertexArray(v);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    s.setBool("useTexture", false);
}

inline void createGeometryBuffers(unsigned int &cubeVAO, unsigned int &cubeVBO, unsigned int &cubeEBO, unsigned int &lightVAO, unsigned int &texCubeVAO, unsigned int &tcVBO, unsigned int &tcEBO) {
    float cv[] = {
        -.5f, -.5f, -.5f, 0, 0, -1, .5f, -.5f, -.5f, 0, 0, -1, .5f, .5f, -.5f, 0, 0, -1, -.5f, .5f, -.5f, 0, 0, -1,
        .5f, -.5f, -.5f, 1, 0, 0, .5f, .5f, -.5f, 1, 0, 0, .5f, -.5f, .5f, 1, 0, 0, .5f, .5f, .5f, 1, 0, 0,
        -.5f, -.5f, .5f, 0, 0, 1, .5f, -.5f, .5f, 0, 0, 1, .5f, .5f, .5f, 0, 0, 1, -.5f, .5f, .5f, 0, 0, 1,
        -.5f, -.5f, .5f, -1, 0, 0, -.5f, .5f, .5f, -1, 0, 0, -.5f, .5f, -.5f, -1, 0, 0, -.5f, -.5f, -.5f, -1, 0, 0,
        .5f, .5f, .5f, 0, 1, 0, .5f, .5f, -.5f, 0, 1, 0, -.5f, .5f, -.5f, 0, 1, 0, -.5f, .5f, .5f, 0, 1, 0,
        -.5f, -.5f, -.5f, 0, -1, 0, .5f, -.5f, -.5f, 0, -1, 0, .5f, -.5f, .5f, 0, -1, 0, -.5f, -.5f, .5f, 0, -1, 0};
    unsigned int ci[] = {0, 3, 2, 2, 1, 0, 4, 5, 7, 7, 6, 4, 8, 9, 10, 10, 11, 8, 12, 13, 14, 14, 15, 12, 16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20};
    
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glGenBuffers(1, &cubeEBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cv), cv, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ci), ci, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 24, (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 24, (void *)12);
    glEnableVertexAttribArray(1);

    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 24, (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 24, (void *)12);
    glEnableVertexAttribArray(1);

    // Textured Cube VAO
    float tcv[] = {
        -.5f, -.5f, -.5f, 0, 0, -1, 0, 0, .5f, -.5f, -.5f, 0, 0, -1, 1, 0, .5f, .5f, -.5f, 0, 0, -1, 1, 1, -.5f, .5f, -.5f, 0, 0, -1, 0, 1,
        .5f, -.5f, -.5f, 1, 0, 0, 0, 0, .5f, .5f, -.5f, 1, 0, 0, 0, 1, .5f, -.5f, .5f, 1, 0, 0, 1, 0, .5f, .5f, .5f, 1, 0, 0, 1, 1,
        -.5f, -.5f, .5f, 0, 0, 1, 0, 0, .5f, -.5f, .5f, 0, 0, 1, 1, 0, .5f, .5f, .5f, 0, 0, 1, 1, 1, -.5f, .5f, .5f, 0, 0, 1, 0, 1,
        -.5f, -.5f, .5f, -1, 0, 0, 0, 0, -.5f, .5f, .5f, -1, 0, 0, 0, 1, -.5f, .5f, -.5f, -1, 0, 0, 1, 1, -.5f, -.5f, -.5f, -1, 0, 0, 1, 0,
        .5f, .5f, .5f, 0, 1, 0, 1, 1, .5f, .5f, -.5f, 0, 1, 0, 1, 0, -.5f, .5f, -.5f, 0, 1, 0, 0, 0, -.5f, .5f, .5f, 0, 1, 0, 0, 1,
        -.5f, -.5f, -.5f, 0, -1, 0, 0, 0, .5f, -.5f, -.5f, 0, -1, 0, 1, 0, .5f, -.5f, .5f, 0, -1, 0, 1, 1, -.5f, -.5f, .5f, 0, -1, 0, 0, 1};

    glGenVertexArrays(1, &texCubeVAO);
    glGenBuffers(1, &tcVBO);
    glGenBuffers(1, &tcEBO);
    glBindVertexArray(texCubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, tcVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tcv), tcv, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tcEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ci), ci, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 32, (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 32, (void *)12);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 32, (void *)24);
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

#endif
