
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
void drawCube(unsigned int &v, Shader &s, glm::mat4 pm, glm::vec3 c, glm::vec3 p, glm::vec3 sc, float sh = 32, float a = 1.0f);
void drawCubeTextured(unsigned int &v, Shader &s, glm::mat4 pm, glm::vec3 c, glm::vec3 p, glm::vec3 sc, unsigned int tex, float tiling = 1.0f, float sh = 32, float a = 1.0f);
unsigned int loadTexture(const char *path);
void drawScene(unsigned int &, unsigned int &, Shader &, Shader &, glm::mat4, glm::mat4);
void updateAnimations(float deltaTime);

// Try to locate resource files in a few common relative locations so the
// executable can find shaders/textures even if the working directory differs.
static bool fileExists(const std::string &path)
{
    std::ifstream f(path);
    return f.good();
}

static std::string findResource(const std::string &name)
{
    std::vector<std::string> candidates = {
        name,
        std::string("./") + name,
        std::string("../") + name,
        std::string("resources/") + name,
        std::string("../resources/") + name,
        std::string("shaders/") + name,
        std::string("../shaders/") + name,
        std::string("textures/") + name,
        std::string("../textures/") + name,
        std::string("assets/") + name,
        std::string("../assets/") + name};
    // Also check for a sibling project folder named "Graphics_Project" in parent dirs
    // (this workspace uses ..\..\Graphics_Project relative to the boss project)
    for (int depth = 0; depth < 5; ++depth) {
        std::string up;
        for (int i = 0; i < depth; ++i) up += "../";
        candidates.push_back(up + std::string("Graphics_Project/") + name);
        candidates.push_back(up + std::string("Graphics_Project/shaders/") + name);
        candidates.push_back(up + std::string("Graphics_Project/textures/") + name);
        candidates.push_back(up + std::string("Graphics_Project/resources/") + name);
    }
    for (const auto &c : candidates)
    {
        if (fileExists(c))
        {
            cout << "Found resource: " << name << " -> " << c << endl;
            return c;
        }
    }
    cout << "Resource not found (will try original path): " << name << endl;
    return name;
}

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
const float MH = 5.0f; // mall height
const float WT = 0.3f; // wall thickness

