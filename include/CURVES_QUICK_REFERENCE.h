/**
 * QUICK REFERENCE: CURVE USAGE IN PROJECT
 * ========================================
 * 
 * Fast lookup guide for curve implementations
 */

// ============================================================================
// 1. ACCESSING CURVE OBJECTS IN CODE
// ============================================================================

extern ConveyorPath* conveyorPath;              // Global - use anywhere
extern EscalatorHandrail* escalatorRail;
extern VisualizableEscalatorHandrail* visibleHandrail;
extern VisualizableConveyorPath* visibleConveyorPath;

// Include header to use
#include "../include/curveApplications.h"

// ============================================================================
// 2. COMMON OPERATIONS
// ============================================================================

// Get position on Catmull-Rom path (0.0 to 1.0)
glm::vec3 pos = conveyorPath->getPointOnPath(0.5f);

// Get tangent (direction) on path
glm::vec3 dir = conveyorPath->getTangentAtPath(0.5f);

// Get escalator handrail point
glm::vec3 railPos = escalatorRail->getHandrailPoint(position);

// ============================================================================
// 3. DRAWING CURVE SURFACES
// ============================================================================

// Draw escalator handrail (as visible brass tube)
if (visibleHandrail != nullptr) {
    glm::mat4 railModel = glm::translate(glm::mat4(1), glm::vec3(0, 0, 0));
    visibleHandrail->draw(shaderObject, railModel, glm::vec3(0.85f, 0.65f, 0.25f));
}

// Draw conveyor path visualization (as line trace)
if (visibleConveyorPath != nullptr) {
    glm::mat4 conveyorModel = glm::translate(glm::mat4(1), glm::vec3(0, 0, 0));
    visibleConveyorPath->draw(shaderObject, conveyorModel, glm::vec3(0.5f, 0.5f, 0.5f));
}

// ============================================================================
// 4. CREATING NEW CURVES (EXAMPLES)
// ============================================================================

// Example 1: Create custom Bezier revolved surface
BezierRevolvedSurface* customTable = new BezierRevolvedSurface();
std::vector<glm::vec3> profile = {
    {0.0f, 0.0f, 0.0f},   // Center
    {1.0f, 0.1f, 0.0f},   // Control point 1
    {2.0f, 0.2f, 0.0f},   // Control point 2
    {2.5f, 0.0f, 0.0f}    // Edge
};
customTable->generate(profile, 20, 32);  // 20 segments, 32 slices
customTable->draw(shader, model, color);

// Example 2: Create custom arch
BezierArch* customArch = new BezierArch();
std::vector<glm::vec3> arch = {
    {-2.0f, 0.0f, 0.0f},  // Left base
    {-1.5f, 3.0f, 0.0f},  // Left rise
    {0.0f, 4.0f, 0.0f},   // Top
    {1.5f, 3.0f, 0.0f},   // Right rise
    {2.0f, 0.0f, 0.0f}    // Right base
};
customArch->generate(arch, 30, 0.5f);  // 30 segments, 0.5 depth
customArch->draw(shader, model, color);

// Example 3: Create custom ruled surface
RuledSurface* customSurface = new RuledSurface();
std::vector<glm::vec3> curve1 = {{0,0,0}, {1,0,0}, {2,0,0}};
std::vector<glm::vec3> curve2 = {{0,1,0}, {1,1,0}, {2,1,0}};
customSurface->generate(curve1, curve2, 20);  // 20 interpolation samples
customSurface->draw(shader, model, color);

// ============================================================================
// 5. ANIMATION INTEGRATION
// ============================================================================

// Use conveyor path for box animation
float normalizedPos = (boxPosition + 25.0f) / 50.0f;  // Normalize
glm::vec3 animatedPos = conveyorPath->getPointOnPath(normalizedPos);

// ============================================================================
// 6. CATMULL-ROM SPLINE USAGE
// ============================================================================

// Create custom Catmull-Rom path
std::vector<glm::vec3> waypoints = {
    {0, 0, 0},      // Start
    {5, 2, 0},      // Via point 1
    {10, 5, 0},     // Via point 2
    {15, 2, 0},     // Via point 3
    {20, 0, 0}      // End
};

std::vector<glm::vec3> smoothPath = 
    CatmullRomSpline::generateCurve(waypoints, 50);  // 50 samples per segment

// Use smooth path for animation
for(int i = 0; i < smoothPath.size(); i++) {
    glm::vec3 point = smoothPath[i];
    // Draw/animate at this point
}

// ============================================================================
// 7. BEZIER EVALUATION
// ============================================================================

// Evaluate Bezier curve at specific parameter
std::vector<glm::vec3> controlPts = {{0,0,0}, {1,1,0}, {2,0,0}};
glm::vec3 point = BezierCurve::evaluate(controlPts, 0.5f);  // Midpoint

// Get tangent vector
glm::vec3 tangent = BezierCurve::evaluateTangent(controlPts, 0.5f);

