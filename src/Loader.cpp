#define STB_IMAGE_IMPLEMENTATION
#include "Loader.hpp"
#include <stb_image.h>

NXPKLoader nxpkLoader;

void Loader::load(const char* path, bool archived, std::string archive)
{
    meshes.clear();
    Assimp::Importer importer;

    const unsigned int flags = aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
                               aiProcess_ImproveCacheLocality | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph |
                               aiProcess_CalcTangentSpace | aiProcess_GenNormals | aiProcess_RemoveRedundantMaterials |
                               aiProcess_FlipUVs;

    if (archived)
    {
        std::string archivePath = archive + ".nxpk";

        std::vector<char> data;

        {
            ZoneScopedN("Load from storage");
            data = nxpkLoader.LoadFromArchive(archivePath, path, ".glb");
        }

        // check if it actually loaded
        if (data.empty())
        {
            std::cerr << "NXPK: failed to load \"" << path << "\" from archive\n";
            return;
        }

        const aiScene* scene;

        {
            ZoneScopedN("Load Assimp");
            scene = importer.ReadFileFromMemory(data.data(), data.size(), flags, "glb");
        }

        // check if it is actually a valid file
        if (!scene)
        {
            std::cerr << "Assimp failed to read from memory: " << importer.GetErrorString() << "\n";
            return;
        }

        {
            ZoneScopedN("ProcessNode");
            processNode(scene->mRootNode, scene);
        }
    }
    else
    {
        const aiScene* scene = importer.ReadFile(path, flags);

        // check if its a valid file
        if (!scene)
        {
            std::cerr << "Assimp failed to read file \"" << path << "\": " << importer.GetErrorString() << "\n";
            return;
        }

        processNode(scene->mRootNode, scene);
    }
}

void Loader::processNode(aiNode* node, const aiScene* scene)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
        processNode(node->mChildren[i], scene);
}

Mesh Loader::processMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    unsigned int textureID = 0;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        vertices.push_back(mesh->mVertices[i].x);
        vertices.push_back(mesh->mVertices[i].y);
        vertices.push_back(mesh->mVertices[i].z);

        vertices.push_back(mesh->mNormals[i].x);
        vertices.push_back(mesh->mNormals[i].y);
        vertices.push_back(mesh->mNormals[i].z);

        if (mesh->mTextureCoords[0])
        {
            vertices.push_back(mesh->mTextureCoords[0][i].x);
            vertices.push_back(mesh->mTextureCoords[0][i].y);
        }
        else
        {
            vertices.push_back(0.0f);
            vertices.push_back(0.0f);
        }
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        {
            aiString str;
            material->GetTexture(aiTextureType_DIFFUSE, 0, &str);

            const aiTexture* tex = scene->GetEmbeddedTexture(str.C_Str());

            if (tex)
            {
                textureID = loadEmbeddedTexture(tex);
            }
            else
            {
                std::string path = "assets/" + std::string(str.C_Str());
                textureID = loadTextureFromFile(path.c_str());
            }
        }
    }

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) (6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    Mesh result;
    result.VAO = VAO;
    result.VBO = VBO;
    result.EBO = EBO;
    result.numIndices = indices.size();
    result.textureID = textureID;

    return result;
}

