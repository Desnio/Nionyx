#include "gui.hpp"

void Gui::gui(std::vector<Object>& objects, std::vector<std::string>& items, nlohmann::json& scene, Object& hitbox,
              std::vector<Light>& lights)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Load");
    ImGui::InputText("File Path", &NewModelPath);
    ImGui::InputText("Name", &NewModelName);
    if (ImGui::Button("Load Model"))
    {
        objects.push_back(Object());
        objects.back().Name = NewModelName;
        objects.back().ModelPath = NewModelPath;
        objects.back().Position = glm::vec3(0.0f, 0.0f, 0.0f);
        objects.back().Rotation = glm::vec3(0.0f, 0.0f, 0.0f);
        objects.back().physicsObject.hitbox.push_back(Hitbox());
        objects.back().physicsObject.hitbox.back().centre = glm::vec3(0.0f, 0.0f, 0.0f);
        objects.back().physicsObject.hitbox.back().halfSize = glm::vec3(0.0f, 0.0f, 0.0f);
        loader.load(objects.back().ModelPath.c_str(), false, "Pak1");
        objects.back().meshes = loader.meshes;
        items.push_back(objects.back().Name);

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

        std::ofstream out(getCurrentScenePath());
        out << scene.dump(4);
        out.close();
    };
    ImGui::End();

    ImGui::Begin("Object");

    if (objects.size() > 0)
    {
        if (currentItem >= static_cast<int>(objects.size()))
        {
            currentItem = objects.size() - 1;
        }

        std::vector<const char*> comboItems;
        comboItems.reserve(items.size());

        for (const auto& name : items)
            comboItems.push_back(name.c_str());

        ImGui::Combo("Item", &currentItem, comboItems.data(), comboItems.size());

        comboItems.clear();

        ImGui::InputText("Name", &objects[currentItem].Name);
        ImGui::InputText("Archive", &objects[currentItem].archive);

        items[currentItem] = objects[currentItem].Name;

        float pos[3];
        pos[0] = objects[currentItem].Position.x;
        pos[1] = objects[currentItem].Position.y;
        pos[2] = objects[currentItem].Position.z;
        ImGui::DragFloat3("Position", pos, 0.1f);
        objects[currentItem].Position = glm::vec3(pos[0], pos[1], pos[2]);

        float rot[3];
        rot[0] = objects[currentItem].Rotation.x;
        rot[1] = objects[currentItem].Rotation.y;
        rot[2] = objects[currentItem].Rotation.z;
        ImGui::DragFloat3("Rotation", rot, 0.1f);
        objects[currentItem].Rotation = glm::vec3(rot[0], rot[1], rot[2]);

        for (int i = 0; i < objects[currentItem].physicsObject.hitbox.size(); i++)
        {
            comboItems.push_back(objects[currentItem].physicsObject.hitbox[i].ID.c_str());
        }

        ImGui::Combo("Hitbox", &currentHitbox, comboItems.data(), comboItems.size());

        float halfsize[3];
        halfsize[0] = objects[currentItem].physicsObject.hitbox[currentHitbox].halfSize.x;
        halfsize[1] = objects[currentItem].physicsObject.hitbox[currentHitbox].halfSize.y;
        halfsize[2] = objects[currentItem].physicsObject.hitbox[currentHitbox].halfSize.z;
        ImGui::DragFloat3("Half Size", halfsize, 0.1f);
        objects[currentItem].physicsObject.hitbox[currentHitbox].halfSize =
            glm::vec3(halfsize[0], halfsize[1], halfsize[2]);

        float centre[3];
        centre[0] = objects[currentItem].physicsObject.hitbox[currentHitbox].centre.x;
        centre[1] = objects[currentItem].physicsObject.hitbox[currentHitbox].centre.y;
        centre[2] = objects[currentItem].physicsObject.hitbox[currentHitbox].centre.z;
        ImGui::DragFloat3("Centre", centre, 0.1f);
        objects[currentItem].physicsObject.hitbox[currentHitbox].centre = glm::vec3(centre[0], centre[1], centre[2]);

        if (ImGui::Button("Create Hitbox"))
        {
            objects[currentItem].physicsObject.hitbox.push_back(Hitbox());
            objects[currentItem].physicsObject.hitbox.back().centre = glm::vec3(0.0f, 0.0f, 0.0f);
            objects[currentItem].physicsObject.hitbox.back().halfSize = glm::vec3(0.0f, 0.0f, 0.0f);
            objects[currentItem].physicsObject.hitbox.back().ID =
                std::to_string(objects[currentItem].physicsObject.hitbox.size() - 1);
        }

        hitbox.physicsObject.hitbox.clear();
        for (int i = 0; i < objects[currentItem].physicsObject.hitbox.size(); i++)
        {
            hitbox.physicsObject.hitbox.push_back(Hitbox());
            hitbox.physicsObject.hitbox.back().halfSize = objects[currentItem].physicsObject.hitbox[i].halfSize;
            hitbox.physicsObject.hitbox.back().centre = objects[currentItem].physicsObject.hitbox[i].centre;
        }
        hitbox.Position = objects[currentItem].Position;

        if (ImGui::Button("Delete Object"))
        {
            scene["Objects"].erase(scene["Objects"].begin() + currentItem);
            objects.erase(objects.begin() + currentItem);
            items.erase(items.begin() + currentItem);

            if (objects.size() > 0)
            {
                currentItem = std::min(currentItem, static_cast<int>(objects.size()) - 1);
            }
            else
            {
                currentItem = 0;
            }
        }

        if (ImGui::Button("Duplicate Object"))
        {
            Object newObject = objects[currentItem];
            newObject.Name += " Copy";
            objects.push_back(newObject);
            items.push_back(newObject.Name);

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
        }
    }
    else
    {
        ImGui::Text("No objects loaded");
    }

    ImGui::Checkbox("Gravity", &objects[currentItem].physicsEnabled);
    ImGui::InputFloat("Mass", &objects[currentItem].physicsObject.mass);

    ImGui::Checkbox("Jumpad Enabled", &objects[currentItem].jumpPad.enabled);
    ImGui::SliderFloat3("Direction", objects[currentItem].jumpPad.direction, -1.0f, 1.0f);
    ImGui::DragFloat("Force", &objects[currentItem].jumpPad.force);

    ImGui::End();

    ImGui::Begin("Scene");

    ImGui::InputText("File Path", &ScenePath);
    if (ImGui::Button("Load"))
    {
        objects.clear();
        items.clear();
        currentItem = 0;
        std::vector<Object> newObjects = loader.loadScene(ScenePath.c_str(), NewScene, false);
        scene = NewScene;
        CurrentScenePath = ScenePath;
        for (size_t i = 0; i < newObjects.size(); i++)
        {
            objects.push_back(newObjects[i]);
            items.push_back(newObjects[i].Name);
        }
    }

    ImGui::End();

    ImGui::Begin("Lights");

    std::vector<const char*> comboItems;
    comboItems.reserve(lights.size());

    for (const auto& light : lights)
        comboItems.push_back(light.name.c_str());

    ImGui::Combo("Item", &currentLight, comboItems.data(), comboItems.size());

    ImGui::InputText("Name", &lights[currentLight].name);

    float pos[3];
    pos[0] = lights[currentLight].position.x;
    pos[1] = lights[currentLight].position.y;
    pos[2] = lights[currentLight].position.z;
    ImGui::DragFloat3("Position", pos, 0.1f);
    lights[currentLight].position = glm::vec3(pos[0], pos[1], pos[2]);

    float col[3];
    col[0] = lights[currentLight].colour.r;
    col[1] = lights[currentLight].colour.g;
    col[2] = lights[currentLight].colour.b;
    ImGui::DragFloat3("Colour", col, 0.1f, 0.0f, 1.0f);
    lights[currentLight].colour = glm::vec3(col[0], col[1], col[2]);

    ImGui::DragFloat("Intensity", &lights[currentLight].intensity, 0.1f, 0.0f);

    if (ImGui::Button("Create Light"))
    {
        lights.push_back(Light());
        lights.back().colour = glm::vec3(1.0f, 1.0f, 1.0f);
        lights.back().intensity = 1.0f;
        lights.back().name = "NewLight";
        lights.back().position = glm::vec3(0.0f, 0.0f, 0.0f);

        for (unsigned long i = 0; i < lights.size(); i++)
        {
            scene["Lights"][i]["Colour"][0] = lights[i].colour.r;
            scene["Lights"][i]["Colour"][1] = lights[i].colour.g;
            scene["Lights"][i]["Colour"][2] = lights[i].colour.b;
            scene["Lights"][i]["Intensity"] = lights[i].intensity;
            scene["Lights"][i]["Name"] = lights[i].name;
            scene["Lights"][i]["Position"][0] = lights[i].position.x;
            scene["Lights"][i]["Position"][1] = lights[i].position.y;
            scene["Lights"][i]["Position"][2] = lights[i].position.z;
        }
    }

    ImGui::End();
}

void Gui::RenderGui()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Gui::guiGame(Camera camera)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Camera");

    float vel = fabs(camera.Velocity.x) + fabs(camera.Velocity.z);
    std::string buf = std::to_string(vel);
    ImGui::Text(buf.c_str());

    ImGui::End();
}