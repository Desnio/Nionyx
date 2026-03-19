#pragma once

#include "pch.hpp"

struct FileEntry
{
    std::string path;
    uint64_t offset;
    uint32_t size;
    uint32_t OriginalSize;
};

class NXPKLoader
{
public:
    std::vector<char> LoadFromArchive(std::string archivePath, std::string filePath, std::string type);
private:
    std::vector<FileEntry> file;
};