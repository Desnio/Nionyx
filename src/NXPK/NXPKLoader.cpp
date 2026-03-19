#include "NXPK.hpp"

off_t size;
namespace fs = std::filesystem;

#if defined(_WIN32)

#include <windows.h>

const char* mapFile(std::string pathtemp)
{
    const char* path = pathtemp.c_str();
    HANDLE file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    LARGE_INTEGER fileSize;
    GetFileSizeEx(file, &fileSize);
    size = static_cast<size_t>(fileSize.QuadPart);

    HANDLE mapping = CreateFileMappingA(file, NULL, PAGE_READONLY, 0, 0, NULL);

    void* datatemp = MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);

    const char* data = static_cast<const char*>(datatemp);

    CloseHandle(mapping);
    CloseHandle(file);

    return data;
}

#elif defined(__APPLE__) || defined(__linux__)

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

const char* mapFile(std::string archivePath)
{
    int fd = open(archivePath.c_str(), O_RDONLY);
    if(fd < 0) {
        std::cerr << "Failed to open archive\n";
        return {};
    }

    size = lseek(fd, 0, SEEK_END);
    const char* archiveData = (const char*) mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if(archiveData == MAP_FAILED) {
        close(fd);
        std::cerr << "Failed to mmap archive\n" << fd << std::endl << size << std::endl << archivePath;
        return {};
    }

    return archiveData;
}

#endif

/*
This loads a single file from a .nxpk archive and returns the char vector
The archive path is relative to the root folder and the file path is relative to the archive.
E.g. if there is a file in Nionyx/pak/assets/cube.glb where pak.nxpk is the packed version of pak/ and is Nionyx/pak.nxpk. The file path will be assets/cube.glb and the archive path with be pak.nxpk
*/

std::vector<char> NXPKLoader::LoadFromArchive(std::string archivePath, std::string filePath, std::string type)
{
    int sizetemp = archivePath.size();
    if(archivePath[sizetemp - 5] != '.')
    {
        archivePath += ".nxpk";
    }
    sizetemp = filePath.size();
    if(filePath[sizetemp - 4] != '.' && filePath[sizetemp - 3] != '.' && filePath[sizetemp - 5] != '.')
    {
        filePath += type;
    }

    const char* archiveData = mapFile(archivePath);

    // read header from the end
    const char* headerPtr = archiveData + size - 20;
    char magic[4];
    uint32_t version;
    uint32_t fileCount;
    uint64_t directoryOffset;

    std::memcpy(magic, headerPtr, 4);
    std::memcpy(&version, headerPtr + 4, 4);
    std::memcpy(&fileCount, headerPtr + 8, 4);
    std::memcpy(&directoryOffset, headerPtr + 12, 8);

    uint32_t Offset = 0.0f;

    for(int i = 0; i < file.size(); i++)
    {
        if(file[i].path == filePath)
        {
            std::vector<char> data(file[i].size);

            std::memcpy(data.data(), archiveData + file[i].offset, file[i].size);

            std::vector<char> decompData(file[i].OriginalSize);

            int result = LZ4_decompress_safe(
                data.data(),
                decompData.data(),
                static_cast<int>(data.size()),
                file[i].OriginalSize
            );

            return decompData;
        }
    }
    
    for (uint32_t i = 0; i < fileCount; i++)
    {
        uint16_t nameLength;
        std::memcpy(&nameLength, archiveData + directoryOffset + Offset, sizeof(nameLength));
        Offset += sizeof(nameLength);

        std::string pathTemp;
        pathTemp.resize(nameLength);
        std::memcpy(pathTemp.data(), archiveData + directoryOffset + Offset, pathTemp.size());
        Offset += pathTemp.size();

        FileEntry entry;
        entry.path = pathTemp;

        std::memcpy(&entry.offset, archiveData + directoryOffset + Offset, sizeof(entry.offset));
        Offset += sizeof(entry.offset);
        std::memcpy(&entry.size, archiveData + directoryOffset + Offset, sizeof(entry.size));
        Offset += sizeof(entry.size);
        std::memcpy(&entry.OriginalSize, archiveData + directoryOffset + Offset, sizeof(entry.OriginalSize));
        Offset += sizeof(entry.OriginalSize);

        if (entry.path == filePath)
        {
            std::vector<char> data(entry.size);

            std::memcpy(data.data(), archiveData + entry.offset, entry.size);

            std::vector<char> decompData(entry.OriginalSize);

            int result = LZ4_decompress_safe(
                data.data(),
                decompData.data(),
                static_cast<int>(data.size()),
                entry.OriginalSize
            );

            return decompData;
        }

        file.push_back(entry);
    }

    return {};
}
