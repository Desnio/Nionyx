#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <iostream>

#include "lz4.h"
#include "lz4hc.h"

namespace fs = std::filesystem;

struct FileEntry
{
    std::string path;
    uint64_t offset;
    uint32_t size;
    uint32_t originalSize;
};

std::vector<char> readFile(const fs::path& path)
{
    std::ifstream file(path, std::ios::binary);

    file.seekg(0, std::ios::end);
    std::size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    file.read(buffer.data(), size);

    return buffer;
}

std::vector<char> compressLZ4(const std::vector<char>& input)
{
    int maxCompressedSize = LZ4_compressBound(static_cast<int>(input.size()));

    std::vector<char> compressed(maxCompressedSize);

    int compressedSize = LZ4_compress_HC(
        input.data(),
        compressed.data(),
        static_cast<int>(input.size()),
        maxCompressedSize,
        LZ4HC_CLEVEL_MAX
    );

    if (compressedSize <= 0)
        throw std::runtime_error("LZ4 compression failed");

    compressed.resize(compressedSize);
    return compressed;
}


void packFolder(const fs::path& folderPath, const std::string& outputFile)
{
    std::ofstream out(outputFile, std::ios::binary);

    if (!out)
    {
        std::cout << "Failed to create archive\n";
        return;
    }

    std::vector<FileEntry> entries;
    
    // Writes raw file data and stores the sizes and offset
    for (auto& entry : fs::recursive_directory_iterator(folderPath))
    {
        if (!entry.is_regular_file() || entry.path().filename() == ".DS_Store")
            continue;

        fs::path relative = fs::relative(entry.path(), folderPath);
        std::string relativeStr = folderPath.generic_string() + "/" + relative.generic_string();

        auto dataTemp = readFile(entry.path());

        auto data = compressLZ4(dataTemp);

        uint64_t offset = out.tellp();
        out.write(data.data(), data.size());

        entries.push_back({
            relativeStr,
            offset,
            static_cast<uint32_t>(data.size()),
            static_cast<uint32_t>(dataTemp.size())
        });

        std::cout << "Packed: " << relativeStr << "\n";
    }

    uint64_t directoryOffset = out.tellp();

    // Writes the directory which has all the data for each file
    // stores the file path, offset and size aswell as the original file size for decompression
    for (const auto& entry : entries)
    {
        uint16_t nameLength = entry.path.size();
        out.write(reinterpret_cast<const char*>(&nameLength), 2);
        out.write(entry.path.data(), nameLength);

        out.write(reinterpret_cast<const char*>(&entry.offset), sizeof(entry.offset));
        out.write(reinterpret_cast<const char*>(&entry.size), sizeof(entry.size));
        out.write(reinterpret_cast<const char*>(&entry.originalSize), sizeof(entry.originalSize));
    }

    // Writes the header now that all the info on offsets and sizes are known
    char magic[4] = { 'N','X','P','K' };
    uint32_t version = 1;
    uint32_t fileCount = entries.size();

    out.write(magic, 4);
    out.write(reinterpret_cast<char*>(&version), sizeof(version));
    out.write(reinterpret_cast<char*>(&fileCount), sizeof(fileCount));
    out.write(reinterpret_cast<char*>(&directoryOffset), sizeof(directoryOffset));

    std::cout << "Archive complete.\n";
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cout << "Usage: ./NXPKPacker <input_folder> <output_archive>\n";
        return 1;
    }

    packFolder(argv[1], argv[2]);
}