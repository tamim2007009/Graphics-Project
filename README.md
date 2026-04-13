# Factory Interior Simulation - Advanced Graphics Project

## ?? Project Overview

**Factory Interior Simulation** is a comprehensive 3D graphics application built with **OpenGL 3.3 Core Profile** that demonstrates advanced computer graphics techniques including sophisticated lighting systems, procedural geometry generation, Bézier curve applications, and realistic manufacturing process simulations. The project showcases a realistic factory mall environment with interactive elements, dynamic texturing, and complex machine animations.

### Key Features
- **Realistic Factory Environment**: A detailed mall layout with multiple shops, factory zones, and parking areas
- **Advanced Lighting System**: 14 point lights, 2 directional lights (sun simulation), and 4 spot lights with day/night cycle
- **Dynamic Manufacturing Processes**: Stamping press, welding robot, conveyor system, and packaging machinery
- **Procedural Geometry**: Fractals, Bézier surfaces, and ruled surfaces for organic shapes
- **Advanced Shading**: Phong and Gouraud shading models with texture blending
- **Interactive Scene Controls**: Real-time lighting toggles, camera controls, and animation adjustments

---

## ?? Technical Specifications

### Core Technologies
- **Language**: C++11/14
- **Graphics API**: OpenGL 3.3 Core Profile
- **Window Management**: GLFW 3.x
- **Mathematics**: GLM (OpenGL Mathematics)
- **Image Loading**: STB Image
- **Build System**: CMake (implied by project structure)

### Dependencies
```
- glad: OpenGL loader
- GLFW: Window and input management
- glm: Vector and matrix mathematics
- stb_image: Image file format support
```

---

## ?? Controls & Features

### Camera Navigation
| Key | Function |
|-----|----------|
| `W/A/S/D` | Move camera forward/left/backward/right |
| `Mouse Move` | Look around (when cursor captured) |
| `Scroll Wheel` | Zoom in/out |
| `C` | Toggle cursor capture (mouse lock) |
| `ESC` | Exit application |

### Lighting Controls
| Key | Function |
|-----|----------|
| `1` | Toggle ambient lighting |
| `2` | Toggle point lights (shop, corridor illumination) |
| `3` | Toggle directional lighting (sun/lamps) |
| `4` | Cycle lighting modes (Ambient?Directional?Diffuse?Combined) |
| `5` | Toggle spot lights (store display highlights) |
| `L` | No-light mode (unlit texture view) |
| `H` | Toggle shading model (Phong ? Gouraud) |

### Texture Controls
| Key | Function |
|-----|----------|
| `T` | Toggle textures ON/OFF |
| `U` | Cycle wrap mode (REPEAT ? CLAMP_TO_EDGE ? MIRRORED_REPEAT) |
| `Y` | Cycle filter mode (LINEAR ? NEAREST) |
| `M` | Toggle mipmapping (Linear Mipmap Linear ? No Mipmaps) |

### Environment Interaction
| Key | Function |
|-----|----------|
| `N` | Toggle day/night cycle animation |
| `E` | Control elevator (up/down) |
| `R` | Escalator direction (up/down/pause) |
| `B` | Open/close boom barrier |
| `G` | Open/close entrance door |

### Manufacturing Simulation Controls
| Key | Function |
|-----|----------|
| `SPACE` | Start/stop conveyor system |
| `+/-` | Increase/decrease conveyor speed |
| `P` | Start/stop packaging system |
| `[/]` | Adjust box creation rate |

### Viewport Controls
| Key | Function |
|-----|----------|
| `F1` | Toggle front orthographic view |
| `F2` | Toggle side orthographic view |
| `F3` | Toggle top orthographic view |
| `F4` | Toggle perspective view |

### Exhibition Feature Controls
| Key | Function |
|-----|----------|
| `6` | Show/hide assignment showcase (SE corner) |
| `7` | Cycle exhibition texture mode (Simple?Texture?Blended) |
| `8` | Toggle exhibition shading (Fragment ? Vertex) |
| `9` | Toggle exhibition object type (Sphere ? Cone) |

---

## ??? Architecture & Core Components

### Scene Structure

