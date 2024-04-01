#pragma once

#include "FileSystem.h"

#include <filesystem>
#include <regex>

namespace fs = std::filesystem;

std::vector<char> FileSystem::ReadFile(const std::string& FileName)
{
    std::ifstream File(FileName, std::ios::ate | std::ios::binary);

    if (!File.is_open())
    {
        throw std::runtime_error("FS > Failed to open file: " + FileName);
    }
    else
    {
        std::cout << "FS > Successfully opened file: " + FileName + "\n";
    }

    size_t FileSize = (size_t)File.tellg();
    std::vector<char> Buffer(FileSize);

    File.seekg(0);
    File.read(Buffer.data(), FileSize);

    File.close();

    return Buffer;
}

void FileSystem::LoadTextures()
{
    int Count = 0;

    for (auto& p : std::filesystem::recursive_directory_iterator("Assets/Textures"))
    {
        Count += 1;
    }

    std::cout << "Found " << Count << " files" << std::endl;
}