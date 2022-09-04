#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/attrdefs.h"

enum Accessibility : uint8_t
{
    None = 0,
    Private = (1 << 0),
    Internal = (1 << 1),
    Protected = (1 << 2),
    Public = (1 << 3)
};

struct CsTypeInfo
{
    std::string m_TypeName;
    std::string m_TypeNamespace;
};

char* ReadBytes(const std::string& filepath, uint32_t* outSize)
{
    std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

    if (!stream)
    {
        std::cerr << "Failed to open file at path : " << filepath << std::endl;
        return nullptr;
    }

    std::streampos end = stream.tellg();
    stream.seekg(0, std::ios::beg);
    uint32_t size = end - stream.tellg();

    if (size == 0)
    {
        std::cerr << "Loadaded assembly is empty! : " << filepath << std::endl;
        return nullptr;
    }

    char* buffer = new char[size];
    stream.read((char*)buffer, size);
    stream.close();

    *outSize = size;
    return buffer;
}