# Brick.jpg Texture Loading - Complete Troubleshooting Guide

## Why brick.jpg is NOT Loading

### Common Causes:
1. **File doesn't exist** - `brick.jpg` file missing from textures folder
2. **Wrong directory** - Executable running from different working directory
3. **Path issues** - `findResource()` can't locate the file
4. **Invalid format** - File is corrupted or not a valid JPEG
5. **Permission issues** - File not readable

---

## SOLUTION 1: Create the brick.jpg Texture File

### Option A: Using PowerShell Script (Windows)
```powershell
# Create a procedurally generated brick texture

Add-Type -AssemblyName System.Drawing

$texDir = "C:\Users\USER\Desktop\Graphics-Project\textures"
if (!(Test-Path $texDir)) { New-Item -ItemType Directory -Path $texDir | Out-Null }

function New-BrickTexture {
    $bmp = New-Object System.Drawing.Bitmap 512, 512
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
    
    # Background - dark brown
    $g.Clear([System.Drawing.Color]::FromArgb(180, 100, 60))
    
    # Draw bricks with mortar lines
    for ($row = 0; $row -lt 16; $row++) {
        for ($col = 0; $col -lt 8; $col++) {
            $x = $col * 64 + ($row % 2) * 32
            $y = $row * 32
            
            # Draw brick
            $brush = New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::FromArgb(200, 120, 80))
            $g.FillRectangle($brush, $x, $y, 62, 30)
            
            # Draw mortar
            $pen = New-Object System.Drawing.Pen ([System.Drawing.Color]::FromArgb(220, 200, 180), 2)
            $g.DrawRectangle($pen, $x, $y, 62, 30)
            
            $brush.Dispose(); $pen.Dispose()
        }
    }
    
    $bmp.Save((Join-Path $texDir 'brick.jpg'), [System.Drawing.Imaging.ImageFormat]::Jpeg)
    $g.Dispose(); $bmp.Dispose()
}

New-BrickTexture
Write-Host "brick.jpg created at: $texDir\brick.jpg"
```

### Option B: Use an Existing Image
1. Find a brick texture image online
2. Save it as `brick.jpg`
3. Place it in: `C:\Users\USER\Desktop\Graphics-Project\textures\`

---

## SOLUTION 2: Fix the Path Resolution

### Step 1: Verify Your Directory Structure

Your project should have:
```
C:\Users\USER\Desktop\Graphics-Project\
??? textures/
?   ??? brick.jpg                    ? MUST BE HERE
?   ??? floor_tiles_texture.jpg
?   ??? grass.jpg
?   ??? industry.jpg
?   ??? ... other textures
??? include/
?   ??? procedural_textures.h
?   ??? resource_utils.h
?   ??? ...
??? src/
?   ??? main.cpp
??? ...
```

### Step 2: Update resource_utils.h (if needed)

Replace the existing `include/resource_utils.h` with this improved version:

```cpp
#ifndef RESOURCE_UTILS_H
#define RESOURCE_UTILS_H

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

inline bool fileExists(const std::string &path) {
    std::ifstream f(path);
    return f.good();
}

inline std::string findResource(const std::string &name) {
    // Print current working directory for debugging
    static bool printed = false;
    if (!printed) {
        std::cout << "\n[Working Directory: " << fs::current_path().string() << "]\n" << std::endl;
        printed = true;
    }
    
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
        std::string("../assets/") + name,
        std::string("../../textures/") + name,
        std::string("../../../textures/") + name
    };
    
    // Check for a sibling project folder
    for (int depth = 0; depth < 5; ++depth) {
        std::string up;
        for (int i = 0; i < depth; ++i) up += "../";
        candidates.push_back(up + std::string("Graphics_Project/") + name);
        candidates.push_back(up + std::string("Graphics_Project/textures/") + name);
        candidates.push_back(up + std::string("Graphics_Project/resources/") + name);
    }
    
    for (const auto &c : candidates) {
        if (fileExists(c)) {
            std::cout << "? Found resource: " << name << " -> " << c << std::endl;
            return c;
        }
    }
    
    std::cout << "? Resource NOT found: " << name << std::endl;
    std::cout << "  Searched paths:" << std::endl;
    for (const auto& c : candidates) {
        std::cout << "    - " << c << std::endl;
    }
    
    return name; // Return original name as fallback
}