// Colors
//  Exterior
const glm::vec3 C_GRASS(0.22f, 0.55f, 0.18f);
const glm::vec3 C_GRASS2(0.18f, 0.48f, 0.15f);
const glm::vec3 C_ROAD(0.25f, 0.25f, 0.27f);
const glm::vec3 C_SIDEWALK(0.68f, 0.66f, 0.62f);
const glm::vec3 C_ROAD_YEL(0.95f, 0.82f, 0.12f);
const glm::vec3 C_ROAD_WHT(0.92f, 0.92f, 0.90f);
const glm::vec3 C_PARKING(0.42f, 0.42f, 0.44f);
const glm::vec3 C_PARK_LINE(0.95f, 0.85f, 0.15f);
const glm::vec3 C_EXT(0.80f, 0.78f, 0.74f); // exterior wall
const glm::vec3 C_GLASS(0.58f, 0.80f, 0.93f);
const glm::vec3 C_AWNING(0.38f, 0.38f, 0.42f);
const glm::vec3 C_TRUNK(0.42f, 0.30f, 0.16f);
const glm::vec3 C_CROWN(0.22f, 0.58f, 0.20f);
const glm::vec3 C_LAMP(0.32f, 0.32f, 0.34f);
const glm::vec3 C_TIRE(0.12f, 0.12f, 0.12f);
const glm::vec3 C_BARRIER_R(0.90f, 0.15f, 0.10f);
// Interior
const glm::vec3 C_FLOOR(0.82f, 0.78f, 0.72f); // mall floor
const glm::vec3 C_CEILING(0.93f, 0.91f, 0.89f);
const glm::vec3 C_CORR(0.85f, 0.83f, 0.78f); // corridor wall
// Fashion shop - PINK walls (from reference)
const glm::vec3 C_FASH_WALL(0.95f, 0.68f, 0.72f);
const glm::vec3 C_FASH_FLOOR(0.78f, 0.65f, 0.55f);
const glm::vec3 C_RACK(0.70f, 0.70f, 0.72f); // clothing rack metal
const glm::vec3 C_MIRROR(0.75f, 0.88f, 0.95f);
const glm::vec3 C_MANNEQUIN(0.85f, 0.78f, 0.65f);
const glm::vec3 C_CASH_DESK(0.60f, 0.45f, 0.30f);
// Tech shop - BLUE-GREY walls (from reference)
const glm::vec3 C_TECH_WALL(0.68f, 0.75f, 0.85f);
const glm::vec3 C_TECH_FLOOR(0.65f, 0.65f, 0.68f);
const glm::vec3 C_DISP_TABLE(0.72f, 0.60f, 0.45f); // display table wood
const glm::vec3 C_DISP_BASE(0.50f, 0.50f, 0.52f);  // table base grey
const glm::vec3 C_PHONE(0.15f, 0.15f, 0.18f);      // phone/device
const glm::vec3 C_SCREEN(0.25f, 0.62f, 0.45f);     // screen green-ish
const glm::vec3 C_LAPTOP(0.55f, 0.55f, 0.58f);
// Gems shop
const glm::vec3 C_GEM_WALL(0.92f, 0.85f, 0.65f);
const glm::vec3 C_GEM_CASE(0.70f, 0.68f, 0.65f);
// Book shop
const glm::vec3 C_BOOK_WALL(0.72f, 0.58f, 0.42f);
const glm::vec3 C_BOOKSHELF(0.50f, 0.35f, 0.22f);
// Food court (from reference - colorful stalls)
const glm::vec3 C_FOOD_FLOOR(0.72f, 0.70f, 0.66f);
const glm::vec3 C_STALL_RED(0.88f, 0.22f, 0.15f);
const glm::vec3 C_STALL_YEL(0.92f, 0.82f, 0.18f);
const glm::vec3 C_STALL_GRN(0.20f, 0.72f, 0.30f);
const glm::vec3 C_STALL_ORG(0.95f, 0.55f, 0.12f);
const glm::vec3 C_STALL_PUR(0.60f, 0.30f, 0.70f);
const glm::vec3 C_TABLE(0.62f, 0.48f, 0.30f);
const glm::vec3 C_CHAIR(0.55f, 0.40f, 0.25f);
const glm::vec3 C_COUNTER(0.65f, 0.60f, 0.55f);
// Prayer room (from reference - calm beige/cream)
const glm::vec3 C_PRAY_WALL(0.90f, 0.87f, 0.80f);
const glm::vec3 C_PRAY_FLOOR(0.78f, 0.68f, 0.55f);
const glm::vec3 C_PRAY_MAT(0.55f, 0.72f, 0.58f);
const glm::vec3 C_PRAY_LAMP(0.92f, 0.88f, 0.75f);
const glm::vec3 C_PRAY_SHELF(0.65f, 0.55f, 0.42f);
// Washroom / Staircase
const glm::vec3 C_WASH(0.88f, 0.88f, 0.86f);
const glm::vec3 C_STAIR(0.58f, 0.56f, 0.52f);
const glm::vec3 C_PARAPET(0.72f, 0.70f, 0.66f);

// Factory / Industrial colors
const glm::vec3 C_FACTORY_FLOOR(0.35f, 0.35f, 0.36f);
const glm::vec3 C_CONVEYOR(0.12f, 0.12f, 0.14f);
const glm::vec3 C_BOX(0.76f, 0.60f, 0.36f);
const glm::vec3 C_METAL(0.55f, 0.56f, 0.58f);
const glm::vec3 C_MACHINE(0.2f, 0.2f, 0.22f);
const glm::vec3 C_ROLLER(0.45f, 0.45f, 0.48f);
const glm::vec3 C_CYAN_ACCENT(0.0f, 0.8f, 1.0f);
const glm::vec3 C_DARK_METAL(0.25f, 0.25f, 0.28f);

//  Lights
const int NUM_POINT_LIGHTS = 14;
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

// Toggle flags
bool texturesEnabled = true;
bool ambientEnabled = true;
bool pointLightsEnabled = true;
bool dirLightEnabled = true;
bool spotLightsEnabled = true;
bool noLightCondition = false;
int shadingMode = 0; // 0 = Phong, 1 = Gouraud

// Day/Night cycle
float timeOfDay = 10.0f;  // 0-24 hours, start at 10 AM
float dayNightSpeed = 0.0f; // hours per second (0 = paused, press N to cycle)
bool dayNightCycleActive = false;

// Spot Lights (store display highlights)
const int NUM_SPOT_LIGHTS = 4;
SpotLight* spotLights_arr[NUM_SPOT_LIGHTS] = {};

