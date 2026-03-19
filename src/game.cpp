#include "game.hpp"

void Game::Movement(float deltaTime, Camera& camera, float moveRight, float moveForward, bool grounded)
{
    ZoneScopedN("Movement");
    glm::vec3 flatForward = normalize(camera.Front - dot(camera.Front, camera.WorldUp) * camera.WorldUp);
    glm::vec3 wishDir = flatForward * moveForward + camera.Right * moveRight;

    float wishSpeed = glm::length(wishDir);

    if (wishSpeed > 0.0f)
        wishDir = glm::normalize(wishDir);

    if (grounded)
    {
        ApplyFriction(camera.Velocity, 5.0f, deltaTime);
        if (wishSpeed > 0.0f)
            Accelerate(camera.Velocity, wishDir, wishSpeed * 5.0f, 10.0f, deltaTime);
    }
    else
    {
        if (wishSpeed > 0.0f)
            Accelerate(camera.Velocity, wishDir, wishSpeed * 5.0f, 1.0f, deltaTime);
    }
}

void Game::ApplyFriction(glm::vec3& velocity, float friction, float deltaTime)
{
    float speed = glm::length(velocity);
    if (speed <= 0.0f)
        return;

    float drop = speed * friction * deltaTime;
    float newSpeed = std::max(speed - drop, 0.0f);

    velocity *= newSpeed / speed;
}

void Game::Accelerate(glm::vec3& velocity, const glm::vec3& wishDir, float wishSpeed, float accel, float deltaTime)
{
    float currentSpeed = glm::dot(velocity, wishDir);
    float addSpeed = wishSpeed - currentSpeed;

    if (addSpeed <= 0.0f)
        return;

    float accelSpeed = accel * deltaTime * wishSpeed;

    if (accelSpeed > addSpeed)
        accelSpeed = addSpeed;

    velocity += wishDir * accelSpeed;
}

void Game::Jump(Camera& camera, float jumpStrength, bool& grounded)
{
    if (grounded)
    {
        camera.Velocity.y += jumpStrength;
        grounded = false;
    }
}
