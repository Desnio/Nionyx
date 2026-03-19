#include "Loader.hpp"
#include "game.hpp"
#include "gui.hpp"
#include "pch.hpp"
#include "physics.hpp"
#include "shader.hpp"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processEngineInput(GLFWwindow* window);
void processGameInput(GLFWwindow* window);
GLFWwindow* initialise();
void physicsRun(std::vector<Object>& objects, Camera& camera, bool EngineMode, Physics& physicsEngine);

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

// this is here because i got annoyed at all the warnings it was giving me saying "window is an unused parameter"
GLFWwindow* w;

std::vector<std::string> items;

nlohmann::json scene;

Gui gui;
App app;

Physics physicsEngine;

Game game;

bool running = true;

int main()
{
    GLFWwindow* window = initialise();

    std::vector<Object> objects;
    std::vector<Light> lights;
    nlohmann::json Config;
    Loader loader;
    {
        ZoneScopedN("Loading config");

        std::ifstream configFile("cfg/cfg.json");
        if (configFile.is_open())
        {
            configFile >> Config;
            configFile.close();
            if (Config["Type"] == "Engine")
            {
                app.modeType = Mode::ENGINE;
            }
            else if (Config["Type"] == "Game")
            {
                app.modeType = Mode::GAME;
            }
            else if (Config["Type"] == "Debug")
            {
                app.modeType = Mode::DEBUG;
            }
        }
        else
        {
            app.modeType = Mode::GAME;
        }
    }

    {
        ZoneScopedN("Loading objects") objects =
            loader.loadScene("saves/scene.json", scene, app.modeType != Mode::ENGINE);
        lights = loader.lights;
    }

    Object hitbox;
    Object LightObj;

    std::optional<Shader> DefaultShader;
    std::optional<Shader> HitboxShader;
    std::optional<Shader> LightShader;

    {
        ZoneScopedN("Load Shaders");
        DefaultShader.emplace("shaders/cube_shader.vs", "shaders/cube_shader.fs", app.modeType != Mode::ENGINE);
        if (app.modeType == Mode::ENGINE)
        {
            hitbox = loader.loadHitbox();
            loader.loadLight(LightObj);
            HitboxShader.emplace("shaders/hitbox.vs", "shaders/hitbox.fs", app.modeType != Mode::ENGINE);
            LightShader.emplace("shaders/light_shader.vs", "shaders/light_shader.fs", app.modeType != Mode::ENGINE);
        }
        else
        {
            camera.Position = glm::vec3(scene["Player"]["Camera Position"][0].get<float>(),
                                        scene["Player"]["Camera Position"][1].get<float>(),
                                        scene["Player"]["Camera Position"][2].get<float>());
        }
    }

    {
        ZoneScopedN("items loop");
        for (size_t i = 0; i < objects.size(); i++)
        {
            items.push_back(objects[i].Name);
        }
    }

    FrameMark;
    std::thread physicsthread(physicsRun, std::ref(objects), std::ref(camera), app.modeType == Mode::ENGINE,
                              std::ref(physicsEngine));

    FrameMark;
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        app.deltaTime = currentFrame - app.lastFrame;
        app.lastFrame = currentFrame;

        glfwPollEvents();
        if (app.modeType == Mode::ENGINE)
        {
            processEngineInput(window);
            gui.gui(objects, items, scene, hitbox, lights);
        }
        else if (app.modeType == Mode::DEBUG)
        {
            processGameInput(window);
            gui.guiGame(camera);
        }
        else if (app.modeType == Mode::GAME)
        {
            processGameInput(window);
        }

        if (app.counter > -1)
        {
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glClear(GL_DEPTH_BUFFER_BIT);
            app.counter = 0;
        }
        else
        {
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            app.counter++;
        }

        glm::mat4 projection;
        glm::mat4 view;
        {
            ZoneScopedN("Setup Shaders");
            DefaultShader->use();
            projection = glm::perspective(glm::radians(camera.Zoom), (float) app.mode->width / (float) app.mode->height,
                                          0.25f, 1000.0f);
            view = camera.GetViewMatrix();
            DefaultShader->setMat4("projection", projection);
            DefaultShader->setMat4("view", view);
            DefaultShader->setInt("lightCount", lights.size());
            DefaultShader->setVec3("viewPos", camera.Position);

            for (size_t i = 0; i < lights.size(); i++)
            {
                DefaultShader->setVec3("lights[" + std::to_string(i) + "].colour", lights[i].colour);
                DefaultShader->setVec3("lights[" + std::to_string(i) + "].position", lights[i].position);
                DefaultShader->setFloat("lights[" + std::to_string(i) + "].intensity", lights[i].intensity);
            }
        }

        glm::mat4 model;

        {
            ZoneScopedN("Render");
            for (unsigned long i = 0; i < objects.size(); i++)
            {
                model = glm::mat4(1.0f);
                model = glm::translate(model, objects[i].Position);
                model = glm::rotate(model, glm::radians(objects[i].Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
                model = glm::rotate(model, glm::radians(objects[i].Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::rotate(model, glm::radians(objects[i].Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
                for (Mesh& mesh : objects[i].meshes)
                {
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, mesh.textureID);
                    DefaultShader->setInt("tex", 0);
                    DefaultShader->setMat4("model", model);

                    glBindVertexArray(mesh.VAO);
                    glDrawElements(GL_TRIANGLES, mesh.numIndices, GL_UNSIGNED_INT, 0);
                    glBindVertexArray(0);
                }
            }
        }

        if (app.modeType == Mode::ENGINE)
        {
            for (int i = 0; i < hitbox.physicsObject.hitbox.size(); i++)
            {
                HitboxShader->use();
                HitboxShader->setMat4("projection", projection);
                HitboxShader->setMat4("view", view);
                model = glm::mat4(1.0f);
                model = glm::translate(model, hitbox.Position);
                model = glm::translate(model, hitbox.physicsObject.hitbox[i].centre);
                model = glm::scale(model, hitbox.physicsObject.hitbox[i].halfSize * 2.0f);
                HitboxShader->setMat4("model", model);

                glDisable(GL_DEPTH_TEST);
                glBindVertexArray(hitbox.meshes[0].VAO);
                glLineWidth(2.0f);
                glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
                glEnable(GL_DEPTH_TEST);
            }

            LightShader->use();
            LightShader->setMat4("projection", projection);
            LightShader->setMat4("view", view);

            model = glm::mat4(1.0f);
            model = glm::translate(model, lights[gui.currentLight].position);
            LightShader->setMat4("model", model);

            for (auto& mesh : LightObj.meshes)
            {
                glBindVertexArray(mesh.VAO);
                glDrawElements(GL_TRIANGLES, mesh.numIndices, GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
            }
        }

        if (app.modeType == Mode::ENGINE || app.modeType == Mode::DEBUG)
        {
            gui.RenderGui();
        }
        glfwSwapBuffers(window);
        FrameMark;
    }
    running = false;
    physicsthread.join();

    {
        ZoneScopedN("Save Scene");
        for (unsigned long i = 0; i < objects.size(); i++)
        {
            scene["Objects"][i]["Name"] = objects[i].Name;
            scene["Objects"][i]["File Path"] = objects[i].ModelPath;
            scene["Objects"][i]["Position"] = {objects[i].Position.x, objects[i].Position.y, objects[i].Position.z};
            scene["Objects"][i]["Rotation"] = {objects[i].Rotation.x, objects[i].Rotation.y, objects[i].Rotation.z};
            scene["Objects"][i]["Gravity"] = objects[i].physicsEnabled;
            scene["Objects"][i]["Mass"] = objects[i].physicsObject.mass;
            for (int o = 0; o < objects[i].physicsObject.hitbox.size(); o++)
            {
                scene["Objects"][i]["Hitbox"][o]["HalfPos"] = {objects[i].physicsObject.hitbox[o].halfSize.x,
                                                               objects[i].physicsObject.hitbox[o].halfSize.y,
                                                               objects[i].physicsObject.hitbox[o].halfSize.z};
                scene["Objects"][i]["Hitbox"][o]["Centre"] = {objects[i].physicsObject.hitbox[o].centre.x,
                                                              objects[i].physicsObject.hitbox[o].centre.y,
                                                              objects[i].physicsObject.hitbox[o].centre.z};
            }
            scene["Objects"][i]["Archive"] = objects[i].archive;
            scene["Objects"][i]["JumpPad"]["Enabled"] = objects[i].jumpPad.enabled;
            scene["Objects"][i]["JumpPad"]["Direction"] = {
                objects[i].jumpPad.direction[0], objects[i].jumpPad.direction[1], objects[i].jumpPad.direction[2]};
            scene["Objects"][i]["JumpPad"]["Force"] = objects[i].jumpPad.force;
        }

        for (unsigned long i = 0; i < lights.size(); i++)
        {
            scene["Lights"][i]["Colour"] = {lights[i].colour.r, lights[i].colour.g, lights[i].colour.b};
            scene["Lights"][i]["Intensity"] = lights[i].intensity;
            scene["Lights"][i]["Name"] = lights[i].name;
            scene["Lights"][i]["Position"] = {lights[i].position.x, lights[i].position.y, lights[i].position.z};
        }

        std::ofstream out(gui.getCurrentScenePath());
        out << scene.dump(4);
        out.close();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}

void processEngineInput(GLFWwindow* window)
{
    static bool escWasDown = false;
    bool escDown = glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;

    if (escDown && !escWasDown)
    {
        app.focused = !app.focused;
        glfwSetInputMode(window, GLFW_CURSOR, app.focused ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }
    escWasDown = escDown;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, app.deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, app.deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, app.deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, app.deltaTime);
}

void processGameInput(GLFWwindow* window)
{
    ZoneScopedN("Game Input");
    static bool escWasDown = false;
    bool escDown = glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;

    if (escDown && !escWasDown)
    {
        app.focused = !app.focused;
        glfwSetInputMode(window, GLFW_CURSOR, app.focused ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }
    escWasDown = escDown;

    float moveForward = 0.0f;
    float moveRight = 0.0f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        moveForward += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        moveForward -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        moveRight -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        moveRight += 1.0f;

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        game.Jump(camera, 5.0f, physicsEngine.grounded);

    game.Movement(app.deltaTime, camera, moveRight, moveForward, physicsEngine.grounded);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    w = window;
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (app.firstMouse)
    {
        app.lastX = xpos;
        app.lastY = ypos;
        app.firstMouse = false;
    }

    float xoffset = xpos - app.lastX;
    float yoffset = app.lastY - ypos;
    app.lastX = xpos;
    app.lastY = ypos;

    if (app.focused == true)
    {
        camera.ProcessMouseMovement(xoffset, yoffset);
    }
    w = window;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
    w = window;
    xoffset = xoffset * 2;
}

GLFWwindow* initialise()
{
    ZoneScoped;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    app.monitor = glfwGetPrimaryMonitor();
    app.mode = glfwGetVideoMode(app.monitor);
    GLFWwindow* window = glfwCreateWindow(app.mode->width, app.mode->height, "Nionyx", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    return window;
}

void physicsRun(std::vector<Object>& objects, Camera& camera, bool EngineMode, Physics& physicsEngine)
{
    tracy::SetThreadName("Physics");

    constexpr auto dt = std::chrono::milliseconds(20);

    while (running)
    {
        auto starttime = std::chrono::steady_clock::now();

        physicsEngine.update(objects, 0.02f, camera, EngineMode);

        std::this_thread::sleep_until(starttime + dt);
    }
}
