#include "physics.hpp"

void Physics::update(std::vector<Object>& objects, float deltaTime, Camera& camera, bool EngineMode)
{
    ZoneScoped;

    for (auto& obj : objects)
    {
        size_t i = &obj - &objects[0];
        if (obj.physicsEnabled)
        {
            obj.physicsObject.velocity += glm::vec3(0.0f, -9.81f, 0.0f) * deltaTime;
            glm::vec3 oldPos = obj.Position;
            obj.Position += obj.physicsObject.velocity * deltaTime;

            for (size_t j = 0; j < objects.size(); j++)
            {
                if (i == j)
                    continue;
                if (checkCollision(obj, objects[j]))
                {
                    obj.Position = oldPos;
                }
            }
        }
    }

    if (!EngineMode)
    {
        Object cameraObject;
        cameraObject.Position = camera.Position;
        cameraObject.physicsObject.hitbox.push_back(Hitbox());
        cameraObject.physicsObject.hitbox.back().halfSize = glm::vec3(0.5f, 2.0f, 0.5f);
        cameraObject.physicsObject.hitbox.back().centre = glm::vec3(0.0f, 0.0f, 0.0f);
        cameraObject.physicsEnabled = true;
        cameraObject.physicsObject.mass = 1.0f;
        cameraObject.physicsObject.velocity = camera.Velocity;

        camera.Velocity += glm::vec3(0.0f, -9.81f * 1.0f, 0.0f) * deltaTime;
        cameraObject.physicsObject.velocity = camera.Velocity;
        glm::vec3 oldPos = cameraObject.Position;
        cameraObject.Position += cameraObject.physicsObject.velocity * deltaTime;

        grounded = false;

        for (size_t j = 0; j < objects.size(); j++)
        {
            checkCamCollision(cameraObject, objects[j], camera, oldPos);
        }

        camera.Position = cameraObject.Position;
    }
}

bool Physics::checkCollision(const Object& a, const Object& b)
{
    bool colliding = false;
    for (int i = 0; i < a.physicsObject.hitbox.size(); i++)
    {
        for (int o = 0; o < b.physicsObject.hitbox.size(); o++)
        {
            glm::vec3 worldCenterA = a.Position + a.physicsObject.hitbox[i].centre;
            glm::vec3 worldCenterB = b.Position + b.physicsObject.hitbox[o].centre;

            glm::vec3 delta = glm::abs(worldCenterA - worldCenterB);
            glm::vec3 totalHalf = a.physicsObject.hitbox[i].halfSize + b.physicsObject.hitbox[o].halfSize;
            penetration = totalHalf - delta;

            bool collidingTemp = delta.x <= totalHalf.x && delta.y <= totalHalf.y && delta.z <= totalHalf.z;
            if (collidingTemp)
            {
                colliding = true;
            }
        }
    }

    return colliding;
}

void Physics::checkCamCollision(Object& a, Object& b, Camera& camera, glm::vec3& oldPos)
{
    bool colliding = false;
    glm::vec3 worldCenterA;
    glm::vec3 worldCenterB;
    for (int i = 0; i < a.physicsObject.hitbox.size(); i++)
    {
        for (int o = 0; o < b.physicsObject.hitbox.size(); o++)
        {
            worldCenterA = a.Position + a.physicsObject.hitbox[i].centre;
            worldCenterB = b.Position + b.physicsObject.hitbox[o].centre;

            glm::vec3 delta = glm::abs(worldCenterA - worldCenterB);
            glm::vec3 totalHalf = a.physicsObject.hitbox[i].halfSize + b.physicsObject.hitbox[o].halfSize;
            penetration = totalHalf - delta;

            bool collidingTemp = delta.x <= totalHalf.x && delta.y <= totalHalf.y && delta.z <= totalHalf.z;

            if (collidingTemp)
            {
                if (penetration.x < penetration.y && penetration.x < penetration.z)
                {
                    if (b.jumpPad.enabled == true)
                    {
                        camera.Velocity.x += b.jumpPad.direction[0] * b.jumpPad.force;
                        camera.Velocity.y += b.jumpPad.direction[1] * b.jumpPad.force;
                        camera.Velocity.z += b.jumpPad.direction[2] * b.jumpPad.force;
                    }
                    else
                    {
                        a.Position.x = oldPos.x;
                        camera.Velocity.x = 0.0f;
                    }
                }
                else if (penetration.y < penetration.x && penetration.y < penetration.z)
                {
                    if (b.jumpPad.enabled == true)
                    {
                        camera.Velocity.x += b.jumpPad.direction[0] * b.jumpPad.force;
                        camera.Velocity.y += b.jumpPad.direction[1] * b.jumpPad.force;
                        camera.Velocity.z += b.jumpPad.direction[2] * b.jumpPad.force;
                    }
                    else
                    {
                        a.Position.y = oldPos.y;
                        camera.Velocity.y = 0.0f;
                        bool fromAbove = worldCenterA.y > worldCenterB.y;
                        if (fromAbove)
                        {
                            grounded = true;
                        }
                    }
                }
                else if (penetration.z < penetration.y && penetration.z < penetration.x)
                {
                    if (b.jumpPad.enabled == true)
                    {
                        camera.Velocity.x += b.jumpPad.direction[0] * b.jumpPad.force;
                        camera.Velocity.y += b.jumpPad.direction[1] * b.jumpPad.force;
                        camera.Velocity.z += b.jumpPad.direction[2] * b.jumpPad.force;
                    }
                    else
                    {
                        a.Position.z = oldPos.z;
                        camera.Velocity.z = 0.0f;
                    }
                }
                return;
            }
        }
    }
}