// Get curvature vector
glm::vec3 curvature = BezierCurve::evaluateCurvature(controlPts, 0.5f);

// ============================================================================
// 8. MEMORY MANAGEMENT
// ============================================================================

// Objects are created in initializeSceneObjects()
// Cleanup in main():

delete conveyorPath;
delete escalatorRail;
delete visibleHandrail;
delete visibleConveyorPath;

// ============================================================================
// 9. RENDERING PARAMETERS
// ============================================================================

// Vertex format (32 bytes per vertex)
// - Position: 3 floats (bytes 0-11)
// - Normal: 3 floats (bytes 12-23)
// - TexCoord: 2 floats (bytes 24-31)

// GPU Rendering setup
glBindVertexArray(VAO);
glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

// ============================================================================
// 10. SHADER SETUP FOR CURVES
// ============================================================================

// Before drawing curve
Shader& shader = shaderObject;  // Your shader
shader.use();
shader.setMat4("projection", projectionMatrix);
shader.setMat4("view", viewMatrix);
shader.setMat4("model", modelMatrix);
shader.setVec3("material.ambient", color * 0.8f);
shader.setVec3("material.diffuse", color);
shader.setVec3("material.specular", glm::vec3(0.3f));
shader.setFloat("material.shininess", 32.0f);

// For textured curves
shader.setBool("useTexture", true);
shader.setFloat("texTiling", 1.0f);
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_2D, textureID);
shader.setInt("texture1", 0);

// ============================================================================
// 11. DEBUGGING CURVES
// ============================================================================

// Print curve information
std::cout << "Conveyor path points: " << conveyorPath->getPathLength() << std::endl;

// Test curve evaluation
glm::vec3 testPos = conveyorPath->getPointOnPath(0.0f);  // Start
glm::vec3 testMid = conveyorPath->getPointOnPath(0.5f);  // Middle
glm::vec3 testEnd = conveyorPath->getPointOnPath(1.0f);  // End

// Visualize curve points (debug mode)
for(float t = 0.0f; t <= 1.0f; t += 0.1f) {
    glm::vec3 point = conveyorPath->getPointOnPath(t);
    // Draw debug sphere at point
}

// ============================================================================
// 12. PERFORMANCE TIPS
// ============================================================================

// DON'T:
// - Don't recalculate curves every frame
// - Don't generate new meshes each frame
// - Don't create too many samples

// DO:
// - Cache generated paths
// - Use one-time GPU upload
// - Pre-sample paths for animation
// - Reuse curve objects between frames

// ============================================================================
// 13. COMMON ISSUES & SOLUTIONS
// ============================================================================

// Issue: Curve not visible
// Solution: Check shader setup, check model matrix, verify VAO binding

// Issue: Strange normals
// Solution: Ensure cross product order is correct in generation

// Issue: Memory leak
// Solution: Verify delete is called in main() cleanup

// Issue: Jerky animation
// Solution: Use sufficient samples, check interpolation parameter

// ============================================================================
// 14. KEY CLASSES REFERENCE
// ============================================================================

class BezierCurve {
    static glm::vec3 evaluate(const std::vector<glm::vec3>& cp, float t);
    static glm::vec3 evaluateTangent(const std::vector<glm::vec3>& cp, float t);
    static glm::vec3 evaluateCurvature(const std::vector<glm::vec3>& cp, float t);
};

class CatmullRomSpline {
    static glm::vec3 evaluate(const glm::vec3& p0, p1, p2, p3, float t);
    static std::vector<glm::vec3> generateCurve(const std::vector<glm::vec3>& cp, int samples);
};

class RuledSurface {
    void generate(const std::vector<glm::vec3>& curve1, curve2, int samples);
    void draw(Shader& s, glm::mat4 model, glm::vec3 color);
    void drawTextured(Shader& s, glm::mat4 model, unsigned int textureID, glm::vec3 color);
};

class BezierRevolvedSurface {
    void generate(const std::vector<glm::vec3>& profile, int segments, int slices);
    void draw(Shader& s, glm::mat4 model, glm::vec3 color);
    void drawTextured(Shader& s, glm::mat4 model, unsigned int textureID, glm::vec3 color);
};

class BezierArch {
    void generate(const std::vector<glm::vec3>& profile, int segments, float depth);
    void draw(Shader& s, glm::mat4 model, glm::vec3 color);
};

// ============================================================================
// 15. QUICK START - MINIMAL EXAMPLE
// ============================================================================

// In initializeSceneObjects():
conveyorPath = new ConveyorPath();

// In rendering loop:
glm::vec3 animatedPosition = conveyorPath->getPointOnPath(normalizedTime);
modelMatrix = glm::translate(glm::mat4(1), animatedPosition);
drawObject(shader, modelMatrix);

// That's it! Smooth animation using spline curve.

// ============================================================================
*/
