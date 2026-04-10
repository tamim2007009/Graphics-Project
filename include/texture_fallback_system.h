#ifndef TEXTURE_FALLBACK_SYSTEM_H
#define TEXTURE_FALLBACK_SYSTEM_H

#include <glad/glad.h>
#include <cmath>
#include <cstdint>
#include <iostream>

// Robust procedural texture generation system
// Generates high-quality fallback textures for missing texture files
class TextureFallbackSystem {
public:
    // ===== BRICK TEXTURE (industrial walls, masonry) =====
    static unsigned int generateBrickTexture() {
        const int width = 512, height = 512;
        unsigned char* data = new unsigned char[width * height * 3];
        
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int idx = (y * width + x) * 3;
                
                // Brick layout pattern (running bond)
                int brick_y = (y / 32) % 2;  // Row offset for bond pattern
                int local_x = (x + brick_y * 16) % 64;  // X position within brick
                int local_y = y % 32;  // Y position within brick
                
                // Base brick color (reddish-brown)
                unsigned char brick_base = 180;
                unsigned char mortar_base = 200;  // Light gray mortar
                
                // Mortar lines at edges (2px)
                unsigned char color;
                if (local_x < 2 || local_y < 2) {
                    color = mortar_base;  // Mortar
                } else {
                    color = brick_base;  // Brick
                }
                
                // Add natural color variation (stone-like texture)
                int variation = ((x * 73 + y * 97) % 256);
                if (variation > 200) {
                    color += 25;  // Lighter areas
                } else if (variation < 50) {
                    color -= 25;  // Darker areas
                }
                
                // Add fine surface texture (noise)
                int noise = ((x >> 2) ^ (y >> 2)) & 0x1F;
                color += (noise - 16) / 2;
                
                // Clamp to valid range
                color = (unsigned char)std::max(0, std::min(255, (int)color));
                
                data[idx] = data[idx + 1] = data[idx + 2] = color;
            }
        }
        
        return createGLTexture(data, width, height);
    }
    
    // ===== CONCRETE TEXTURE (factory floors, base surfaces) =====
    static unsigned int generateConcreteTexture() {
        const int width = 512, height = 512;
        unsigned char* data = new unsigned char[width * height * 3];
        
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int idx = (y * width + x) * 3;
                
                // Concrete base color (neutral gray)
                unsigned char base = 150;
                unsigned char color = base;
                
                // Aggregate particles (small stones in concrete)
                int particle_seed = (x * 73 + y * 97) % 256;
                if (particle_seed > 180) {
                    color += 35;  // Light aggregate
                } else if (particle_seed < 70) {
                    color -= 30;  // Dark aggregate
                }
                
                // Weathering cracks and stains
                int weathering = ((x * 131 + y * 151) % 256);
                if (weathering > 220) {
                    color -= 40;  // Dark stains
                } else if (weathering < 30) {
                    color += 20;  // Weathered lighter areas
                }
                
                // Surface texture (fine noise for roughness)
                int surface_noise = ((x >> 3) ^ (y >> 3)) & 0xFF;
                color += (surface_noise - 128) / 8;
                
                // Clamp to valid range
                color = (unsigned char)std::max(0, std::min(255, (int)color));
                
                data[idx] = data[idx + 1] = data[idx + 2] = color;
            }
        }
        
        return createGLTexture(data, width, height);
    }
    
    // ===== METAL TEXTURE (brushed steel, industrial surfaces) =====
    static unsigned int generateMetalTexture() {
        const int width = 512, height = 512;
        unsigned char* data = new unsigned char[width * height * 3];
        
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int idx = (y * width + x) * 3;
                
                // Brushed metal base color (silver-gray)
                unsigned char base = 160;
                unsigned char color = base;
                
                // Brushed effect (horizontal directional grain)
                int brush_pattern = (y / 2) % 8;
                if (brush_pattern < 2) {
                    color -= 15;  // Darker brush strokes
                } else if (brush_pattern > 5) {
                    color += 10;  // Slightly lighter areas
                }
                
                // Subtle metallic highlights
                int highlight = (x * 37) % 256;
                if (highlight > 220) {
                    color += 25;  // Specular highlights
                } else if (highlight < 40) {
                    color -= 20;  // Shadow areas
                }
                
                // Fine surface scratches (realistic wear)
                int scratches = ((x * 41 + y * 43) % 256);
                if (scratches > 240 || scratches < 20) {
                    color += (scratches & 1) ? -3 : 3;
                }
                
                // Clamp to valid range
                color = (unsigned char)std::max(0, std::min(255, (int)color));
                
                data[idx] = data[idx + 1] = data[idx + 2] = color;
            }
        }
        
        return createGLTexture(data, width, height);
    }
    
    // ===== WOOD TEXTURE (furniture, natural elements) =====
    static unsigned int generateWoodTexture() {
        const int width = 512, height = 512;
        unsigned char* data = new unsigned char[width * height * 3];
        
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int idx = (y * width + x) * 3;
                
                // Wood plank layout (vertical grain)
                int plank = (x / 64) % 4;  // 4 planks across
                int plank_y = y;
                
                // Base wood color (medium brown)
                unsigned char base_r = 139, base_g = 90, base_b = 43;
                
                // Wood grain variation (changes with plank offset)
                unsigned int grain_seed = (plank_y * 73 + plank * 128) & 0xFFFF;
                int grain_variation = ((grain_seed * 41 + x * 17) % 256);
                
                // Color variation based on grain
                unsigned char r = base_r;
                unsigned char g = base_g;
                unsigned char b = base_b;
                
                if (grain_variation > 200) {
                    // Lighter wood grain
                    r += 30;
                    g += 20;
                    b += 15;
                } else if (grain_variation < 50) {
                    // Darker wood grain
                    r -= 25;
                    g -= 15;
                    b -= 10;
                }
                
                // Add fine wood texture detail
                int detail = ((x >> 2) ^ (y >> 2)) & 0x0F;
                r += (detail - 8) / 2;
                g += (detail - 8) / 3;
                b += (detail - 8) / 4;
                
                // Clamp to valid ranges
                r = (unsigned char)std::max(0, std::min(255, (int)r));
                g = (unsigned char)std::max(0, std::min(255, (int)g));
                b = (unsigned char)std::max(0, std::min(255, (int)b));
                
                data[idx] = r;
                data[idx + 1] = g;
                data[idx + 2] = b;
            }
        }
        
        return createGLTexture(data, width, height);
    }
    
    // ===== ASPHALT TEXTURE (parking lots, roads) =====
    static unsigned int generateAsphaltTexture() {
        const int width = 512, height = 512;
        unsigned char* data = new unsigned char[width * height * 3];
        
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int idx = (y * width + x) * 3;
                
                // Asphalt base color (dark gray)
                unsigned char base = 60;
                unsigned char color = base;
                
                // Aggregate stones embedded in asphalt (pebble pattern)
                int pebble_seed = (x * 83 + y * 107) % 256;
                if (pebble_seed > 180) {
                    // Light stone aggregate
                    color += 50;
                } else if (pebble_seed > 100) {
                    // Medium stone
                    color += 20;
                } else if (pebble_seed < 40) {
                    // Dark stone
                    color -= 15;
                }
                
                // Weathering and aging effect
                int aging = ((x >> 3) ^ (y >> 3)) & 0xFF;
                if (aging > 220) {
                    color += 15;  // Lighter worn areas
                }
                
                // Subtle oil stains
                int stain_pattern = ((x * 97 + y * 113) % 256);
                if (stain_pattern > 240) {
                    color -= 20;  // Dark stains
                }
                
                // Clamp to valid range
                color = (unsigned char)std::max(0, std::min(255, (int)color));
                
                data[idx] = data[idx + 1] = data[idx + 2] = color;
            }
        }
        
        return createGLTexture(data, width, height);
    }
    
    // ===== ROOF TILES TEXTURE (building roofs, decorative surfaces) =====
    static unsigned int generateRoofTileTexture() {
        // Reuse brick pattern as roof tiles have similar structure
        return generateBrickTexture();
    }

private:
    // Helper function to create OpenGL texture from raw pixel data
    static unsigned int createGLTexture(unsigned char* data, int width, int height) {
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        // Upload texture data to GPU
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        
        // Generate mipmaps for efficient rendering at different scales
        glGenerateMipmap(GL_TEXTURE_2D);
        
        // Set default texture parameters for best quality
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        // Clean up temporary pixel data
        delete[] data;
        
        // Unbind texture
        glBindTexture(GL_TEXTURE_2D, 0);
        
        return textureID;
    }
};

#endif // TEXTURE_FALLBACK_SYSTEM_H