// Elevator state machine
enum ElevatorState { ELEV_STOPPED_BOTTOM, ELEV_DOOR_CLOSING_UP, ELEV_MOVING_UP, ELEV_STOPPED_TOP, ELEV_DOOR_CLOSING_DOWN, ELEV_MOVING_DOWN, ELEV_DOOR_OPENING };
ElevatorState elevatorState = ELEV_STOPPED_BOTTOM;
float elevatorY = 0.0f;
float elevatorDoorOffset = 0.0f; // 0 = closed, 1 = open
float elevatorTimer = 0.0f;

// Escalator
float escalatorOffset = 0.0f;
int escalatorDir = 1; // 1=up, -1=down, 0=paused

// Boom barrier
float barrierAngle = 0.0f; // 0=closed, 90=open
bool barrierOpen = false;

// Wheel rotation for cars
float wheelRotation = 0.0f;

// Cursor capture toggle
bool cursorCaptured = true;

// Entrance glass door
bool entranceDoorOpen = false;
float entranceDoorOffset = 0.0f; // 0=closed, 1=open

// Texture IDs
unsigned int texCubeVAO = 0;
unsigned int texFloor = 0, texFashion = 0, texTech = 0, texGems = 0, texFood = 0, texTreeLeaf = 0, texTreeBark = 0, texGrass = 0;
unsigned int texBrick = 0, texConcrete = 0, texMetal = 0, texWood = 0, texRoof = 0, texAsphalt = 0;

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
const int NUM_TREES = 10;
FractalTree* fractalTrees[NUM_TREES] = {};

// Bezier furniture objects
BezierRevolvedSurface* bezierTable = nullptr;
BezierArch* bezierArch = nullptr;
BezierRevolvedSurface* bezierColorChamber = nullptr;
BezierRevolvedSurface* mathCone = nullptr; // True mathematical curvy Cone

// Assignment Features Exhibition State
bool exShowcase = true;       // Key '6' - Show/Hide Exhibition Stand
bool exBlendMode = false;     // Key '7' - false = Simple, true = Blended
bool exVertexShade = false;   // Key '8' - false = Fragment, true = Vertex
int exObjectType = 0;         // Key '9' - 0 = Sphere, 1 = Cone

// Factory animation state
float conveyorOffset = 0.0f;
float globalTime = 0.0f;
float conveyorSpeed = 2.0f;  // Controllable conveyor speed (units/sec)
bool conveyorRunning = true; // Toggle conveyor on/off
bool packagingRunning = true; // Toggle packaging system
float packingSpeed = 1.0f;  // Controllable packing speed

// Palletizer animation state (pick -> move -> place -> return)
float palletizerCycle = 0.0f;
int palletizerStackCount = 0;
int palletizerCurrentSource = -1;

// Box tracking for realistic handling
struct Box {
    float position;  // Linear position along path
    glm::vec3 color; // Current color
    bool active;     // Is this box in the system?
    float creationTime; // When was this box created?
    float rotation;  // Box rotation for realistic tumbling
    float vx, vz;    // Velocity components for physics
    unsigned int textureID; // Texture ID for the applied texture
};
const int MAX_BOXES = 50;
Box boxes[MAX_BOXES];
float boxCreationRate = 4.0f;  // Synchronized process: 1 box every 4.0s (matches machine cycles)
float lastBoxCreationTime = 0.0f;
int gatheredBoxes = 0; // Finished packed boxes gathered in shipping area

// Factory Lift System
float factoryLiftY = 0.0f;
int factoryLiftState = 0; // 0=gathering, 1=up, 2=hold, 3=down
float factoryLiftWait = 0.0f;

// Machine states for realistic kinematics
struct MachineState {
    float pressStroke;       // 0 to 1: press head position
    float pressVelocity;     // Velocity of press head
    bool pressExtending;     // Direction of press motion
    float weldArmAngle1;     // Joint 1 rotation
    float weldArmAngle2;     // Joint 2 rotation
    float weldTorchGlow;     // Glow intensity from welding
    float robotGripperOpen;  // 0 to 1: gripper opening
};
MachineState machineState = {0.5f, 0.0f, false, 0.0f, 0.0f, 0.0f, 0.5f};

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

