#pragma once

#include "json.hpp"
#include "pch.hpp"
#include "NXPK.hpp"

class Loader
{
    public:
        void load(const char* path, bool archived, std::string archive);
        std::vector<unsigned int> indices;
        std::vector<Mesh> meshes;
        std::vector<Object> loadScene(const char* path, nlohmann::json& scene, bool archive);
        Object loadHitbox();
        std::vector<Light> lights;
        void loadLight(Object &Light);
    private:
        std::unordered_map<const aiTexture*, unsigned int> embeddedTextureCache;
        void processNode(aiNode* node, const aiScene* scene);
        Mesh processMesh(aiMesh* mesh, const aiScene* scene);
        unsigned int loadTextureFromFile(const char* path);
        unsigned int loadEmbeddedTexture(const aiTexture* tex);
        std::vector<Object> objects;
        bool cached = false;
        std::vector<std::string> cache;
};