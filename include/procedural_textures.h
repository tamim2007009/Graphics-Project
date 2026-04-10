// procedural_textures.h
// Fallback procedural texture generation for missing texture files
// Generates simple tiling patterns programmatically

#ifndef PROCEDURAL_TEXTURES_H
#define PROCEDURAL_TEXTURES_H

#include <glad/glad.h>
#include <cmath>
#include <cstdint>

class ProceduralTextures {
public:
    // Generate a simple brick texture (512x512)
    static unsigned int generateBrickTexture() {
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
                
                // Add variation
                color += ((x ^ y) & 0x0F) - 8;
                
                data[idx] = data[idx + 1] = data[idx + 2] = color;
            }
        }
        
        return createGLTexture(data, width, height);
    }
    
    // Generate concrete texture (512x512)
    static unsigned int generateConcreteTexture() {
        const int width = 512, height = 512;
        unsigned char* data = new unsigned char[width * height * 3];
        
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int idx = (y * width + x) * 3;
                
                // Concrete base color
                unsigned char base = 150;
                unsigned char color = base;
                
                // Add noise for weathering
                color += ((x * 73 + y * 97) % 256) > 200 ? 30 : -20;
                color += ((x >> 4) ^ (y >> 4)) & 0x1F;
                
                data[idx] = data[idx + 1] = data[idx + 2] = color;
            }
        }
        
        return createGLTexture(data, width, height);
    }
    
    // Generate metal texture (512x512)
    static unsigned int generateMetalTexture() {
        const int width = 512, height = 512;
        unsigned char* data = new unsigned char[width * height * 3];
        
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int idx = (y * width + x) * 3;
                
                // Metal base (silver-gray)
                unsigned char base = 160;
                
                // Brushed metal effect with horizontal lines
                unsigned char color = base;
                if ((y / 2) % 8 < 2) {
                    color -= 20;
                }
                
                // Add subtle noise
                color += ((x * 31 + y * 47) % 256) > 220 ? 15 : -10;
                
                data[idx] = data[idx + 1] = data[idx + 2] = color;
            }
        }
        
        return createGLTexture(data, width, height);
    }
    
    // Generate wood texture (512x512)
    static unsigned int generateWoodTexture() {
        const int width = 512, height = 512;
        unsigned char* data = new unsigned char[width * height * 3];
        
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int idx = (y * width + x) * 3;
                
                // Wood plank base (brown)
                unsigned char base_r = 139;
                unsigned char base_g = 90;
                unsigned char base_b = 43;
                
                // Wood grain effect
                int plank = (x / 64) % 4;
                unsigned char variation = ((y + plank * 128) * 73) % 256;
                
                unsigned char r = base_r + (variation > 200 ? 30 : -20);
                unsigned char g = base_g + (variation > 200 ? 15 : -10);
                unsigned char b = base_b + (variation > 200 ? 10 : -5);
                
                data[idx] = r;
                data[idx + 1] = g;
                data[idx + 2] = b;
            }
        }
        
        return createGLTexture(data, width, height);
    }
    
    // Generate asphalt texture (512x512)
    static unsigned int generateAsphaltTexture() {
        const int width = 512, height = 512;
        unsigned char* data = new unsigned char[width * height * 3];
        
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int idx = (y * width + x) * 3;
                
                // Asphalt base (dark gray)
                unsigned char base = 80;
                unsigned char color = base;
                
                // Aggregate stones
                unsigned char noise = ((x * 101 + y * 137) % 256);
                if (noise > 240) {
                    color = 130; // light stone
                } else if (noise > 200) {
                    color = 110; // medium stone
                } else {
                    color += (noise % 30) - 15;
                }
                
                data[idx] = data[idx + 1] = data[idx + 2] = color;
            }
        }
        
        return createGLTexture(data, width, height);
    }
    
    // Generate grass texture (512x512)
    static unsigned int generateGrassTexture() {
        const int width = 512, height = 512;
        unsigned char* data = new unsigned char[width * height * 3];
        
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int idx = (y * width + x) * 3;
                
                // Grass colors
                unsigned char base_r = 76;
                unsigned char base_g = 175;
                unsigned char base_b = 80;
                
                // Variation
                unsigned char noise = ((x * 43 + y * 61) % 256);
                unsigned char variation = (noise & 0x7F);
                
                unsigned char r = base_r + (variation > 128 ? 20 : -15);
                unsigned char g = base_g + (variation > 128 ? 10 : -10);
                unsigned char b = base_b + (variation > 128 ? 15 : -10);
                
                data[idx] = r;
                data[idx + 1] = g;
                data[idx + 2] = b;
            }
        }
        
        return createGLTexture(data, width, height);
    }

private:
    static unsigned int createGLTexture(unsigned char* data, int width, int height) {
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        glBindTexture(GL_TEXTURE_2D, 0);
        
        delete[] data;
        return textureID;
    }
};

#endif // PROCEDURAL_TEXTURES_H
