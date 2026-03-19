#pragma once

#include "pch.hpp"
#include "physics.hpp"

class Game
{
public:
    void Movement(float deltaTime, Camera &camera, float moveRight, float moveForward, bool grounded);
    void Jump(Camera &camera, float jumpStrength, bool& grounded);
private:
    void ApplyFriction(glm::vec3& velocity, float friction, float deltaTime);
    void Accelerate(glm::vec3& velocity, const glm::vec3& wishDir, float wishSpeed, float accel, float deltaTime);
};