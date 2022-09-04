#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"

struct CsTypeInfo
{
    std::string m_TypeName;
    std::string m_TypeNamespace;
};

static std::vector<CsTypeInfo> s_TypeInfos = std::vector<CsTypeInfo>();

static std::string s_DomainName = "Test";
const std::string s_AppDomainName = "TestApp";
const std::string s_CsClassName = "TestLib";
static MonoDomain* s_RootDomain;
static MonoDomain* s_AppDomain;
static MonoAssembly* s_AppAssembly;
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

void PrintAssemblyInfo(MonoAssembly* assembly)
{
    MonoImage* image = mono_assembly_get_image(assembly);
    const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
    int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

    for (int32_t i = 0; i < numTypes; i++)
    {
        uint32_t cols[MONO_TYPEDEF_SIZE];
        mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

        std::string nameSpace = std::string(mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]));
        std::string name = std::string(mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]));

        std::cout << "Type Name : " << name << "\n" << "Type Namespace : " << nameSpace << "\n";

        CsTypeInfo typeInfo{
            name,
            nameSpace
        };

        s_TypeInfos.emplace_back(typeInfo);
    }
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

MonoObject* CreateClassInstance(const char* nameSpace, const char* className)
{
    // Get a reference to the class we want to instantiate
    MonoClass* testingClass = GetClassInAssembly(s_AppAssembly, "", s_CsClassName.c_str());

    // Allocate an instance of our class
    MonoObject* classInstance = mono_object_new(s_AppDomain, testingClass);

    if (classInstance == nullptr)
    {
        // Log error here and abort
        std::cerr << "Failed to Create instance of class : " << nameSpace << "." << className;
    }

    // Call the parameterless (default) constructor
    mono_runtime_object_init(classInstance);

    return classInstance;
}

void CallPrintFloatVarMethod(MonoObject* objectInstance)
{
    MonoClass* instanceClass = mono_object_get_class(objectInstance);

    MonoMethod* method = mono_class_get_method_from_name(instanceClass, "PrintFloatVar", 0);

    if (method == nullptr)
    {
        std::cerr << "Failed to find method PrintFloatVar\n";
        return;
    }

    // Call the C# method on the objectInstance instance, and get any potential exceptions
    MonoObject* exception = nullptr;
    mono_runtime_invoke(method, objectInstance, nullptr, &exception);

}

void InitMono()
{
    std::string root(std::getenv("MONO_DIR"));
    std::string assemblyDir = root + "/lib/mono/4.5";
    mono_set_assemblies_path(assemblyDir.c_str());

    MonoDomain* rootDomain = mono_jit_init(s_DomainName.c_str());
    if (rootDomain == nullptr)
    {
        std::cerr << "Failed to initialize MonoDomain : " << s_DomainName;
    }

    s_RootDomain = rootDomain;

    MonoDomain* appDomain = mono_domain_create_appdomain((char*)s_AppDomainName.c_str(), nullptr);
    if (appDomain == nullptr)
    {
        std::cerr << "Failed to initialize AppDomain : " << s_AppDomainName;
    }

    s_AppDomain = appDomain;

    mono_domain_set(s_AppDomain, true);

    std::string currentPath = std::filesystem::current_path().string();

    MonoAssembly* testAssembly = LoadCSharpAssembly("TestLib.dll");

    if (testAssembly == nullptr)
    {
        return;
    }

    s_AppAssembly = testAssembly;

    PrintAssemblyInfo(s_AppAssembly);
}

void Playground()
{

    MonoObject* classInstance = CreateClassInstance("", s_CsClassName.c_str());

    CallPrintFloatVarMethod(classInstance);
    std::cout << "Finished" << std::endl;
}

int main()
{
    std::cout << "Mono Test Program" << std::endl;
    InitMono();
    Playground();
}