#### Lighting System
- **14 Point Lights**: Strategic placement throughout mall and factory zones
  - Corridor illumination: 2 lights in N-S corridor
  - Shop lighting: 6 lights for retail areas
  - Functional areas: Escalators, washrooms, food court (4 lights each)
  - Parking lot: 2 overhead lights
  
- **2 Directional Lights**: Simulating sun from two angles with day/night interpolation
  - Warm daylight diffuse: `(0.9, 0.75, 0.4)`
  - Cool night diffuse: `(0.05, 0.05, 0.12)`
  
- **4 Spot Lights**: Retail display emphasis
  - Tech store: 2 spots on merchandise display tables
  - Gems store: 1 spot on display cases
  - Bookstore: 1 spot on reading area

#### Scene Objects

**Geometric Primitives**
- Sphere mesh: Multi-segment/stack construction for wheels and leaves
- Pyramid: Tree cone representation with configurable material properties
- Cube: Basic building block for environment

**Procedural Geometry**
- **Fractal Trees**: 10 trees with parametric generation (height, radius, depth, branching angle)
- **Bézier Surfaces**: Revolution surfaces for furniture (tables, arches, color chamber)
- **Catmull-Rom Splines**: Smooth curves for escalator handrails and conveyor paths
- **Ruled Surfaces**: Escalator steps and handrail tube meshes

**Dynamic Objects**
- Animated boxes (conveyor workflow)
- Rotating wheels and gears
- Articulated robotic arm (6-axis kinematics)
- Hydraulic stamping press
- Color application chamber
- Palletizer with lifting mechanism

#### Material System
Materials include:
- Ambient, diffuse, specular components
- Shininess coefficient (4-64)
- Texture coordinates and blending modes
- Dynamic color transitions through workflow stages

### Rendering Pipeline

#### Shaders (8 shaders total)
1. **Phong Shading Suite**
   - `vertexShaderForPhongShading.vs` / `fragmentShaderForPhongShading.fs`: Standard per-fragment lighting
   - `vertexShaderForPhongShadingWithTexture.vs` / `fragmentShaderForPhongShadingWithTexture.fs`: Textured variant

2. **Gouraud Shading Suite**
   - `vertexShaderForGouraudShading.vs` / `fragmentShaderForGouraudShading.fs`: Per-vertex lighting
   - `vertexShader.vs` / `fragmentShader.fs`: Basic unlit rendering

#### Texture System
**Supported Formats**: JPG, PNG
**Texture Wrapping Modes**: GL_REPEAT, GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT
**Filter Modes**: GL_LINEAR, GL_NEAREST
**Mipmapping**: Dual modes (Linear Mipmap Linear / Nearest Mipmap Nearest)

**Loaded Textures** (14 total):
- Floor tiles, shop facades, tree foliage/bark
- Grass, brick, concrete, metal, wood surfaces
- Roof, asphalt, procedurally generated textures
- Task-specific signage (stamping, welding, packing, inspection)

---

## ?? Rendering Techniques

### Lighting Models

**Phong Model**
```
L = ambient + ?(diffuse + specular)
I_diffuse = kd * (N · L) * light_color
I_specular = ks * (R · V)^n * light_color
```

**Gouraud Model**
```
Lighting calculations performed per-vertex in vertex shader
Colors interpolated across fragments (more efficient but less accurate)
```

### Day/Night Cycle
- **Smooth Transitions**: 24-hour simulation with smooth interpolation
  - Full day: 6:00 - 18:00
  - Twilight: 18:00 - 20:00 & 4:00 - 6:00
  - Full night: 20:00 - 4:00
- **Dynamic Color Adjustments**: Sky, ambient lighting, and directional light colors change with time

### Texture Blending
- Simple surface color rendering
- Pure texture mapping
- Blended texture + surface color (customizable mix factor)
- Dynamic blending during manufacturing stages

---

## ?? Manufacturing Simulation

### Workflow Stages
Each box processes through distinct manufacturing stages with color/texture changes:

1. **Raw Input** (Position: -25 to -20)
   - Red-brown colored boxes queued for processing
   
2. **Stamping Press** (Position: -20 to -9)
   - Hydraulic press with realistic dwell and cycle timing
   - Color transition to deeper red indicating pressed material
   - Micro-vibrations and resonance modeling

3. **Color Application** (Position: -9 to -7)
   - Vibrant neon color assignment
   - Simulated application chamber with Bézier geometry
   - 6 distinct color options for visual variety