unsigned int Loader::loadTextureFromFile(const char* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

    if (data)
    {
        GLenum format = GL_RGB;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Failed to load texture "
                     "at path: "
                  << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

unsigned int Loader::loadEmbeddedTexture(const aiTexture* tex)
{
    if (embeddedTextureCache.count(tex))
        return embeddedTextureCache[tex];

    unsigned int id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    int w, h, c;
    unsigned char* data = stbi_load_from_memory((unsigned char*) tex->pcData, tex->mWidth, &w, &h, &c, 0);

    if (!data)
    {
        std::cout << "Failed to decode "
                     "embedded texture\n";
        return 0;
    }

    GLenum format = (c == 4) ? GL_RGBA : GL_RGB;

    glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    embeddedTextureCache[tex] = id;
    return id;
}

std::vector<Object> Loader::loadScene(const char* path, nlohmann::json& scene, bool archive)
{
    objects.clear();

    if (archive)
    {
        NXPKLoader loader;
        auto data = loader.LoadFromArchive("Saves", path, ".json");

        scene = nlohmann::json::parse(data.begin(), data.end());
    }
    else
    {
        std::ifstream sceneFile(path);

        if (!sceneFile.is_open())
        {
            std::cerr << "Failed to open scene file: " << path << std::endl;
            return objects;
        }

        try
        {
            sceneFile >> scene;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Failed to parse JSON: " << e.what() << std::endl;
            return objects;
        }
        sceneFile.close();
    }

    auto& obj = scene["Objects"];
    int objectsCount = obj.size();

    for (int i = 0; i < objectsCount; i++)
    {
        if (!obj[i].contains("Name") || !obj[i].contains("File Path") || !obj[i].contains("Position") ||
            !obj[i].contains("Rotation"))
        {
            std::cerr << "Skipping object " << i << ": missing required fields" << std::endl;
            continue;
        }

        meshes.clear();
        embeddedTextureCache.clear();
        objects.push_back(Object());
        objects.back().Name = obj[i]["Name"].get<std::string>();
        objects.back().ModelPath = obj[i]["File Path"].get<std::string>();
        objects.back().Position = glm::vec3(obj[i]["Position"][0].get<float>(), obj[i]["Position"][1].get<float>(),
                                            obj[i]["Position"][2].get<float>());
        objects.back().Rotation = glm::vec3(obj[i]["Rotation"][0].get<float>(), obj[i]["Rotation"][1].get<float>(),
                                            obj[i]["Rotation"][2].get<float>());
        objects.back().physicsEnabled = obj[i]["Gravity"].get<bool>();
        objects.back().physicsObject.mass = obj[i]["Mass"].get<float>();
        objects.back().physicsObject.velocity = glm::vec3(0.0f, 0.0f, 0.0f);
        for (int o = 0; o < obj[i]["Hitbox"].size(); o++)
        {
            objects.back().physicsObject.hitbox.push_back(Hitbox());
            objects.back().physicsObject.hitbox.back().halfSize = glm::vec3(
                obj[i]["Hitbox"][o]["HalfPos"][0].get<float>(), obj[i]["Hitbox"][o]["HalfPos"][1].get<float>(),
                obj[i]["Hitbox"][o]["HalfPos"][2].get<float>());
            objects.back().physicsObject.hitbox.back().centre =
                glm::vec3(obj[i]["Hitbox"][o]["Centre"][0].get<float>(), obj[i]["Hitbox"][o]["Centre"][1].get<float>(),
                          obj[i]["Hitbox"][o]["Centre"][2].get<float>());
            objects.back().physicsObject.hitbox.back().ID = std::to_string(o);
        }
        objects.back().archive = obj[i]["Archive"].get<std::string>();
        objects.back().jumpPad.enabled = obj[i]["JumpPad"]["Enabled"];
        objects.back().jumpPad.direction[0] = obj[i]["JumpPad"]["Direction"][0];
        objects.back().jumpPad.direction[1] = obj[i]["JumpPad"]["Direction"][1];
        objects.back().jumpPad.direction[2] = obj[i]["JumpPad"]["Direction"][2];

        objects.back().jumpPad.force = obj[i]["JumpPad"]["Force"];

        cached = false;
        for (int i = 0; i < cache.size(); i++)
        {
            if (cache[i] == objects.back().ModelPath)
            {
                objects.back().meshes = objects[i].meshes;
                cache.push_back("");
                cached = true;
            }
        }

        if (cached == false)
        {
            load(objects.back().ModelPath.c_str(), archive, objects.back().archive);
            objects.back().meshes = meshes;
            cache.push_back(objects.back().ModelPath.c_str());
        }
    }

    auto& light = scene["Lights"];
    int lightCount = light.size();
    for (int i = 0; i < lightCount; i++)
    {
        lights.push_back(Light());
        lights.back().colour = glm::vec3(light[i]["Colour"][0].get<float>(), light[i]["Colour"][1].get<float>(),
                                         light[i]["Colour"][2].get<float>());
        lights.back().intensity = light[i]["Intensity"].get<float>();
        lights.back().name = light[i]["Name"];
        lights.back().position = glm::vec3(light[i]["Position"][0].get<float>(), light[i]["Position"][1].get<float>(),
                                           light[i]["Position"][2].get<float>());
    }

    return objects;
}

Object Loader::loadHitbox()
{
    Object hitbox;
    hitbox.Name = "Hitbox";
    hitbox.ModelPath = "";
    hitbox.Position = glm::vec3(0.0f);
    hitbox.Rotation = glm::vec3(0.0f);
    hitbox.physicsEnabled = false;
    hitbox.physicsObject.mass = 0.0f;
    hitbox.physicsObject.velocity = glm::vec3(0.0f);
    hitbox.physicsObject.hitbox.push_back(Hitbox());
    hitbox.physicsObject.hitbox.back().halfSize = glm::vec3(1.0f);
    hitbox.physicsObject.hitbox.back().centre = glm::vec3(0.0f);

    float vertices[] = {-0.5, -0.5, -0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, -0.5,
                        -0.5, -0.5, 0.5,  0.5, -0.5, 0.5,  0.5, 0.5, 0.5,  -0.5, 0.5, 0.5};

    unsigned int indices[] = {0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7};

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    hitbox.meshes.push_back(Mesh{VAO, VBO, EBO, 24, 0});

    return hitbox;
}

void Loader::loadLight(Object& Light)
{
    Light.ModelPath = "EngineAssets/Sphere.glb";
    Light.Name = "Light";
    Light.physicsEnabled = false;
    Light.Position = glm::vec3(0.0f, 0.0f, 0.0f);
    Light.Rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    meshes.clear();
    load("EngineAssets/Sphere.glb", false, "");
    Light.meshes = meshes;
}
