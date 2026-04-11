
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../include/shader.h"
#include "../include/basic_camera.h"
#include <cmath>
#include "../include/pointLight.h"
#include "../include/directionalLight.h"
#include "../include/spotLight.h"
#include "../include/sphere.h"
#include "../include/pyramid.h"
#include "../include/bezierCurve.h"
#include "../include/fractalTree.h"
#include "../include/stb_image.h"
#include "../include/texture_fallback_system.h"
#include "../include/enhanced_signboard.h"
#include "../include/texture_paths.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
using namespace std;

// ====================== COMPLEX MACHINES NAMESPACE ======================
#include "../include/complex_machine.h"

// ---- Forward Declarations ----
void framebuffer_size_callback(GLFWwindow *, int, int);
void key_callback(GLFWwindow *, int, int, int, int);
void mouse_callback(GLFWwindow *, double, double);
void scroll_callback(GLFWwindow *, double, double);
void processInput(GLFWwindow *);
void updateAnimations(float deltaTime);

#include "../include/resource_utils.h"

// Settings
unsigned int SCR_WIDTH = 1400, SCR_HEIGHT = 900;
float lastX = 700, lastY = 450;
bool firstMouse = true;
BasicCamera basic_camera(0, 1.7f, 50, 0, 1.7f, 30, glm::vec3(0, 1, 0));
float deltaTime = 0, lastFrame = 0;

// Mall Dimensions (from map)
//  Mall: X: -25 to 25, Z: -25 to 25, Y: 0 to 5
//  N-S corridor: X: -3 to 3
//  E-W corridor: Z: 10 to 16 (divides shops from back areas)
//  Entrance: south wall Z=25, gap X:-4 to 4
extern const float MH = 5.0f; // mall height
extern const float WT = 0.3f; // wall thickness

#include "../include/scene_colors.h"

//  Lights
extern const int NUM_POINT_LIGHTS = 14;
glm::vec3 plPos[NUM_POINT_LIGHTS] = {
    // N-S corridor
    {0, 4.5f, 18},
    {0, 4.5f, 4},
    // Tech Shop (SW)
    {-14, 4.5f, 16},
    // Clothing Shop (SE)
    {14, 4.5f, 16},
    // Escalator + Staircase area
    {-20, 4.5f, 4},
    {20, 4.5f, 4},
    // Prayer Room
    {-14, 4.5f, -10},
    // Gems Shop
    {14, 4.5f, -10},
    // Wash Room + Lift
    {-14, 4.5f, -3},
    {9, 4.5f, -3},
    // Food court
    {-10, 4.5f, -19},
    {10, 4.5f, -19},
    // Parking lot
    {-10, 6.2f, 42},
    {10, 6.2f, 42}};
PointLight *pointLights[NUM_POINT_LIGHTS];
DirectionalLight dirLight(
    glm::vec3(-10, 6.2f, 42), glm::vec3(0, -1, 0),
    glm::vec3(0.08f, 0.07f, 0.03f), glm::vec3(0.9f, 0.75f, 0.4f),
    glm::vec3(0.7f, 0.55f, 0.3f), 35.f);
DirectionalLight dirLight2(
    glm::vec3(10, 6.2f, 42), glm::vec3(0, -1, 0),
    glm::vec3(0.08f, 0.07f, 0.03f), glm::vec3(0.9f, 0.75f, 0.4f),
    glm::vec3(0.7f, 0.55f, 0.3f), 35.f);
int activeViewport = 0;

// Spot Lights (store display highlights)
extern const int NUM_SPOT_LIGHTS = 4;
SpotLight* spotLights_arr[NUM_SPOT_LIGHTS] = {};

// Texture wrap/filter controls
GLint currentWrapMode = GL_REPEAT;        // GL_REPEAT, GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT
GLint currentFilterMode = GL_LINEAR;      // GL_LINEAR, GL_NEAREST
GLint currentMipmapFilter = GL_LINEAR_MIPMAP_LINEAR; // for MIN_FILTER
bool useMipmapping = true;
const char* wrapModeNames[] = {"GL_REPEAT", "GL_CLAMP_TO_EDGE", "GL_MIRRORED_REPEAT"};
const char* filterModeNames[] = {"GL_LINEAR", "GL_NEAREST"};
GLint wrapModes[] = {GL_REPEAT, GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT};
GLint filterModes[] = {GL_LINEAR, GL_NEAREST};
int currentWrapIndex = 0;
int currentFilterIndex = 0;

// Spheres
Sphere *sphWheel = nullptr;
Sphere *sphHub = nullptr;
Sphere *sphLeaf[5] = {};

// Pyramid
Pyramid *treeCone = nullptr;

// Fractal Trees
extern const int NUM_TREES = 10;
FractalTree* fractalTrees[NUM_TREES] = {};

// Bezier furniture objects
BezierRevolvedSurface* bezierTable = nullptr;
BezierArch* bezierArch = nullptr;
BezierRevolvedSurface* bezierColorChamber = nullptr;
BezierRevolvedSurface* mathCone = nullptr; // True mathematical curvy Cone

#include "../include/scene_renderer.h"
#include "../include/scene_state.h"

// Signboards for displaying information
std::vector<EnhancedSignboard> signboards;

// ---- Initialization Helpers ----
void printControls() {
    cout << "\n==============================================================" << endl;
    cout << "        FACTORY INTERIOR SIMULATION - ENHANCED TEXTURING" << endl;
    cout << "==============================================================" << endl;
    
    cout << "\n--- CAMERA & GENERAL ---" << endl;
    cout << "  W/A/S/D      Move | Mouse Look | ESC Exit" << endl;
    cout << "  C            Toggle Cursor (Mouse Lock)" << endl;
    cout << "  F1-F4        Fullscreen viewport toggle" << endl;
    
    cout << "\n--- LIGHTING CONTROLS ---" << endl;
    cout << "  1            Toggle Ambient Light" << endl;
    cout << "  2            Toggle Point Lights" << endl;
    cout << "  3            Toggle Directional Light (Sun/Lamps)" << endl;
    cout << "  5            Toggle Spot Lights (Store displays)" << endl;
    cout << "  L            Toggle No Light Condition (Unlit Texture)" << endl;
    cout << "  H            Toggle Shading Mode (Phong <-> Gouraud)" << endl;
    
    cout << "\n--- TEXTURE CONTROLS (ADVANCED) ---" << endl;
    cout << "  T            Toggle Textures ON/OFF" << endl;
    cout << "  U            Cycle Wrap Mode (REPEAT -> CLAMP_TO_EDGE -> MIRRORED)" << endl;
    cout << "  Y            Cycle Filter Mode (LINEAR <-> NEAREST)" << endl;
    cout << "  M            Toggle Mipmapping (Linear <-> Nearest)" << endl;
    
    cout << "\n--- ENVIRONMENT CONTROLS ---" << endl;
    cout << "  N            Toggle Day/Night Cycle" << endl;
    cout << "  E            Elevator Up/Down" << endl;
    cout << "  R            Escalator Direction (Up/Down/Pause)" << endl;
    cout << "  B            Boom Barrier Open/Close" << endl;
    cout << "  G            Open/Close Entrance Door" << endl;
    
    cout << "\n--- ASSIGNMENT FEATURES EXHIBITION (Inside SE Corner, X:21, Z:21) ---" << endl;
    cout << "  6            Toggle Exhibition Stand Visibility" << endl;
    cout << "  7            Toggle Texture: Simple vs Blended with Surface Color" << endl;
    cout << "  8            Toggle Shading: Fragment (Phong) vs Vertex (Gouraud)" << endl;
    cout << "  9            Toggle Curvy Object Type: Sphere vs Cone" << endl;
    
    cout << "\n--- FACTORY CONVEYOR CONTROLS ---" << endl;
    cout << "  SPACE        Start/Stop Conveyor" << endl;
    cout << "  + / -        Speed Up/Slow Down Conveyor" << endl;
    cout << "  P            Start/Stop Packaging System" << endl;
    cout << "  [ / ]        Adjust Box Creation Rate" << endl;
    
    cout << "\n--- TIPS FOR BEST REALISM ---" << endl;
    cout << "  - Disable textures (T key) to see pure lighting" << endl;
    cout << "  - Use linear mipmapping (M key) for smooth surfaces" << endl;
    cout << "  - Enable all lights for full scene illumination" << endl;
    cout << "  - Watch hydraulic press timing in manufacturing zone" << endl;
    cout << "  - Observe welding robot torch glow during active welding" << endl;
    cout << "\n" << endl;
}

