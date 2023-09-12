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

struct CsInterfaceImplInfo
{
    std::string m_InterfaceName;
    std::string m_ClassName;
};

struct CsMethodImplInfo
{
    std::string m_Declaration;
    std::string m_Body;
    std::string m_ClassName;
};

struct CsTypeRefInfo
{
    std::string m_Name;
    std::string m_Namespace;
    std::string m_Scope;
};

struct CsAssemblyRefInfo
{
    std::string m_Name;
    uint32_t    m_Major;
    uint32_t    m_Minor;
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


// Gets the accessibility level of the given field
uint8_t GetFieldAccessibility(MonoClassField* field)
{
    uint8_t accessibility = (uint8_t)Accessibility::None;
    uint32_t accessFlag = mono_field_get_flags(field) & MONO_FIELD_ATTR_FIELD_ACCESS_MASK;

    switch (accessFlag)
    {
    case MONO_FIELD_ATTR_PRIVATE:
    {
        accessibility = (uint8_t)Accessibility::Private;
        break;
    }
    case MONO_FIELD_ATTR_FAM_AND_ASSEM:
    {
        accessibility |= (uint8_t)Accessibility::Protected;
        accessibility |= (uint8_t)Accessibility::Internal;
        break;
    }
    case MONO_FIELD_ATTR_ASSEMBLY:
    {
        accessibility = (uint8_t)Accessibility::Internal;
        break;
    }
    case MONO_FIELD_ATTR_FAMILY:
    {
        accessibility = (uint8_t)Accessibility::Protected;
        break;
    }
    case MONO_FIELD_ATTR_FAM_OR_ASSEM:
    {
        accessibility |= (uint8_t)Accessibility::Private;
        accessibility |= (uint8_t)Accessibility::Protected;
        break;
    }
    case MONO_FIELD_ATTR_PUBLIC:
    {
        accessibility = (uint8_t)Accessibility::Public;
        break;
    }

    }
    return accessibility;
}

// Gets the accessibility level of the given property
uint8_t GetPropertyAccessbility(MonoProperty* property)
{
    uint8_t accessibility = (uint8_t)Accessibility::None;

    // Get a reference to the property's getter method
    MonoMethod* propertyGetter = mono_property_get_get_method(property);
    if (propertyGetter != nullptr)
    {
        // Extract the access flags from the getters flags
        uint32_t accessFlag = mono_method_get_flags(propertyGetter, nullptr) & MONO_METHOD_ATTR_ACCESS_MASK;

        switch (accessFlag)
        {
        case MONO_FIELD_ATTR_PRIVATE:
        {
            accessibility = (uint8_t)Accessibility::Private;
            break;
        }
        case MONO_FIELD_ATTR_FAM_AND_ASSEM:
        {
            accessibility |= (uint8_t)Accessibility::Protected;
            accessibility |= (uint8_t)Accessibility::Internal;
            break;
        }
        case MONO_FIELD_ATTR_ASSEMBLY:
        {
            accessibility = (uint8_t)Accessibility::Internal;
            break;
        }
        case MONO_FIELD_ATTR_FAMILY:
        {
            accessibility = (uint8_t)Accessibility::Protected;
            break;
        }
        case MONO_FIELD_ATTR_FAM_OR_ASSEM:
        {
            accessibility |= (uint8_t)Accessibility::Private;
            accessibility |= (uint8_t)Accessibility::Protected;
            break;
        }
        case MONO_FIELD_ATTR_PUBLIC:
        {
            accessibility = (uint8_t)Accessibility::Public;
            break;
        }
        }
    }

    // Get a reference to the property's setter method
    MonoMethod* propertySetter = mono_property_get_set_method(property);
    if (propertySetter != nullptr)
    {
        // Extract the access flags from the setters flags
        uint32_t accessFlag = mono_method_get_flags(propertySetter, nullptr) & MONO_METHOD_ATTR_ACCESS_MASK;
        if (accessFlag != MONO_FIELD_ATTR_PUBLIC)
            accessibility = (uint8_t)Accessibility::Private;
    }
    else
    {
        accessibility = (uint8_t)Accessibility::Private;
    }

    return accessibility;
}

MonoAssembly* LoadCSharpAssembly(const std::string& assemblyPath)
{
    uint32_t fileSize = 0;
    char* fileData = ReadBytes(assemblyPath, &fileSize);

    MonoImageOpenStatus status;
    MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

    if (status != MONO_IMAGE_OK)
    {
        const char* errorMessage = mono_image_strerror(status);
        std::cerr << "Failed to create MonoAssembly for assembly : " << errorMessage << std::endl;
        return nullptr;
    }

    MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.c_str(), &status, 0);
    mono_image_close(image);

    // Don't forget to free the file data
    delete[] fileData;

    return assembly;
}


MonoClass* GetClassInAssembly(MonoAssembly* assembly, const char* namespaceName, const char* className)
{
    MonoImage* image = mono_assembly_get_image(assembly);
    MonoClass* klass = mono_class_from_name(image, namespaceName, className);

    if (klass == nullptr)
    {
        // Log error here
        return nullptr;
    }

    return klass;
}