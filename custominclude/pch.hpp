#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <chrono>
#include <thread>
#include <optional>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_stdlib.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stb_image.h>

#include <json.hpp>

#include <lz4.h>

#include "tracy/Tracy.hpp"
#include "common/TracySystem.hpp"

enum Mode
{
    ENGINE,
    GAME,
    DEBUG
};

struct App
{
    float lastX;
    float lastY;
    bool firstMouse = true;
    
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    
    bool focused = true;
    
    GLFWmonitor* monitor;
    const GLFWvidmode* mode;

    int counter = 0;

    double fixedDelta = 1.0/60.0;
    float accumulator = 0.0f;
    double lastTime = 0.0f;
    double frameTime = 0.0f;
    int frameAcc = 0;
    float lastCheck = 0.0f;

    Mode modeType;
};

struct Mesh {
    unsigned int VAO, VBO, EBO;
    unsigned int numIndices;
    unsigned int textureID;
};

struct Hitbox
{
    glm::vec3 halfSize;
    glm::vec3 centre;
    std::string ID;
};

struct PhysicsObject
{
    float mass;
    glm::vec3 velocity;
    std::vector<Hitbox> hitbox;
};

struct Light
{
    glm::vec3 position;
    glm::vec3 colour;
    float intensity;
    std::string name;
};

struct JumpPad
{
    bool enabled;
    float direction[3];
    float force;
};

struct Object {
    std::string Name;
    std::string ModelPath;
    glm::vec3 Position;
    glm::vec3 Rotation;
    std::vector<Mesh> meshes;
    bool physicsEnabled = false;
    PhysicsObject physicsObject;
    std::string archive;
    JumpPad jumpPad;
};
