#pragma once

#include "pch.hpp"
#include "Loader.hpp"

class Gui {
public:
    void gui(std::vector<Object>& objects, std::vector<std::string>& items, nlohmann::json& scene, Object& hitbox, std::vector<Light>& lights);
    void RenderGui();
    void guiGame(Camera camera);
    std::string getCurrentScenePath() const { return CurrentScenePath; }
    int currentLight = 0;
private:
    std::string NewModelPath = "assets/";
    std::string NewModelName = "";
    std::string ScenePath = "saves/";
    std::string CurrentScenePath = "saves/scene.json";
    int currentItem = 0;
    int currentHitbox = 0;
    Loader loader;
    nlohmann::json NewScene;
};