GLFWwindow* initializeGraphics() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Factory Interior", NULL, NULL);
    if (!window) return NULL;
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return NULL;
    glEnable(GL_DEPTH_TEST);
    return window;
}

unsigned int loadTexture(const char *path);
void createGeometryBuffers(unsigned int &cubeVAO, unsigned int &cubeVBO, unsigned int &cubeEBO, unsigned int &lightVAO, unsigned int &texCubeVAO, unsigned int &tcVBO, unsigned int &tcEBO);

void loadSceneTextures() {
    cout << "\n--- LOADING TEXTURES ---" << endl;
    stbi_set_flip_vertically_on_load(true);
    
    // Primary textures (these should load from resources)
    texFloor = loadTexture(findResource(TexturePaths::FloorTiles).c_str());
    texFashion = loadTexture(findResource(TexturePaths::FashionShop).c_str());
    texTech = loadTexture(findResource(TexturePaths::TechShop).c_str());
    texGems = loadTexture(findResource(TexturePaths::GemsShop).c_str());
    texFood = loadTexture(findResource(TexturePaths::FoodCourt).c_str());
    texTreeLeaf = loadTexture(findResource(TexturePaths::TreeLeaves).c_str());
    texTreeBark = loadTexture(findResource(TexturePaths::TreeBark).c_str());
    texGrass = loadTexture(findResource(TexturePaths::Grass).c_str());
    
    cout << "\n--- GENERATING PROCEDURAL TEXTURES (ENHANCED REALISM) ---" << endl;
    
    // Optional textures - use procedural generation directly
    cout << "  -> Generating procedural brick texture..." << endl;
    texBrick = TextureFallbackSystem::generateBrickTexture();
    
    cout << "  -> Generating procedural concrete texture..." << endl;
    texConcrete = TextureFallbackSystem::generateConcreteTexture();
    
    cout << "  -> Generating procedural metal texture..." << endl;
    texMetal = TextureFallbackSystem::generateMetalTexture();
    
    cout << "  -> Generating procedural wood texture..." << endl;
    texWood = TextureFallbackSystem::generateWoodTexture();
    
    cout << "  -> Generating procedural roof texture..." << endl;
    texRoof = TextureFallbackSystem::generateRoofTileTexture();
    
    cout << "  -> Generating procedural asphalt texture..." << endl;
    texAsphalt = TextureFallbackSystem::generateAsphaltTexture();
    
    cout << "\n--- TEXTURE LOADING COMPLETE ---\n" << endl;
}

