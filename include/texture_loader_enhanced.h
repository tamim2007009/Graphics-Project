#ifndef TEXTURE_LOADER_ENHANCED_H
#define TEXTURE_LOADER_ENHANCED_H

#include <glad/glad.h>
#include <stb_image.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

class TextureLoaderEnhanced {
public:
    /**
     * Enhanced texture loading with detailed diagnostics
     * Returns textureID on success, 0 on failure
     */
    static unsigned int loadTextureWithDiagnostics(const std::string& path) {
        std::cout << "\n--- LOADING TEXTURE ---" << std::endl;
        std::cout << "Requested path: " << path << std::endl;
        
        // Try to find the file
        std::string resolvedPath = findResourceEnhanced(path);
        
        if (resolvedPath.empty()) {
            std::cout << "ERROR: Could not find texture file: " << path << std::endl;
            return 0;
        }
        
        std::cout << "Resolved to: " << resolvedPath << std::endl;
        
        // Load the texture
        int w, h, ch;
        unsigned char* data = stbi_load(resolvedPath.c_str(), &w, &h, &ch, 0);
        
        if (!data) {
            std::cout << "ERROR: Failed to load image data from: " << resolvedPath << std::endl;
            std::cout << "STB Error: " << stbi_failure_reason() << std::endl;
            return 0;
        }
        
        std::cout << "? Image loaded: " << w << "x" << h << " (" << ch << " channels)" << std::endl;
        
        // Create OpenGL texture
        unsigned int id = 0;
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        
        GLenum internalFormat = (ch == 4) ? GL_RGBA : GL_RGB;
        GLenum format = (ch == 4) ? GL_RGBA : GL_RGB;
        
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(data);
        
        std::cout << "? Texture created (ID: " << id << ")" << std::endl;
        return id;
    }
    
    /**
     * Print all search paths for debugging
     */
    static void printSearchPaths(const std::string& filename) {
        std::cout << "\n=== TEXTURE SEARCH PATHS ===" << std::endl;
        std::cout << "Looking for: " << filename << std::endl << std::endl;
        
        std::vector<std::string> candidates = generateSearchPaths(filename);
        
        for (int i = 0; i < candidates.size(); ++i) {
            bool exists = fileExists(candidates[i]);
            std::string status = exists ? "? FOUND" : "? not found";
            std::cout << "[" << (i + 1) << "] " << status << " : " << candidates[i] << std::endl;
            
            if (exists) {
                std::cout << "    ^ This path exists!" << std::endl;
            }
        }
        std::cout << "\n";
    }
    
private:
    static bool fileExists(const std::string& path) {
        std::ifstream f(path);
        return f.good();
    }
    
    static std::vector<std::string> generateSearchPaths(const std::string& name) {
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
        
        // Check in Graphics_Project sibling folder
        for (int depth = 0; depth < 5; ++depth) {
            std::string up;
            for (int i = 0; i < depth; ++i) up += "../";
            candidates.push_back(up + std::string("Graphics_Project/") + name);
            candidates.push_back(up + std::string("Graphics_Project/textures/") + name);
            candidates.push_back(up + std::string("Graphics_Project/resources/") + name);
        }
        
        return candidates;
    }
    
    static std::string findResourceEnhanced(const std::string& name) {
        std::vector<std::string> candidates = generateSearchPaths(name);
        
        for (const auto& path : candidates) {
            if (fileExists(path)) {
                return path;
            }
        }
        
        return "";
    }
};

#endif // TEXTURE_LOADER_ENHANCED_H