void createGeometryBuffers(unsigned int &cubeVAO, unsigned int &cubeVBO, unsigned int &cubeEBO, unsigned int &lightVAO, unsigned int &texCubeVAO, unsigned int &tcVBO, unsigned int &tcEBO) {
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

// DRAW CUBE
void drawCube(unsigned int &v, Shader &s, glm::mat4 pm, glm::vec3 c, glm::vec3 p, glm::vec3 sc, float sh, float a)
{
    s.use();
    glm::mat4 m = glm::translate(pm, p);
    m = glm::scale(m, sc);
    s.setVec3("material.ambient", c * a);
    s.setVec3("material.diffuse", c);
    s.setVec3("material.specular", glm::vec3(0.3f));
    s.setFloat("material.shininess", sh);
    s.setBool("useTexture", false);
    s.setMat4("model", m);
    glBindVertexArray(v);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

// Enhanced material setup for realistic rendering
void setMaterial(Shader &s, glm::vec3 color, float shininess, float specIntensity, float ambientIntensity = 0.6f) {
    s.setVec3("material.ambient", color * ambientIntensity);
    s.setVec3("material.diffuse", color);
    s.setVec3("material.specular", glm::vec3(specIntensity));
    s.setFloat("material.shininess", shininess);
}

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

// DRAW CUBE TEXTURED
void drawCubeTextured(unsigned int &v, Shader &s, glm::mat4 pm, glm::vec3 c, glm::vec3 p, glm::vec3 sc, unsigned int tex, float tiling, float sh, float a)
{
    s.use();
    glm::mat4 m = glm::translate(pm, p);
    m = glm::scale(m, sc);
    s.setVec3("material.ambient", c * a);
    s.setVec3("material.diffuse", c);
    s.setVec3("material.specular", glm::vec3(0.3f));
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

// DRAW SCENE

void drawScene(unsigned int &V, unsigned int &LV, Shader &ls, Shader &fs, glm::mat4 view, glm::mat4 proj)
{
    ls.use();
    ls.setMat4("view", view);
    ls.setMat4("projection", proj);
    ls.setVec3("viewPos", basic_camera.eye);
    glm::mat4 I(1);
    float WH = MH / 2;

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
        
        // If blended, the object color is strongly red. If simple texture, we make it white to verify it works without surface color influence.
        glm::vec3 objColor = exBlendMode ? glm::vec3(1.0f, 0.2f, 0.2f) : glm::vec3(1.0f, 1.0f, 1.0f);
        
        // Force blend mode for the fragment shader inside the exhibition call
        ls.setBool("blendWithColor", exBlendMode);
        
        // Rotate the display object
        glm::mat4 objModel = glm::translate(I, {exX, 1.5f, exZ});
        objModel = glm::rotate(objModel, glm::radians(globalTime * 30.0f), {0, 1, 0});
        
        // We draw the "textured curvy surfaced object (at least sphere and cone)"
        if (exObjectType == 0) {
            // Draw Sphere
            objModel = glm::scale(objModel, {1.6f, 1.6f, 1.6f});
            sphHub->ambient = objColor;
            sphHub->diffuse = objColor;
            sphHub->drawSphereTextured(ls, objModel, texBrick, 2.0f); // Tiled brick sphere
        } else {
            // Draw Mathematical Curvy Cone
            objModel = glm::translate(objModel, {0.0f, -0.8f, 0.0f}); // Base center alignment
            objModel = glm::scale(objModel, {1.5f, 1.5f, 1.5f});
            mathCone->drawTextured(ls, objModel, texBrick, objColor, 3.0f, 64.0f);
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

    // MALL EXTERIOR (X:-25 to 25, Z:-25 to 25)

    // North wall - with brick texture for realism
    drawCubeTextured(texCubeVAO, ls, I, C_EXT, {0, WH, -25}, {50, MH, WT}, texBrick, 8.0f, 32, 0.9f);
    // South wall (gap X:-4 to 4 for entrance) - with brick texture
    drawCubeTextured(texCubeVAO, ls, I, C_EXT, {-14.5f, WH, 25}, {21, MH, WT}, texBrick, 8.0f, 32, 0.9f);
    drawCubeTextured(texCubeVAO, ls, I, C_EXT, {14.5f, WH, 25}, {21, MH, WT}, texBrick, 8.0f, 32, 0.9f);
    drawCube(V, ls, I, C_EXT, {0, MH - .5f, 25}, {8, 1, WT}); // strip above entrance
    // West wall - with brick texture
    drawCubeTextured(texCubeVAO, ls, I, C_EXT, {-25, WH, 0}, {WT, MH, 50}, texBrick, 10.0f, 32, 0.9f);
    // East wall - with brick texture
    drawCubeTextured(texCubeVAO, ls, I, C_EXT, {25, WH, 0}, {WT, MH, 50}, texBrick, 10.0f, 32, 0.9f);
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

    // FACTORY FLOOR & CEILING (industrial look)
    ls.setBool("blendWithColor", true);
    drawCubeTextured(texCubeVAO, ls, I, C_FACTORY_FLOOR, {0, -.02f, 0}, {49.5f, .08f, 49.5f}, texConcrete, 10.0f, 8, .95f);
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
        // Complete industrial manufacturing facility
        drawCubeTextured(texCubeVAO, ls, I, C_FACTORY_FLOOR, {-14, .03f, 16}, {21, .04f, 16}, texConcrete, 6.0f, 16, 1.0f);
        drawCubeTextured(texCubeVAO, ls, I, C_MACHINE * 0.8f, {-24.5f, WH, 16}, {WT, MH, 16}, texBrick, 4.0f, 32, 1.0f); // dark industrial wall
        
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

    // ======== CLOTHING SHOP (X:4 to 24.5, Z:8 to 24) — Southeast ========
    {
        // Convert clothing shop into advanced assembly area
        drawCubeTextured(texCubeVAO, ls, I, C_FACTORY_FLOOR, {14, .03f, 16}, {21, .04f, 16}, texConcrete, 6.0f, 16, 1.0f);
        drawCubeTextured(texCubeVAO, ls, I, C_FASH_WALL * 0.6f, {24.5f, WH, 16}, {WT, MH, 16}, texBrick, 4.0f, 32, 1.0f);
        
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

    // ======== ESCALATOR (West wall, X:-22 to -18, Z:0 to 8, facing EAST into corridor) ========
    {
        float esX = -20.f, esZ1 = 8.0f, esZ2 = 0.0f;
        float esLen = esZ1 - esZ2;
        float esH = 4.5f;
        // Side rails (run N-S)
        drawCube(V, ls, I, C_LAMP, {esX-0.7f, esH/2, (esZ1+esZ2)/2}, {.05f,esH+.5f,esLen});
        drawCube(V, ls, I, C_LAMP, {esX+0.7f, esH/2, (esZ1+esZ2)/2}, {.05f,esH+.5f,esLen});
        // Handrails
        drawCube(V, ls, I, C_LAMP*1.2f, {esX-0.7f, esH+.3f, (esZ1+esZ2)/2}, {.08f,.06f,esLen});
        drawCube(V, ls, I, C_LAMP*1.2f, {esX+0.7f, esH+.3f, (esZ1+esZ2)/2}, {.08f,.06f,esLen});
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
        // Floor area around escalator
        drawCube(V, ls, I, C_STAIR*.9f, {-20, .03f, 4}, {8, .04f, 8});
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
        drawCube(V, ls, I, C_PRAY_WALL, {-24.5f, WH, -10.5f}, {WT, MH, 7});
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
        drawCube(V, ls, I, C_GEM_WALL, {24.5f, WH, -10.5f}, {WT, MH, 7});
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
    drawCube(V, ls, I, C_PARAPET, {0, pY, -25}, {50, 1, .3f});
    drawCube(V, ls, I, C_PARAPET, {0, pY, 25}, {50, 1, .3f});
    drawCube(V, ls, I, C_PARAPET, {-25, pY, 0}, {.3f, 1, 50});
    drawCube(V, ls, I, C_PARAPET, {25, pY, 0}, {.3f, 1, 50});

    // ===== DRAW ALL SIGNBOARDS =====
    // Update signboard status dynamically based on system states
    if (signboards.size() >= 10) {
        signboards[0].isActive = conveyorRunning && (machineState.pressVelocity != 0.0f); // STAMPING
        signboards[1].isActive = conveyorRunning && (machineState.weldTorchGlow > 0.15f); // WELDING
        signboards[2].isActive = conveyorRunning; // INSPECT
        signboards[3].isActive = packagingRunning && (palletizerCurrentSource >= 0); // PALLET
        signboards[4].isActive = packagingRunning; // WRAP
        signboards[5].isActive = packagingRunning; // SHIP
        signboards[6].isActive = conveyorRunning; // ASSEMBLY
        signboards[7].isActive = conveyorRunning; // COLOR APPLIED
        signboards[8].isActive = packagingRunning; // PACKED
        signboards[9].isActive = gatheredBoxes > 0; // GATHERED
    }
    
    for (auto& board : signboards) {
        board.draw(V, ls, I, globalTime);
    }

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

void updateAnimations(float deltaTime) {
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
 