void initializeSceneObjects() {
    for (int i = 0; i < NUM_POINT_LIGHTS; i++)
    {
        glm::vec3 a(0.1f), d(0.6f, 0.58f, 0.55f), sp(0.5f);
        pointLights[i] = new PointLight(plPos[i].x, plPos[i].y, plPos[i].z, a.x, a.y, a.z, d.x, d.y, d.z, sp.x, sp.y, sp.z, 1, .09f, .032f, i);
    }

    // Create Spot Lights (store display highlights)
    // Tech store - 2 spots aimed at display tables
    spotLights_arr[0] = new SpotLight(
        glm::vec3(-10.5f, 4.5f, 19), glm::vec3(0, -1, 0),
        glm::vec3(0.05f), glm::vec3(0.9f, 0.85f, 0.8f), glm::vec3(0.8f),
        15.0f, 25.0f, 1.0f, 0.09f, 0.032f, 0);
    spotLights_arr[1] = new SpotLight(
        glm::vec3(-5.5f, 4.5f, 22), glm::vec3(0, -1, 0),
        glm::vec3(0.05f), glm::vec3(0.9f, 0.85f, 0.8f), glm::vec3(0.8f),
        15.0f, 25.0f, 1.0f, 0.09f, 0.032f, 1);
    // Gems store - spot on display cases
    spotLights_arr[2] = new SpotLight(
        glm::vec3(9, 4.5f, 20), glm::vec3(0, -1, 0),
        glm::vec3(0.05f), glm::vec3(1.0f, 0.95f, 0.7f), glm::vec3(0.9f),
        12.0f, 22.0f, 1.0f, 0.09f, 0.032f, 2);
    // Bookstore reading area
    spotLights_arr[3] = new SpotLight(
        glm::vec3(20, 4.5f, 23), glm::vec3(0, -1, 0.1f),
        glm::vec3(0.04f), glm::vec3(0.85f, 0.8f, 0.65f), glm::vec3(0.5f),
        18.0f, 28.0f, 1.0f, 0.09f, 0.032f, 3);

    // Create sphere objects for wheels and leaves
    sphWheel = new Sphere(1.0f, 18, 9, glm::vec3(.12f), glm::vec3(.12f), glm::vec3(.2f), 16);
    sphHub = new Sphere(1.0f, 12, 6, glm::vec3(.7f), glm::vec3(.72f), glm::vec3(.9f), 64);
    glm::vec3 leafG[] = {{.22f, .55f, .18f}, {.18f, .48f, .15f}, {.25f, .58f, .20f}, {.20f, .50f, .16f}, {.15f, .45f, .14f}};
    for (int i = 0; i < 5; i++)
        sphLeaf[i] = new Sphere(1.0f, 12, 8, leafG[i], leafG[i], glm::vec3(.15f), 8);

    // Create tree cone pyramid
    treeCone = new Pyramid(texTreeLeaf, glm::vec3(.22f, .55f, .18f), glm::vec3(.22f, .55f, .18f), glm::vec3(.1f), 8.f);

    // Create Fractal Trees for outdoor planters
    glm::vec3 treePositions[] = {{-20, 0, 28}, {22, 0, 28}, {-36, 0, 15}, {-36, 0, -10}, {36, 0, 12}, {36, 0, -15}, {-16, 0, 62}, {16, 0, 62}, {-45, 0, 40}, {45, 0, -18}};
    for (int i = 0; i < NUM_TREES; i++) {
        fractalTrees[i] = new FractalTree();
        // Vary tree parameters slightly for natural look
        float height = 3.5f + (i % 3) * 0.5f;
        float radius = 0.2f + (i % 2) * 0.05f;
        int depth = 3 + (i % 2); // depth 3 or 4
        float angle = 28.0f + (i % 3) * 4.0f;
        fractalTrees[i]->generate(glm::vec3(0), height, radius, depth, angle);
    }

    // Create Bezier rounded table top (profile: flat disk shape)
    bezierTable = new BezierRevolvedSurface();
    {
        std::vector<glm::vec3> tableProfile = {
            {0.0f, 0.0f, 0.0f},   // center
            {0.3f, 0.02f, 0.0f},  // slight curve up
            {0.5f, 0.03f, 0.0f},  // mid
            {0.55f, 0.0f, 0.0f}   // edge curves down
        };
        bezierTable->generate(tableProfile, 12, 16);
    }

    // Create Bezier arch for doorways
    bezierArch = new BezierArch();
    {
        std::vector<glm::vec3> archProfile = {
            {-1.5f, 0.0f, 0.0f},  // left base
            {-1.5f, 3.5f, 0.0f},  // left rise
            {0.0f, 4.5f, 0.0f},   // top peak
            {1.5f, 3.5f, 0.0f},   // right rise
            {1.5f, 0.0f, 0.0f}    // right base
        };
        bezierArch->generate(archProfile, 20, 0.3f);
    }

    // Create Bezier cylinder for realistic color application belt
    bezierColorChamber = new BezierRevolvedSurface();
    {
        std::vector<glm::vec3> chamberProfile = {
            {1.0f, -1.8f, 0.0f},  // Entrance radius 1.0, height -1.8 (will be X after rotation)
            {1.4f, -0.6f, 0.0f},  // Bulging belly for machine internals
            {1.4f,  0.6f, 0.0f},  // Bulging belly for machine internals
            {1.0f,  1.8f, 0.0f}   // Exit radius 1.0, height 1.8
        };
        bezierColorChamber->generate(chamberProfile, 20, 32);
    }
    
    // Create true mathematical curvy Cone for exhibition
    mathCone = new BezierRevolvedSurface();
    {
        // Simple straight profile representing a cone radius 1 -> 0, height 0 -> 2
        std::vector<glm::vec3> coneProfile = {
            {1.0f, 0.0f, 0.0f}, // Base
            {0.0f, 2.0f, 0.0f}  // Tip
        };
        mathCone->generate(coneProfile, 2, 32); // Minimal segments, high slices for smooth curve revolution
    }

    // Initialize box system
    for (int i = 0; i < MAX_BOXES; i++) {
        boxes[i].active = false;
        boxes[i].position = 0.0f;
        boxes[i].color = glm::vec3(0.8f, 0.3f, 0.15f);
        boxes[i].creationTime = 0.0f;
        boxes[i].rotation = 0.0f;
        boxes[i].vx = 0.0f;
        boxes[i].vz = 0.0f;
        boxes[i].textureID = 0;
    }
    
    // Initialize signboards for different factory areas
    signboards.push_back(EnhancedSignboard(
        {-22, 3.3f, 6}, {1.0f, 0.6f, 0.05f}, "STAMPING",
        {0.2f, 0.2f, 0.22f}, {1.0f, 1.0f, 1.0f}, {0.55f, 0.56f, 0.58f}
    ));
    signboards.push_back(EnhancedSignboard(
        {-10, 3.0f, 13}, {1.0f, 0.6f, 0.05f}, "WELDING",
        {0.2f, 0.2f, 0.22f}, {1.0f, 0.7f, 0.0f}, {0.55f, 0.56f, 0.58f}
    ));
    signboards.push_back(EnhancedSignboard(
        {-18, 1.8f, -6}, {0.9f, 0.5f, 0.05f}, "INSPECT",
        {0.15f, 0.15f, 0.18f}, {0.0f, 1.0f, 0.8f}, {0.5f, 0.5f, 0.52f}
    ));
    signboards.push_back(EnhancedSignboard(
        {-2, 1.8f, -5}, {0.9f, 0.5f, 0.05f}, "PALLET",
        {0.15f, 0.15f, 0.18f}, {1.0f, 1.0f, 0.0f}, {0.5f, 0.5f, 0.52f}
    ));
    signboards.push_back(EnhancedSignboard(
        {15, 2.0f, -6}, {0.9f, 0.5f, 0.05f}, "WRAP",
        {0.15f, 0.15f, 0.18f}, {0.0f, 1.0f, 1.0f}, {0.5f, 0.5f, 0.52f}
    ));
    signboards.push_back(EnhancedSignboard(
        {20, 1.8f, -5}, {1.0f, 0.5f, 0.05f}, "SHIP",
        {0.15f, 0.15f, 0.18f}, {0.0f, 0.8f, 1.0f}, {0.5f, 0.5f, 0.52f}
    ));
    signboards.push_back(EnhancedSignboard(
        {14, 2.0f, 12}, {1.0f, 0.5f, 0.05f}, "ASSEMBLY",
        {0.15f, 0.15f, 0.18f}, {0.0f, 1.0f, 0.2f}, {0.5f, 0.5f, 0.52f}
    ));
    signboards.push_back(EnhancedSignboard(
        {-8.0f, 2.3f, 14.2f}, {1.2f, 0.5f, 0.05f}, "COLOR APPLIED",
        {0.1f, 0.12f, 0.16f}, {0.2f, 0.95f, 1.0f}, {0.5f, 0.55f, 0.58f}
    ));
    signboards.push_back(EnhancedSignboard(
        {15.0f, 2.4f, -8.2f}, {1.0f, 0.5f, 0.05f}, "PACKED",
        {0.12f, 0.12f, 0.15f}, {1.0f, 1.0f, 1.0f}, {0.5f, 0.55f, 0.58f}
    ));
    signboards.push_back(EnhancedSignboard(
        {20.0f, 2.6f, -2.8f}, {1.2f, 0.5f, 0.05f}, "GATHERED",
        {0.12f, 0.12f, 0.15f}, {0.0f, 1.0f, 0.7f}, {0.5f, 0.55f, 0.58f}
    ));
}