4. **Welding & Texture** (Position: -7 to 6)
   - Advanced 6-axis robotic arm with kinematics
   - Realistic welding torch arc glow (intensity: 0.3-1.0)
   - Precision gripper control with adaptive force feedback
   - Material texture application

5. **Boxing/Packaging** (Position: 6 to 10)
   - Packaging system (toggleable)
   - Box color transitions to cardboard (0.92, 0.92, 0.90)
   - Pneumatic/mechanical sound effects (simulated via timing)

6. **Lift & Distribution** (Position: 10 to 40)
   - Hydraulic lift system with collision detection
   - Automatic unloading on top floor
   - Palletization (16 boxes per pallet)
   - Final product staging

### Machine Kinematics

**Stamping Press**
- Approach phase: 0-10% of cycle (cubic easing acceleration)
- Dwell phase: 10-50% (pressure maintenance with vibrations)
- Return phase: 50-85% (cubic easing deceleration)
- Pause: 85-100% (wait for next aligned box)

**Welding Robot**
- Base rotation: Continuous Z-axis rotation (18°/sec)
- Joint 2: Sinusoidal shoulder motion with seam-following capability
- Gripper: 4-finger precision grasp with force control
- Arc glow: Synchronized with weld cycle (intensity variation)

**Conveyor System**
- Speed control: 0.2 to 3.0 units/second
- Collision detection with lift system
- Smooth path following with Catmull-Rom interpolation
- Visual curve display with configurable radius

---

## ?? Project Structure

```
Graphics_Project/
??? src/
?   ??? main.cpp              # Main application entry point
??? include/
?   ??? shader.h              # Shader compilation and management
?   ??? sphere.h              # Sphere mesh generation
?   ??? pyramid.h             # Pyramid geometry
?   ??? fractal_tree.h        # Fractal tree generation
?   ??? bezierCurve.h         # Bézier surface computation
?   ??? curveApplications.h   # Catmull-Rom splines
?   ??? basic_camera.h        # Free-look camera implementation
?   ??? pointLight.h          # Point light structure
?   ??? directionalLight.h    # Directional light (sun) structure
?   ??? spotLight.h           # Spot light structure
?   ??? complex_machine.h     # Machine kinematics (press, robot, etc.)
?   ??? scene_renderer.h      # Rendering functions
?   ??? scene_state.h         # Global scene state variables
?   ??? resource_utils.h      # Resource file finding utilities
?   ??? stb_image.h           # Image loading library
?   ??? [additional headers]
??? shaders/
?   ??? vertexShaderForPhongShading.vs
?   ??? fragmentShaderForPhongShading.fs
?   ??? vertexShaderForGouraudShading.vs
?   ??? fragmentShaderForGouraudShading.fs
?   ??? vertexShader.vs
?   ??? fragmentShader.fs
??? textures/
?   ??? floor_tiles_texture.jpg
?   ??? fashion_shop.png
?   ??? tech_shop.png
?   ??? gems_shop.jpg
?   ??? tree_leaves_texture.png
?   ??? tree_texture.jpg
?   ??? grass.jpg
?   ??? industry.jpg
?   ??? [additional textures]
??? README.md                  # This file
```

---

## ?? Getting Started

### Prerequisites
- Modern C++ compiler (C++11 minimum)
- CMake 3.10+
- OpenGL 3.3 capable graphics card
- Dependencies: GLFW, GLAD, GLM, stb_image

### Building

```bash
# Clone repository
git clone https://github.com/tamim2007009/Graphics-Project.git
cd Graphics_Project

# Build with CMake
mkdir build
cd build
cmake ..
cmake --build .

# Run application
./Graphics_Project
```

### Running
```bash
# Default window size: 1400x900
# Press 'ESC' to exit
./Graphics_Project
```

---

## ?? Key Implementation Details

### Advanced Lighting Algorithms

1. **Multi-Light Composition**
   - Simultaneous rendering with 20+ lights
   - Deferred composition of ambient, diffuse, and specular components
   - Attenuation modeling: `1 / (1 + 0.09d + 0.032d²)`

2. **Dynamic Lighting**
   - Real-time light enable/disable toggle
   - Component-wise lighting control (ambient/diffuse/specular)
   - Shading model switching at runtime

