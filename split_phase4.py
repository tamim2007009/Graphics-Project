import re
import sys

with open('src/main.cpp', 'r', encoding='utf-8') as f:
    text = f.read()

match = re.search(r'void drawScene\(unsigned int &V, unsigned int &LV, Shader &ls, Shader &fs, glm::mat4 view, glm::mat4 proj\)[\s\S]+?(?=// INPUT)', text)
if not match:
    print("Could not find drawScene block via Regex")
    sys.exit(1)

ds_body = match.group(0)

# Replace sections with lambda wrappers for modular reading
ds_body = ds_body.replace('    // OUTDOOR ENVIRONMENT\n', '    auto drawOutdoor = [&]() {\n    // OUTDOOR ENVIRONMENT\n')
ds_body = ds_body.replace('    // PARKING LOT (Z: 27 to 55)\n', '    };\n    drawOutdoor();\n\n    auto drawParkingLot = [&]() {\n    // PARKING LOT (Z: 27 to 55)\n')
ds_body = ds_body.replace('    // MALL EXTERIOR (X:-25 to 25, Z:-25 to 25)\n', '    };\n    drawParkingLot();\n\n    auto drawMallExterior = [&]() {\n    // MALL EXTERIOR (X:-25 to 25, Z:-25 to 25)\n')
ds_body = ds_body.replace('    // FACTORY FLOOR & CEILING (industrial look)\n', '    };\n    drawMallExterior();\n\n    auto drawFactoryWarehouse = [&]() {\n    // FACTORY FLOOR & CEILING (industrial look)\n')
ds_body = ds_body.replace('    // ======== CLOTHING SHOP (X:4 to 24.5, Z:8 to 24)', '    };\n    drawFactoryWarehouse();\n\n    auto drawClothingShop = [&]() {\n    // ======== CLOTHING SHOP (X:4 to 24.5, Z:8 to 24)')
ds_body = ds_body.replace('    // ======== ESCALATOR (West wall, X:-22 to -18', '    };\n    drawClothingShop();\n\n    auto drawMallAmenities = [&]() {\n    // ======== ESCALATOR (West wall, X:-22 to -18')
ds_body = ds_body.replace('    // LIGHT FIXTURE GEOMETRY (visual only, lights off)', '    };\n    drawMallAmenities();\n\n    // LIGHT FIXTURE GEOMETRY (visual only, lights off)')

header_content = """#ifndef SCENE_RENDERER_H
#define SCENE_RENDERER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "shader.h"
#include "basic_camera.h"
#include "pointLight.h"
#include "spotLight.h"
#include "sphere.h"
#include "pyramid.h"
#include "bezierCurve.h"
#include "fractalTree.h"
#include "scene_state.h"
#include "scene_colors.h"
#include "render_utils.h"
#include "complex_machine.h"
#include "enhanced_signboard.h"

// External globals from main.cpp
extern BasicCamera basic_camera;
extern const float MH;
extern const float WT;

extern const int NUM_POINT_LIGHTS;
extern glm::vec3 plPos[];
extern PointLight *pointLights[];

extern const int NUM_SPOT_LIGHTS;
extern SpotLight* spotLights_arr[];

extern Sphere *sphWheel;
extern Sphere *sphHub;
extern Sphere *sphLeaf[];
extern Pyramid *treeCone;

extern const int NUM_TREES;
extern FractalTree* fractalTrees[];

extern BezierRevolvedSurface* bezierTable;
extern BezierArch* bezierArch;
extern BezierRevolvedSurface* bezierColorChamber;
extern BezierRevolvedSurface* mathCone;

extern std::vector<EnhancedSignboard> signboards;

inline """ + ds_body + """
#endif
"""

with open('include/scene_renderer.h', 'w', encoding='utf-8') as f:
    f.write(header_content)

new_text = text[:match.start()] + "\n" + text[match.end():]
new_text = new_text.replace('const float MH = 5.0f;', 'extern const float MH = 5.0f;')
new_text = new_text.replace('const float WT = 0.3f;', 'extern const float WT = 0.3f;')
new_text = new_text.replace('const int NUM_POINT_LIGHTS = 14;', 'extern const int NUM_POINT_LIGHTS = 14;')
new_text = new_text.replace('const int NUM_SPOT_LIGHTS = 4;', 'extern const int NUM_SPOT_LIGHTS = 4;')
new_text = new_text.replace('const int NUM_TREES = 10;', 'extern const int NUM_TREES = 10;')

# Inject the include header right before the scene_state include
inc_idx = new_text.find('#include "../include/scene_state.h"')
if inc_idx != -1:
    new_text = new_text[:inc_idx] + '#include "../include/scene_renderer.h"\n' + new_text[inc_idx:]

# Remove forward declaration of drawScene securely using Regex
new_text = re.sub(r'void drawScene\([^\;]+\);\n', '', new_text)

with open('src/main.cpp', 'w', encoding='utf-8') as f:
    f.write(new_text)

print("Modularization Phase 4 Successful")