void createGeometryBuffersIGNORE(unsigned int &cubeVAO, unsigned int &cubeVBO, unsigned int &cubeEBO, unsigned int &lightVAO, unsigned int &texCubeVAO, unsigned int &tcVBO, unsigned int &tcEBO) {
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

// MAIN
int main()
{
    GLFWwindow *window = initializeGraphics();
    if (!window)
    {
        cout << "Graphics initialization failed" << endl;
        glfwTerminate();
        return -1;
    }
    
    printControls();

    Shader ls(findResource("vertexShaderForPhongShading.vs").c_str(), findResource("fragmentShaderForPhongShading.fs").c_str());
    Shader fs(findResource("vertexShader.vs").c_str(), findResource("fragmentShader.fs").c_str());

    unsigned int cubeVAO, cubeVBO, cubeEBO, lightVAO, tcVBO, tcEBO;
    createGeometryBuffers(cubeVAO, cubeVBO, cubeEBO, lightVAO, texCubeVAO, tcVBO, tcEBO);

    loadSceneTextures();
    initializeSceneObjects();

    while (!glfwWindowShouldClose(window))
    {
        float t = (float)glfwGetTime();
        deltaTime = t - lastFrame;
        lastFrame = t;
        processInput(window);

        updateAnimations(deltaTime);

        // Compute dayNightMix: 0 = full day (noon), 1 = full night (midnight)
        // Smooth transition: day 6-18, night 20-4, transitions at 4-6 and 18-20
        float dnMix = 0.0f;
        if (timeOfDay >= 6.0f && timeOfDay < 18.0f) {
            dnMix = 0.0f; // full day
        } else if (timeOfDay >= 20.0f || timeOfDay < 4.0f) {
            dnMix = 1.0f; // full night
        } else if (timeOfDay >= 18.0f && timeOfDay < 20.0f) {
            dnMix = (timeOfDay - 18.0f) / 2.0f; // dusk transition
        } else { // 4 to 6
            dnMix = 1.0f - (timeOfDay - 4.0f) / 2.0f; // dawn transition
        }

        // Sky color interpolation
        glm::vec3 daySky(0.53f, 0.81f, 0.98f);
        glm::vec3 nightSky(0.02f, 0.02f, 0.08f);
        glm::vec3 skyColor = glm::mix(daySky, nightSky, dnMix);
        glClearColor(skyColor.r, skyColor.g, skyColor.b, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Animations updated above -> updateAnimations(deltaTime)

        // Elevator state machine update
        elevatorTimer += deltaTime;
        switch (elevatorState) {
        case ELEV_STOPPED_BOTTOM:
        case ELEV_STOPPED_TOP:
            break; // waiting for key press
        case ELEV_DOOR_CLOSING_UP:
        case ELEV_DOOR_CLOSING_DOWN:
            elevatorDoorOffset = glm::max(elevatorDoorOffset - deltaTime * 1.5f, 0.0f);
            if (elevatorDoorOffset <= 0.0f) {
                elevatorState = (elevatorState == ELEV_DOOR_CLOSING_UP) ? ELEV_MOVING_UP : ELEV_MOVING_DOWN;
                elevatorTimer = 0.0f;
            }
            break;
        case ELEV_MOVING_UP:
            elevatorY = glm::min(elevatorY + deltaTime * 1.5f, MH);
            if (elevatorY >= MH) {
                elevatorState = ELEV_STOPPED_TOP;
                elevatorDoorOffset = 0.0f;
                elevatorTimer = 0.0f;
                // Auto-open door at top
                elevatorState = ELEV_DOOR_OPENING;
            }
            break;
        case ELEV_MOVING_DOWN:
            elevatorY = glm::max(elevatorY - deltaTime * 1.5f, 0.0f);
            if (elevatorY <= 0.0f) {
                elevatorState = ELEV_STOPPED_BOTTOM;
                elevatorDoorOffset = 0.0f;
                elevatorTimer = 0.0f;
                elevatorState = ELEV_DOOR_OPENING;
            }
            break;
        case ELEV_DOOR_OPENING:
            elevatorDoorOffset = glm::min(elevatorDoorOffset + deltaTime * 1.5f, 1.0f);
            if (elevatorDoorOffset >= 1.0f) {
                elevatorState = (elevatorY >= MH) ? ELEV_STOPPED_TOP : ELEV_STOPPED_BOTTOM;
            }
            break;
        }

        // ---- Shader Setup ----
        ls.use();
        ls.setVec3("viewPos", basic_camera.eye);
        ls.setBool("noLightCondition", noLightCondition);
        ls.setInt("shadingMode", shadingMode);
        
        // Enhanced ambient lighting based on day/night cycle
        glm::vec3 ambientDay(0.15f, 0.15f, 0.18f);
        glm::vec3 ambientNight(0.03f, 0.03f, 0.05f);
        glm::vec3 ambientColor = glm::mix(ambientDay, ambientNight, dnMix);
        ls.setVec3("globalAmbient", ambientEnabled ? ambientColor : glm::vec3(0));
        
        ls.setFloat("dayNightMix", dnMix);
        ls.setInt("numPointLights", pointLightsEnabled ? NUM_POINT_LIGHTS : 0);
        if (pointLightsEnabled)
        {
            for (int i = 0; i < NUM_POINT_LIGHTS; i++)
                pointLights[i]->setUpPointLight(ls);
        }

        // Sun directional light interpolation (warm day → cool night)
        glm::vec3 sunDiffDay(0.9f, 0.75f, 0.4f);
        glm::vec3 sunDiffNight(0.05f, 0.05f, 0.12f);
        glm::vec3 sunSpecDay(0.7f, 0.55f, 0.3f);
        glm::vec3 sunSpecNight(0.02f, 0.02f, 0.05f);
        dirLight.diffuse = glm::mix(sunDiffDay, sunDiffNight, dnMix);
        dirLight.specular = glm::mix(sunSpecDay, sunSpecNight, dnMix);
        dirLight2.diffuse = dirLight.diffuse;
        dirLight2.specular = dirLight.specular;

        if (dirLightEnabled)
            dirLight.turnOn();
        else
            dirLight.turnOff();
        dirLight.setUpDirectionalLight(ls);
        if (dirLightEnabled)
            dirLight2.turnOn();
        else
            dirLight2.turnOff();
        dirLight2.setUpDirectionalLight(ls, "dirLight2");

        // Spot lights
        ls.setInt("numSpotLights", spotLightsEnabled ? NUM_SPOT_LIGHTS : 0);
        if (spotLightsEnabled) {
            for (int i = 0; i < NUM_SPOT_LIGHTS; i++)
                spotLights_arr[i]->setUpSpotLight(ls);
        }

        ls.setBool("useTexture", false);
        ls.setBool("blendWithColor", false);
        ls.setFloat("texTiling", 1.0f);

        int w = SCR_WIDTH, h = SCR_HEIGHT, hw = w / 2, hh = h / 2;
        float os = 40.f;
        glm::mat4 oP = glm::ortho(-os, os, -os * ((float)hh / hw), os * ((float)hh / hw), 0.1f, 300.f);
        glm::mat4 pP = glm::perspective(glm::radians(basic_camera.Zoom), (float)hw / (float)hh, 0.1f, 300.f);
        glm::vec3 gc(0, MH * .5f, 15);
        glm::mat4 vF = glm::lookAt(glm::vec3(0, MH * .5f, 90), gc, {0, 1, 0});
        glm::mat4 vS = glm::lookAt(glm::vec3(90, MH * .5f, 15), gc, {0, 1, 0});
        glm::mat4 vT = glm::lookAt(glm::vec3(0, 90, 15.01f), gc, {0, 0, -1});
        glm::mat4 vP = basic_camera.createViewMatrix();

        glEnable(GL_SCISSOR_TEST);
        if (activeViewport == 0)
        {
            glViewport(0, hh, hw, hh);
            glScissor(0, hh, hw, hh);
            glClear(GL_DEPTH_BUFFER_BIT);
            drawScene(cubeVAO, lightVAO, ls, fs, vF, oP);
            glViewport(hw, hh, hw, hh);
            glScissor(hw, hh, hw, hh);
            glClear(GL_DEPTH_BUFFER_BIT);
            drawScene(cubeVAO, lightVAO, ls, fs, vS, oP);
            glViewport(0, 0, hw, hh);
            glScissor(0, 0, hw, hh);
            glClear(GL_DEPTH_BUFFER_BIT);
            drawScene(cubeVAO, lightVAO, ls, fs, vT, oP);
            glViewport(hw, 0, hw, hh);
            glScissor(hw, 0, hw, hh);
            glClear(GL_DEPTH_BUFFER_BIT);
            drawScene(cubeVAO, lightVAO, ls, fs, vP, pP);
            glDisable(GL_SCISSOR_TEST);
            fs.use();
            fs.setMat4("projection", glm::ortho(0.f, (float)w, 0.f, (float)h, -1.f, 1.f));
            fs.setMat4("view", glm::mat4(1));
            fs.setVec3("color", glm::vec3(0.3f));
            glViewport(0, 0, w, h);
            glBindVertexArray(lightVAO);
            glm::mat4 m = glm::translate(glm::mat4(1), {w * .5f, (float)hh, 0});
            m = glm::scale(m, {(float)w, 2, 1});
            fs.setMat4("model", m);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            m = glm::translate(glm::mat4(1), {(float)hw, h * .5f, 0});
            m = glm::scale(m, {2, (float)h, 1});
            fs.setMat4("model", m);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        }
        else
        {
            float fa = (float)w / h;
            glm::mat4 sv, sp;
            glm::mat4 fO = glm::ortho(-os, os, -os / fa, os / fa, .1f, 300.f);
            glm::mat4 fP = glm::perspective(glm::radians(basic_camera.Zoom), fa, .1f, 300.f);
            switch (activeViewport)
            {
            case 1:
                sv = vF;
                sp = fO;
                break;
            case 2:
                sv = vS;
                sp = fO;
                break;
            case 3:
                sv = vT;
                sp = fO;
                break;
            default:
                sv = vP;
                sp = fP;
            }
            glViewport(0, 0, w, h);
            glScissor(0, 0, w, h);
            glClear(GL_DEPTH_BUFFER_BIT);
            drawScene(cubeVAO, lightVAO, ls, fs, sv, sp);
            glDisable(GL_SCISSOR_TEST);
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    for (int i = 0; i < NUM_POINT_LIGHTS; i++)
        delete pointLights[i];
    for (int i = 0; i < NUM_SPOT_LIGHTS; i++)
        delete spotLights_arr[i];
    delete sphWheel;
    delete sphHub;
    for (int i = 0; i < 5; i++)
        delete sphLeaf[i];
    delete treeCone;
    delete bezierTable;
    delete bezierArch;
    delete bezierColorChamber;
    delete mathCone;
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteVertexArrays(1, &texCubeVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteBuffers(1, &cubeEBO);
    glfwTerminate();
    return 0;
}

#include "../include/render_utils.h"

// LOAD TEXTURE with proper OpenGL parameters
unsigned int loadTexture(const char *path)
{
    unsigned int id = 0;
    glGenTextures(1, &id);
    int w, h, ch;
    unsigned char *data = stbi_load(path, &w, &h, &ch, 0);
    if (data)
    {
        GLenum fmt = ch == 4 ? GL_RGBA : GL_RGB;
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        // Apply default texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        stbi_image_free(data);
        cout << "  [OK] Loaded: " << path << " (" << w << "x" << h << ")" << endl;
    }
    else
    {
        // Don't print warning here, let the caller handle it to avoid duplicate messages
        glDeleteTextures(1, &id);
        return 0;
    }
    return id;
}

// Apply texture wrap mode to all loaded textures
void applyTextureWrapMode(GLint wrapMode)
{
    unsigned int textureList[] = {texFloor, texFashion, texTech, texGems, texFood, 
                                  texTreeLeaf, texTreeBark, texGrass, texBrick, 
                                  texConcrete, texMetal, texWood, texRoof, texAsphalt};
    
    for (unsigned int tex : textureList) {
        if (tex != 0) {
            glBindTexture(GL_TEXTURE_2D, tex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
        }
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

// Apply filter mode to all loaded textures
void applyTextureFilterMode(GLint filterMode, GLint mipmapFilter)
{
    unsigned int textureList[] = {texFloor, texFashion, texTech, texGems, texFood, 
                                  texTreeLeaf, texTreeBark, texGrass, texBrick, 
                                  texConcrete, texMetal, texWood, texRoof, texAsphalt};
    
    for (unsigned int tex : textureList) {
        if (tex != 0) {
            glBindTexture(GL_TEXTURE_2D, tex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmapFilter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);
        }
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

// DRAW SCENE


// INPUT
void processInput(GLFWwindow *w)
{
    if (glfwGetKey(w, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(w, true);
    if (glfwGetKey(w, GLFW_KEY_W) == GLFW_PRESS)
        basic_camera.move(FORWARD, deltaTime);
    if (glfwGetKey(w, GLFW_KEY_S) == GLFW_PRESS)
        basic_camera.move(BACKWARD, deltaTime);
    if (glfwGetKey(w, GLFW_KEY_A) == GLFW_PRESS)
        basic_camera.move(LEFT, deltaTime);
    if (glfwGetKey(w, GLFW_KEY_D) == GLFW_PRESS)
        basic_camera.move(RIGHT, deltaTime);
    // Staircase height (X:16-24, Z:0-8, steps face EAST)
    float x = basic_camera.eye.x, z = basic_camera.eye.z;
    if (x >= 16 && x <= 24 && z >= 0 && z <= 8)
    {
        float t = (x - 16.f) / 8.f; // X=16 bottom, X=24 top
        t = glm::clamp(t, 0.f, 1.f);
        basic_camera.eye.y = 1.7f + t * MH;
    }
    else if (basic_camera.eye.y > 3 && x >= -25 && x <= 25 && z >= -25 && z <= 25)
    {
        basic_camera.eye.y = MH + 1.7f;
    }
    else
    {
        basic_camera.eye.y = 1.7f;
    }
}

void updateAnimationsIGNORE(float deltaTime) {
    // ---- Machine Animation Updates (Realistic Kinematics) ----
    // STAMPING PRESS: Precise hydraulic motion with realistic force profile
    // Using realistic press cycle: approach -> dwell -> return
    float pressMotionTime = 4.0f; // Synchronized global 4.0 second cycle
    float pressCyclePhase = fmod(globalTime, pressMotionTime) / pressMotionTime; // 0 to 1
    
    if (pressCyclePhase < 0.1f) {
        // Fast approach (0-10%): rapid descent with smooth deceleration using cubic easing
        float approach = pressCyclePhase / 0.1f;
        float easeIn = approach * approach * (3.0f - 2.0f * approach);
        machineState.pressStroke = 0.5f - easeIn * 0.48f;
        machineState.pressVelocity = -easeIn * 8.5f;
    } else if (pressCyclePhase < 0.50f) {
        // Dwell at pressure (10-50%): sustained contact with micro-vibrations
        float dwellTime = (pressCyclePhase - 0.1f) / 0.4f;
        
        float vibrationFreq = 9.5f + dwellTime * 1.5f;
        float vibrationAmplitude = 0.008f * (1.0f - dwellTime * 0.3f);
        
        float resonanceFreq = 15.0f;
        float resonanceAmplitude = 0.003f * sin(dwellTime * 3.14159f);
        
        machineState.pressStroke = 0.02f + sin(globalTime * vibrationFreq) * vibrationAmplitude + 
                                  sin(globalTime * resonanceFreq) * resonanceAmplitude;
        machineState.pressVelocity = (cosf(globalTime * vibrationFreq) * vibrationAmplitude + 
                                     cosf(globalTime * resonanceFreq) * resonanceAmplitude) * 0.5f;
    } else if (pressCyclePhase < 0.85f) {
        // Return phase (50-85%): rapid retract with smooth acceleration
        float returnPhase = (pressCyclePhase - 0.50f) / 0.35f;
        float easeOut = 1.0f - (1.0f - returnPhase) * (1.0f - returnPhase) * (1.0f - returnPhase);
        machineState.pressStroke = 0.02f + easeOut * 0.48f;
        machineState.pressVelocity = easeOut * 11.0f;
    } else {
        // Pause before next cycle (85-100%): wait for next aligned box 
        machineState.pressStroke = 0.5f;
        machineState.pressVelocity = 0.0f;
    }
    
    // WELDING ROBOT: Full 6-axis articulated arm with synchronized task motion
    // Realistic manufacturing cycle: pick -> inspect -> position -> weld -> place -> return
    float weldCycleTime = 4.0f; // Synchronized global 4.0 second cycle
    float weldCyclePhase = fmod(globalTime, weldCycleTime) / weldCycleTime; // 0 to 1
    
    // Joint 1: Base rotation (Z-axis) - tracks workpiece on turntable with smooth motion
    // Uses sinusoidal motion for smooth continuous rotation
    machineState.weldArmAngle1 = fmod(globalTime * 18.0f, 360.0f); // slower than press for safety (ANSI-compliant speeds)
    
    // Joint 2: Shoulder joint - reaches to different workpiece positions
    if (weldCyclePhase < 0.15f) {
        // Approach: move to input position (fast movement phase)
        float approachTime = weldCyclePhase / 0.15f;
        // Use acceleration profile: starts slow, speeds up, ends smooth
        float accelProfile = approachTime * approachTime * (3.0f - 2.0f * approachTime);
        machineState.weldArmAngle2 = 25.0f + accelProfile * 40.0f;
    } else if (weldCyclePhase < 0.68f) {
        // Welding: maintain position with micro-adjustments for seam following
        float weldTime = (weldCyclePhase - 0.15f) / 0.53f;
        
        // Main seam following motion (sine wave for realistic torch movement)
        float seamMotion = sin(weldTime * 3.14159f * 2.0f) * 3.2f;
        
        // Fine position adjustments (higher frequency micro-corrections)
        float microCorrection = sin(globalTime * 4.5f) * 1.5f;
        
        machineState.weldArmAngle2 = 65.0f + seamMotion + microCorrection;
    } else if (weldCyclePhase < 0.88f) {
        // Retract: move back after welding with deceleration
        float retractTime = (weldCyclePhase - 0.68f) / 0.20f;
        float decelerationProfile = 1.0f - (1.0f - retractTime) * (1.0f - retractTime) * (1.0f - retractTime);
        machineState.weldArmAngle2 = 65.0f - decelerationProfile * 40.0f;
    } else {
        // Return to home position (safe zone)
        machineState.weldArmAngle2 = 25.0f;
    }
    
    // Wrist joint: tool orientation - follows main welding seam path
    // This creates realistic torch angle adjustments during welding
    machineState.weldArmAngle2 += sin(globalTime * 0.65f) * 6.5f; // smooth oscillation for seam following
    
    // Welding torch glow: synchronized with actual welding phase
    if (weldCyclePhase >= 0.15f && weldCyclePhase < 0.68f) {
        // Active welding - intense arc with realistic current pulsing and heat dynamics
        float weldIntensity = sin((weldCyclePhase - 0.15f) / 0.53f * 3.14159f) * 0.5f + 0.5f; // smooth intensity curve
        
        // Welding current oscillation (realistic AC/DC ripple, 50-100Hz typical)
        float arcCurrent = sin(globalTime * 14.5f) * 0.35f + 0.65f; // 30-100% of max current
        
        // Arc stability variations (more stable mid-weld)
        float stabilityFactor = weldIntensity * 0.8f + 0.2f;
        
        // Heat build-up effect (slightly increasing glow as weld progresses)
        float heatBuildUp = (weldCyclePhase - 0.15f) / 0.53f;
        float totalGlow = 0.5f + (weldIntensity * 0.35f) + (arcCurrent * 0.25f) + (heatBuildUp * 0.1f);
        
        machineState.weldTorchGlow = glm::clamp(totalGlow, 0.3f, 1.0f);
    } else {
        // Arc off - residual heat glow fades
        float fadePhase = (weldCyclePhase >= 0.68f) ? (weldCyclePhase - 0.68f) : (weldCyclePhase + (1.0f - 0.68f));
        machineState.weldTorchGlow = glm::max(0.1f - fadePhase * 0.1f, 0.05f);
    }
    
    // ROBOTIC GRIPPER: Synchronized 4-finger precision gripper with adaptive gripping force
    if (weldCyclePhase < 0.12f) {
        // Approaching workpiece - gripper opening to receive part
        float openPhase = weldCyclePhase / 0.12f;
        // Smooth opening motion (ease-out curve)
        float easeOut = 1.0f - (1.0f - openPhase) * (1.0f - openPhase);
        machineState.robotGripperOpen = 0.85f - easeOut * 0.5f; // opens from 0.85 to 0.35
    } else if (weldCyclePhase < 0.18f) {
        // Closing gripper - precision grasp with force feedback
        float closePhase = (weldCyclePhase - 0.12f) / 0.06f;
        // Smooth closing with slight overshoot for secure grip
        float easeIn = closePhase * closePhase * (3.0f - 2.0f * closePhase);
        machineState.robotGripperOpen = 0.35f - easeIn * 0.28f; // closes from 0.35 to 0.07
    } else if (weldCyclePhase < 0.68f) {
        // Holding during welding - adaptive gripping with force variation to prevent slip
        float holdPhase = (weldCyclePhase - 0.18f) / 0.50f;
        
        // Adaptive force control: higher force when welding (more current draw)
        float weldIntensity = sin(holdPhase * 3.14159f * 2.0f) * 0.5f + 0.5f;
        float graspForce = 0.07f + (weldIntensity * 0.08f); // force varies 0.07 to 0.15
        
        // Add slight tremor from welding vibrations transmitted through workpiece
        float weldVibration = sin(globalTime * 8.0f) * 0.002f;
        machineState.robotGripperOpen = graspForce + weldVibration;
    } else if (weldCyclePhase < 0.82f) {
        // Opening gripper - release part with controlled motion
        float releasePhase = (weldCyclePhase - 0.68f) / 0.14f;
        // Smooth opening for safe part release
        float easeOut = 1.0f - (1.0f - releasePhase) * (1.0f - releasePhase);
        machineState.robotGripperOpen = 0.07f + easeOut * 0.78f; // opens from 0.07 to 0.85
    } else {
        // Gripper fully open (safe position)
        machineState.robotGripperOpen = 0.85f;
    }

    // Factory conveyor & global time updates
    if (conveyorRunning) {
        conveyorOffset += deltaTime * conveyorSpeed; // Controllable conveyor movement speed
    }
    globalTime += deltaTime;

    // Box creation system - create boxes at regular intervals
    lastBoxCreationTime += deltaTime;
    if (lastBoxCreationTime >= boxCreationRate && conveyorRunning) {
        for (int i = 0; i < MAX_BOXES; i++) {
            if (!boxes[i].active) {
                boxes[i].active = true;
                boxes[i].position = -25.0f; // Start at input
                boxes[i].color = glm::vec3(0.8f, 0.3f, 0.15f);
                boxes[i].creationTime = globalTime;
                boxes[i].textureID = 0;
                lastBoxCreationTime = 0.0f;
                break;
            }
        }
    }

    // Update box positions and colors - REALISTIC WORKFLOW
    for (int i = 0; i < MAX_BOXES; i++) {
        if (boxes[i].active) {
            // Move box along the path incrementally so packaging toggle can pause downstream flow
            if (conveyorRunning)
            {
                float step = deltaTime * conveyorSpeed * 2.0f;
                bool takesLiftBranch = (i % 3 == 0);
                
                if (!packagingRunning && boxes[i].position >= 8.0f && !takesLiftBranch)
                    step = 0.0f; // pause at packer waiting area
                
                // Lift collision and gathering logic
                if (takesLiftBranch && boxes[i].position >= 20.0f) {
                    bool blocked = false;
                    
                    if (factoryLiftState == 0) {
                        // Gathering at bottom
                        if (boxes[i].position >= 28.5f) blocked = true;
                    } else if (factoryLiftState == 1 || factoryLiftState == 3) {
                        // Lift in motion (up or down). Block entering shaft, block on lift
                        if (boxes[i].position >= 26.2f && boxes[i].position <= 29.5f) blocked = true;
                    } else if (factoryLiftState == 2) {
                        // Lift at top. Unloading.
                        // Block entering shaft at bottom.
                        if (boxes[i].position >= 26.2f && boxes[i].position <= 27.0f) blocked = true;
                    }
                    
                    if (!blocked) {
                        for(int j=0; j<MAX_BOXES; j++) {
                            if(i!=j && boxes[j].active && (j%3==0) && boxes[j].position > boxes[i].position && boxes[j].position < 40.0f && (boxes[j].position - boxes[i].position) < 0.65f) {
                                blocked = true;
                                break;
                            }
                        }
                    }
                    if (blocked) step = 0.0f;
                }

                boxes[i].position += step;
            }
            
            // Realistic rolling rotation (continuous, proportional to distance)
            float pathDistance = boxes[i].position + 25.0f;
            boxes[i].rotation = fmod(pathDistance * 20.0f, 360.0f);

            // PROCESSING WORKFLOW STAGES - Color changes through workflow
            if (boxes[i].position < -20.0f) {
                // STAGE 0: Raw input waiting (red-brown)
                boxes[i].color = glm::vec3(0.8f, 0.35f, 0.15f);
                boxes[i].textureID = 0;
            } 
            else if (boxes[i].position < -9.0f) {
                // STAGE 1: Stamping Press (Red Transition)
                float stageFactor = (boxes[i].position + 20.0f) / 11.0f;
                boxes[i].color = glm::mix(
                    glm::vec3(0.8f, 0.35f, 0.15f), 
                    glm::vec3(0.8f, 0.1f, 0.1f), 
                    stageFactor
                );
                boxes[i].textureID = 0;
            } 
            else if (boxes[i].position < -7.0f) {
                // STAGE 2: Approaching Color Chamber. Decide vibrant final color based on its ID
                glm::vec3 brightColors[] = {
                     glm::vec3(0.1f, 0.6f, 1.0f), // Neon Blue
                     glm::vec3(1.0f, 0.2f, 0.5f), // Neon Pink
                     glm::vec3(1.0f, 0.8f, 0.0f), // Neon Yellow
                     glm::vec3(0.1f, 0.9f, 0.2f), // Neon Green
                     glm::vec3(1.0f, 0.5f, 0.0f), // Neon Orange
                     glm::vec3(0.7f, 0.2f, 1.0f)  // Neon Purple
                };
                boxes[i].color = brightColors[i % 6];
                boxes[i].textureID = 0;
            }
            else if (boxes[i].position < 6.0f) {
                // STAGE 3 & 4: Processed and passing Welding line with Textures
                if (boxes[i].textureID == 0 && boxes[i].position > -6.0f) {
                    unsigned int textures[] = {texTech, texFashion, texGems, texFood, texBrick, texWood};
                    boxes[i].textureID = textures[i % 6];
                }
                // Weld heat effect overlays slightly
                float weldEffect = machineState.weldTorchGlow * 0.15f;
                boxes[i].color += glm::vec3(weldEffect, weldEffect, weldEffect * 0.5f);
            }
            else if (boxes[i].position < 10.0f) {
                if (!(i % 3 == 0)) {
                    // STAGE 5: Down the chute into Boxing Machine (packs into cardboard)
                    boxes[i].textureID = 0;
                    float packFactor = glm::clamp((boxes[i].position - 6.0f) / 4.0f, 0.0f, 1.0f);
                    boxes[i].color = glm::mix(boxes[i].color, glm::vec3(0.92f, 0.92f, 0.90f), packFactor);
                }
            }
            else if (boxes[i].position < 40.0f) {
                if (!(i % 3 == 0)) {
                    // STAGE 6: Invisible tracker for logic pipeline.
                    boxes[i].textureID = 0;
                    // Drawn=false logic prevents it from floating here.
                }
            }
            else {
                // STAGE 8: Complete logically.
                boxes[i].active = false;
            }
        }
    }

    int boxesOnLift = 0;
    for (int i = 0; i < MAX_BOXES; i++) {
        if (boxes[i].active && (i % 3 == 0) && boxes[i].position >= 27.0f && boxes[i].position <= 28.6f) {
            boxesOnLift++;
        }
    }
    
    // Factory Lift System Update
    if (factoryLiftState == 0) { // Gathering
        if (boxesOnLift >= 3) {
            factoryLiftState = 1;
        }
    } else if (factoryLiftState == 1) { // Up
        factoryLiftY += deltaTime * 2.0f;
        if (factoryLiftY >= 3.0f) {
            factoryLiftY = 3.0f;
            factoryLiftState = 2; // hold
            factoryLiftWait = 0.0f;
        }
    } else if (factoryLiftState == 2) { // Hold at top and unload
        bool liftClear = true;
        for(int i=0; i<MAX_BOXES; i++) {
            if (boxes[i].active && (i % 3 == 0) && boxes[i].position >= 27.0f && boxes[i].position <= 29.5f) {
                liftClear = false;
                break;
            }
        }
        if (liftClear) {
            factoryLiftState = 3; // go down once everyone rolled off onto the upper floor
        }
    } else if (factoryLiftState == 3) { // Down
        factoryLiftY -= deltaTime * 2.0f;
        if (factoryLiftY <= 0.0f) {
            factoryLiftY = 0.0f;
            factoryLiftState = 0; // ready to gather
        }
    }

    // Palletizer cycle update (driven by packaging system state)
    if (packagingRunning) {
        palletizerCycle += deltaTime * packingSpeed;
        if (palletizerCycle >= 1.0f) palletizerCycle -= 1.0f;
    }

    // Select a source box for pick phase (from packer output segment)
    if (packagingRunning && palletizerCurrentSource < 0 && palletizerCycle < 0.18f) {
        for (int i = 0; i < MAX_BOXES; ++i) {
            if (boxes[i].active && boxes[i].position > 8.0f && boxes[i].position < 12.0f) {
                palletizerCurrentSource = i;
                break;
            }
        }
    }

    // Commit placement at end of place phase
    if (packagingRunning && palletizerCurrentSource >= 0 && palletizerCycle > 0.72f && palletizerCycle < 0.80f) {
        if (boxes[palletizerCurrentSource].active) {
            boxes[palletizerCurrentSource].active = false; // consumed by palletizer
            palletizerStackCount++;
            
            // Transfer an entire pallet to finished goods staging automatically
            if (palletizerStackCount >= 16) {
                palletizerStackCount -= 16;
                if (gatheredBoxes < 72) gatheredBoxes += 16;
            }
        }
        palletizerCurrentSource = -1;
    }

    // Escalator step movement
    escalatorOffset += escalatorDir * deltaTime * 2.0f;
    if (escalatorOffset > 10.0f) escalatorOffset -= 10.0f;
    if (escalatorOffset < 0.0f) escalatorOffset += 10.0f;

    // Boom barrier animation
    float targetAngle = barrierOpen ? 90.0f : 0.0f;
    if (barrierAngle < targetAngle) barrierAngle = glm::min(barrierAngle + deltaTime * 90.0f, targetAngle);
    if (barrierAngle > targetAngle) barrierAngle = glm::max(barrierAngle - deltaTime * 90.0f, targetAngle);

    // Entrance door animation
    float doorTarget = entranceDoorOpen ? 1.0f : 0.0f;
    if (entranceDoorOffset < doorTarget) entranceDoorOffset = glm::min(entranceDoorOffset + deltaTime * 1.2f, doorTarget);
    if (entranceDoorOffset > doorTarget) entranceDoorOffset = glm::max(entranceDoorOffset - deltaTime * 1.2f, doorTarget);

}

void key_callback(GLFWwindow *w, int key, int sc, int action, int mods)
{
    // Accept initial press and repeats (some platforms may send repeat events)
    if (action != GLFW_PRESS && action != GLFW_REPEAT)
        return;
    if (key == GLFW_KEY_F1)
    {
        activeViewport = (activeViewport == 1) ? 0 : 1;
        cout << "Viewport toggled -> " << activeViewport << " (F1)" << endl;
    }
    if (key == GLFW_KEY_F2)
    {
        activeViewport = (activeViewport == 2) ? 0 : 2;
        cout << "Viewport toggled -> " << activeViewport << " (F2)" << endl;
    }
    if (key == GLFW_KEY_F3)
    {
        activeViewport = (activeViewport == 3) ? 0 : 3;
        cout << "Viewport toggled -> " << activeViewport << " (F3)" << endl;
    }
    if (key == GLFW_KEY_F4)
    {
        activeViewport = (activeViewport == 4) ? 0 : 4;
        cout << "Viewport toggled -> " << activeViewport << " (F4)" << endl;
    }
    if (key == GLFW_KEY_1)
    {
        ambientEnabled = !ambientEnabled;
        cout << "Ambient: " << (ambientEnabled ? "ON" : "OFF") << endl;
    }
    if (key == GLFW_KEY_2)
    {
        pointLightsEnabled = !pointLightsEnabled;
        cout << "Point Lights: " << (pointLightsEnabled ? "ON" : "OFF") << endl;
    }
    if (key == GLFW_KEY_3)
    {
        dirLightEnabled = !dirLightEnabled;
        cout << "Dir Light: " << (dirLightEnabled ? "ON" : "OFF") << endl;
    }
    if (key == GLFW_KEY_5)
    {
        spotLightsEnabled = !spotLightsEnabled;
        cout << "Spot Lights: " << (spotLightsEnabled ? "ON" : "OFF") << endl;
    }
    if (key == GLFW_KEY_L)
    {
        noLightCondition = !noLightCondition;
        cout << "Lighting Condition: " << (noLightCondition ? "NO LIGHT (Unlit)" : "NORMAL") << endl;
    }
    if (key == GLFW_KEY_H)
    {
        shadingMode = (shadingMode == 0) ? 1 : 0;
        cout << "Shading Mode: " << (shadingMode == 0 ? "PHONG" : "GOURAUD") << endl;
    }
    if (key == GLFW_KEY_T)
    {
        texturesEnabled = !texturesEnabled;
        cout << "Textures: " << (texturesEnabled ? "ON" : "OFF") << endl;
    }
    if (key == GLFW_KEY_U)
    {
        currentWrapIndex = (currentWrapIndex + 1) % 3;
        currentWrapMode = wrapModes[currentWrapIndex];
        applyTextureWrapMode(currentWrapMode);
        cout << "Texture Wrap Mode: " << wrapModeNames[currentWrapIndex] << endl;
    }
    if (key == GLFW_KEY_Y)
    {
        currentFilterIndex = (currentFilterIndex + 1) % 2;
        currentFilterMode = filterModes[currentFilterIndex];
        // Update min filter based on mipmapping
        if (useMipmapping) {
            currentMipmapFilter = (currentFilterMode == GL_LINEAR) 
                ? GL_LINEAR_MIPMAP_LINEAR 
                : GL_NEAREST_MIPMAP_NEAREST;
        } else {
            currentMipmapFilter = currentFilterMode;
        }
        applyTextureFilterMode(currentFilterMode, currentMipmapFilter);
        cout << "Texture Filter Mode: " << filterModeNames[currentFilterIndex] << endl;
    }
    if (key == GLFW_KEY_M)
    {
        useMipmapping = !useMipmapping;
        // Reapply filter with new mipmap setting
        if (useMipmapping) {
            currentMipmapFilter = (currentFilterMode == GL_LINEAR) 
                ? GL_LINEAR_MIPMAP_LINEAR 
                : GL_NEAREST_MIPMAP_NEAREST;
        } else {
            currentMipmapFilter = currentFilterMode;
        }
        applyTextureFilterMode(currentFilterMode, currentMipmapFilter);
        cout << "Mipmapping: " << (useMipmapping ? "ENABLED (Linear Mipmap Linear)" : "DISABLED (No Mipmaps)") << endl;
    }
    if (key == GLFW_KEY_N)
    {
        dayNightCycleActive = !dayNightCycleActive;
        dayNightSpeed = dayNightCycleActive ? 3.0f : 0.0f; // 3 hours per second for demo
        cout << "Day/Night Cycle: " << (dayNightCycleActive ? "ON" : "OFF")
             << " (Time: " << (int)timeOfDay << ":00)" << endl;
    }
    if (key == GLFW_KEY_E)
    {
        // Elevator toggle
        if (elevatorState == ELEV_STOPPED_BOTTOM) {
            elevatorState = ELEV_DOOR_CLOSING_UP;
            elevatorDoorOffset = 1.0f; // start with door open -> closing
            cout << "Elevator: Going UP" << endl;
        } else if (elevatorState == ELEV_STOPPED_TOP) {
            elevatorState = ELEV_DOOR_CLOSING_DOWN;
            elevatorDoorOffset = 1.0f;
            cout << "Elevator: Going DOWN" << endl;
        } else {
            cout << "Elevator: In motion..." << endl;
        }
    }
    if (key == GLFW_KEY_R)
    {
        escalatorDir = (escalatorDir + 2) % 3 - 1; // cycle: 1 -> 0 -> -1 -> 1
        const char* dirs[] = {"DOWN", "PAUSED", "UP"};
        cout << "Escalator: " << dirs[escalatorDir + 1] << endl;
    }
    if (key == GLFW_KEY_B)
    {
        barrierOpen = !barrierOpen;
        cout << "Boom Barrier: " << (barrierOpen ? "OPENING" : "CLOSING") << endl;
    }
    if (key == GLFW_KEY_C)
    {
        cursorCaptured = !cursorCaptured;
        glfwSetInputMode(w, GLFW_CURSOR, cursorCaptured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        if (cursorCaptured) firstMouse = true; // avoid jump when re-capturing
        cout << "Cursor: " << (cursorCaptured ? "CAPTURED" : "FREE") << endl;
    }
    if (key == GLFW_KEY_G)
    {
        entranceDoorOpen = !entranceDoorOpen;
        cout << "Entrance Door: " << (entranceDoorOpen ? "OPENING" : "CLOSING") << endl;
    }
    
    // --- Assignment Exhibition Toggles ---
    if (key == GLFW_KEY_6)
    {
        exShowcase = !exShowcase;
        cout << "Assignment Showcase: " << (exShowcase ? "VISIBLE" : "HIDDEN") << endl;
    }
    if (key == GLFW_KEY_7)
    {
        exBlendMode = !exBlendMode;
        cout << "Exhibition Texture: " << (exBlendMode ? "BLENDED with Object Color" : "SIMPLE (No Surface Color)") << endl;
    }
    if (key == GLFW_KEY_8)
    {
        exVertexShade = !exVertexShade;
        cout << "Exhibition Lighting Computation: " << (exVertexShade ? "ON VERTEX (Gouraud)" : "ON FRAGMENT (Phong)") << endl;
    }
    if (key == GLFW_KEY_9)
    {
        exObjectType = (exObjectType + 1) % 2;
        cout << "Exhibition Curvy Object: " << (exObjectType == 0 ? "SPHERE" : "CONE") << endl;
    }

    if (key == GLFW_KEY_SPACE)
    {
        conveyorRunning = !conveyorRunning;
        cout << "Conveyor: " << (conveyorRunning ? "RUNNING" : "STOPPED") << endl;
    }
    if (key == GLFW_KEY_EQUAL || key == GLFW_KEY_KP_ADD)
    {
        conveyorSpeed = glm::min(conveyorSpeed + 0.2f, 3.0f);
        cout << "Conveyor Speed: " << conveyorSpeed << " units/sec" << endl;
    }
    if (key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_SUBTRACT)
    {
        conveyorSpeed = glm::max(conveyorSpeed - 0.2f, 0.2f);
        cout << "Conveyor Speed: " << conveyorSpeed << " units/sec" << endl;
    }
    if (key == GLFW_KEY_P)
    {
        packagingRunning = !packagingRunning;
        cout << "Packaging System: " << (packagingRunning ? "RUNNING" : "STOPPED") << endl;
    }
    if (key == GLFW_KEY_LEFT_BRACKET)
    {
        boxCreationRate = glm::max(boxCreationRate - 0.1f, 0.2f);
        cout << "Box Creation Rate: 1 box every " << boxCreationRate << " seconds" << endl;
    }
    if (key == GLFW_KEY_RIGHT_BRACKET)
    {
        boxCreationRate = glm::min(boxCreationRate + 0.1f, 2.0f);
        cout << "Box Creation Rate: 1 box every " << boxCreationRate << " seconds" << endl;
    }
}
void mouse_callback(GLFWwindow *w, double xp, double yp)
{
    if (!cursorCaptured) return; // don't move camera when cursor is free
    float x = (float)xp, y = (float)yp;
    if (firstMouse)
    {
        lastX = x;
        lastY = y;
        firstMouse = false;
    }
    basic_camera.ProcessMouseMovement(x - lastX, lastY - y);
    lastX = x;
    lastY = y;
}
void scroll_callback(GLFWwindow *w, double xo, double yo) { basic_camera.ProcessMouseScroll((float)yo); }
void framebuffer_size_callback(GLFWwindow *w, int width, int height)
{
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
}
 