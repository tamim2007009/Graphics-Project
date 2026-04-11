#ifndef RESOURCE_UTILS_H
#define RESOURCE_UTILS_H

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <filesystem>

// Try to locate resource files in a few common relative locations so the
// executable can find shaders/textures even if the working directory differs.
inline bool fileExists(const std::string &path)
{
    try {
        std::ifstream f(path);
        bool exists = f.good();
        f.close();
        return exists;
    } catch (...) {
        return false;
    }
}

inline std::string findResource(const std::string &name)
{
    // Print current working directory once for debugging
    static bool printed_cwd = false;
    if (!printed_cwd) {
        try {
            std::cout << "[CWD] " << std::filesystem::current_path().string() << std::endl;
        } catch (...) {
            std::cout << "[CWD] Unable to determine current working directory" << std::endl;
        }
        printed_cwd = true;
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
        std::string("../../textures/") + name,
        std::string("../../../textures/") + name,
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
            std::cout << "? Found: " << name << " -> " << c << std::endl;
            return c;
        }
    }
    
    std::cout << "? Resource NOT found: " << name << std::endl;
    std::cout << "  Checked paths:" << std::endl;
    for (int i = 0; i < std::min(5, (int)candidates.size()); ++i) {
        std::cout << "    - " << candidates[i] << std::endl;
    }
    std::cout << "  (and " << (candidates.size() - 5) << " more paths)" << std::endl;
    
    return name;
}

#endif
