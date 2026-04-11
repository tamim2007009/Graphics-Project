#ifndef RESOURCE_UTILS_H
#define RESOURCE_UTILS_H

#include <string>
#include <vector>
#include <fstream>
#include <iostream>

// Try to locate resource files in a few common relative locations so the
// executable can find shaders/textures even if the working directory differs.
inline bool fileExists(const std::string &path)
{
    std::ifstream f(path);
    return f.good();
}

inline std::string findResource(const std::string &name)
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
            std::cout << "Found resource: " << name << " -> " << c << std::endl;
            return c;
        }
    }
    std::cout << "Resource not found (will try original path): " << name << std::endl;
    return name;
}

#endif