3. **Day/Night Transition**
   - Smooth interpolation between day and night light properties
   - Time-based color correction for natural appearance
   - 3-hour per second simulation for demo purposes

### Procedural Generation

**Fractal Trees**
- Recursive branch generation with parametric control
- Height, radius, branching angle, and depth variation
- 10 instances with randomized parameters

**Bézier Surfaces**
- 2D Bézier curve generation with control points
- Surface of revolution for symmetric objects (tables, cones)
- Tessellation control for smooth geometry

**Catmull-Rom Interpolation**
- Smooth curve generation for escalator handrails
- Conveyor path visualization
- 100+ sample points for smooth rendering

### Texture Management

**Coordinate Systems**
- Standard 2D UV mapping for all geometry
- Texture tiling with configurable wrap modes
- Automatic coordinate generation for procedural meshes

**Filtering & Mipmapping**
- GL_LINEAR for smooth textures (default)
- GL_NEAREST for pixel-perfect appearance
- Automatic mipmap generation for memory efficiency

---

## ?? Testing & Validation

### Performance Metrics
- Target FPS: 60+ (depends on hardware)
- Scene complexity: ~100k triangles (varies with LOD)
- Active lights: 14 point + 2 directional + 4 spot = 20 lights
- Texture memory: ~50MB (14 textures, various resolutions)

### Quality Assurance
- Lighting consistency across all surfaces
- Smooth transitions during day/night cycle
- Realistic machine kinematics and timing
- Collision detection for conveyor/lift interaction
- Proper texture filtering and wrapping

---

## ?? Mathematical Foundations

### Phong Reflection Model
```
I = Ia·ka + ?[Il(kd(N·L) + ks(R·V)^n)]
```
- **Ia**: Ambient light intensity
- **Il**: Light intensity
- **kd**: Diffuse reflection coefficient
- **ks**: Specular reflection coefficient
- **n**: Shininess exponent

### Attenuation Formula
```
F_att = 1 / (1 + 0.09d + 0.032d²)
```
- Linear and quadratic falloff for realistic light propagation

### Day/Night Interpolation
```
color_final = mix(color_day, color_night, dnMix)
```
- Smooth GLSL mix function for seamless transitions
- Independent color adjustment for sky, ambient, and directional light

---

## ?? Troubleshooting

### Common Issues

| Issue | Solution |
|-------|----------|
| Black screen | Check texture loading; verify shaders compile correctly |
| No lighting visible | Ensure lights are enabled (keys 1-5); check shader uniforms |
| Texture distortion | Try changing wrap mode (U key) or filter mode (Y key) |
| Low frame rate | Reduce resolution, toggle textures (T key), or use simpler viewport |
| Camera clipping through geometry | Adjust near/far plane or camera height |

---

## ?? Future Enhancements

1. **Advanced Features**
   - Normal mapping for enhanced surface detail
   - Parallax mapping for depth illusion
   - Shadow mapping (PCF) for realistic shadows
   - Screen-space ambient occlusion (SSAO)

2. **Performance Optimization**
   - Frustum culling for off-screen objects
   - Instancing for repeated geometry
   - Compute shaders for physics simulation
   - Level-of-Detail (LOD) system

3. **Content Expansion**
   - Additional factory zones
   - More complex machinery
   - Multi-floor exploration
   - Interactive object manipulation

4. **User Interface**
   - HUD with real-time statistics
   - Performance metrics display
   - Scene graph visualization
   - Configuration menu system

---

## ?? License

This project is part of an academic graphics programming assignment. 

---

## ????? Author

**Tamim**  
Graphics Programming Portfolio Project  
[GitHub Repository](https://github.com/tamim2007009/Graphics-Project)

---

## ?? Acknowledgments

- OpenGL Tutorial Community for reference materials
- LearnOpenGL.com for foundational graphics concepts
- GLM, GLFW, and GLAD library maintainers
- NVIDIA and Khronos Group for OpenGL specifications

---

## ?? Support & Feedback

For questions, bug reports, or suggestions:
- Open an issue on the GitHub repository
- Refer to shader compilation errors in console output
- Check shader uniforms are properly configured in shader classes

---

**Last Updated**: 2024  
**Status**: Production Ready  
**OpenGL Version**: 3.3 Core Profile  
**C++ Standard**: C++11/14