#endif
```

---

## SOLUTION 3: Use Enhanced Texture Loader with Diagnostics

### Step 1: Use the new texture loader in main.cpp

Replace your `loadSceneTextures()` function with:

```cpp
void loadSceneTextures() {
    cout << "\n--- LOADING TEXTURES WITH ENHANCED DIAGNOSTICS ---\n" << endl;
    stbi_set_flip_vertically_on_load(true);
    
    // Print search paths for brick.jpg
    #include "../include/texture_loader_enhanced.h"
    TextureLoaderEnhanced::printSearchPaths("textures/brick.jpg");
    
    // Load brick texture with diagnostics
    cout << "Loading brick texture..." << endl;
    texBrick = TextureLoaderEnhanced::loadTextureWithDiagnostics("textures/brick.jpg");
    
    if (texBrick == 0) {
        cout << "\n? Brick texture failed to load. Generating procedural fallback..." << endl;
        texBrick = TextureFallbackSystem::generateBrickTexture();
        cout << "? Procedural brick texture generated (ID: " << texBrick << ")" << endl;
    }
    
    // Load other textures (similar pattern)...
}
```

---

## SOLUTION 4: Verify brick.jpg File

### Step 1: Check file exists
```powershell
Test-Path "C:\Users\USER\Desktop\Graphics-Project\textures\brick.jpg"
```

### Step 2: Check file properties
```powershell
Get-Item "C:\Users\USER\Desktop\Graphics-Project\textures\brick.jpg" | Select-Object Length, LastWriteTime
```

### Step 3: Verify it's a valid JPEG
- Try opening it in an image viewer
- File size should be > 1KB
- Should display without errors

---

## SOLUTION 5: Programmatically Generate brick.jpg

Use this C++ code to generate the texture on startup:

```cpp
#include <stb_image_write.h>

void generateBrickTextureFile() {
    const int width = 512, height = 512;
    unsigned char* data = new unsigned char[width * height * 3];
    
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = (y * width + x) * 3;
            
            // Brick pattern
            int brick_y = (y / 32) % 2;
            int local_x = (x + brick_y * 16) % 64;
            int local_y = y % 32;
            
            unsigned char base = 180;
            unsigned char mortar = 200;
            unsigned char color = (local_x < 2 || local_y < 2) ? mortar : base;
            
            color += ((x ^ y) & 0x0F) - 8;
            
            data[idx] = data[idx + 1] = data[idx + 2] = color;
        }
    }
    
    // Save to disk
    int result = stbi_write_jpg("textures/brick.jpg", width, height, 3, data, 95);
    
    if (result) {
        std::cout << "? brick.jpg generated successfully!" << std::endl;
    } else {
        std::cout << "? Failed to generate brick.jpg" << std::endl;
    }
    
    delete[] data;
}
```

---

## SOLUTION 6: Fallback System (Already Implemented)

Your code already has this, but verify it's working:

```cpp
texBrick = loadTexture(findResource("textures/brick.jpg").c_str());
if (texBrick == 0) {
    cout << "Brick not found, generating procedural..." << endl;
    texBrick = TextureFallbackSystem::generateBrickTexture();  // ? This should trigger
}
```

---

## Quick Checklist

- [ ] Verify `textures/` folder exists
- [ ] Verify `textures/brick.jpg` exists and is readable
- [ ] Check file size > 1KB
- [ ] Try opening it in an image viewer
- [ ] Run program and check console output
- [ ] If still failing, use PowerShell script to generate it
- [ ] Verify procedural fallback is being used if needed

---

## Expected Console Output (Success)

```
? Found resource: textures/brick.jpg -> textures/brick.jpg
? Image loaded: 512x512 (3 channels)
? Texture created (ID: 3)
```

## Expected Console Output (Fallback)

```
? Resource NOT found: textures/brick.jpg
Brick texture not found, generating procedural brick texture...
? Brick texture generated
```

---

## If All Else Fails

1. **Check permissions**: Make sure user can read files in textures/
2. **Try absolute path**: Use `C:\Users\USER\Desktop\Graphics-Project\textures\brick.jpg` directly
3. **Rebuild project**: Clean and rebuild the entire solution
4. **Check Visual Studio output**: Confirm working directory in Output pane
5. **Use debugger**: Set breakpoint in `loadTexture()` and check path values
