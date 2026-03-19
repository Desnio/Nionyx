#pragma once

#include <pch.hpp>

class Physics
{
public:
    void update(std::vector<Object>& objects, float deltaTime, Camera& camera, bool EngineMode);
    bool grounded = false;
private:
    bool checkCollision(const Object& a, const Object& b);
    void checkCamCollision(Object& a, Object& b, Camera& camera, glm::vec3& oldPos);
    bool checkCollisionGround(const Object& a, const Object& b);
    void resolveCollision(Object& a, Object& b);
    bool groundedAndColliding = false;
    bool groundedTemp = false;
    glm::vec3 penetration;
};
