#include "MonoUtils.hpp"

static std::vector<CsTypeInfo> s_TypeInfos = std::vector<CsTypeInfo>();
static std::string s_DomainName     = "Test";
const std::string s_AppDomainName   = "TestApp";
const std::string s_CsClassName     = "TestLib";

static MonoDomain*      s_RootDomain;
static MonoDomain*      s_AppDomain;
static MonoAssembly*    s_AppAssembly;




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

void CallIncrementFloatVar(MonoObject* objectInstance, float value)
{
    MonoClass* instanceClass = mono_object_get_class(objectInstance);

    MonoMethod* method = mono_class_get_method_from_name(instanceClass, "IncrementFloatVar", 1);

    if (method == nullptr)
    {
        std::cerr << "Failed to find method IncrementFloatVar\n";
        return;
    }

    // Call the C# method on the objectInstance instance, and get any potential exceptions
    MonoObject* exception = nullptr;
    void* args[] =
    {
        &value
    };
    mono_runtime_invoke(method, objectInstance, args, &exception);
}

void InitMono()
{
    std::string root(std::getenv("MONO_DIR"));
    std::string assemblyDir = root + "/lib/mono/4.5";
    mono_set_assemblies_path(assemblyDir.c_str());

    s_RootDomain = mono_jit_init(s_DomainName.c_str());
    if (s_RootDomain == nullptr)
    {
        std::cerr << "Failed to initialize MonoDomain : " << s_DomainName;
        return;
    }

    s_AppDomain = mono_domain_create_appdomain((char*)s_AppDomainName.c_str(), nullptr);
    if (s_AppDomain == nullptr)
    {
        std::cerr << "Failed to initialize AppDomain : " << s_AppDomainName;
        return;
    }

    mono_domain_set(s_AppDomain, true);
    MonoAssembly* s_AppAssembly = LoadCSharpAssembly("TestLib.dll");

    if (s_AppAssembly == nullptr)
    {
        return;
    }

    PrintAssemblyInfo(s_AppAssembly);
}

void Playground()
{

    MonoObject* classInstance = CreateClassInstance("", s_CsClassName.c_str());

    CallPrintFloatVarMethod(classInstance);
    CallIncrementFloatVar(classInstance, 3.0f);
    CallPrintFloatVarMethod(classInstance);

    std::cout << "Finished" << std::endl;

    MonoClass* testingClass = mono_object_get_class(classInstance);

    // Get a reference to the public field called "MyPublicFloatVar"
    MonoClassField* floatField = mono_class_get_field_from_name(testingClass, "PublicFloat");
    uint8_t floatFieldAccessibility = GetFieldAccessibility(floatField);

    if (floatFieldAccessibility & (uint8_t)Accessibility::Public)
    {
        // We can safely write a value to this
    }

    // Get a reference to the private field called "m_Name"
    MonoClassField* nameField = mono_class_get_field_from_name(testingClass, "_name");
    uint8_t nameFieldAccessibility = GetFieldAccessibility(nameField);

    if (nameFieldAccessibility & (uint8_t)Accessibility::Private)
    {
        // We shouldn't write to this field
    }

    // Get a reference to the public property called "Name"
    MonoProperty* nameProperty = mono_class_get_property_from_name(testingClass, "Name");
    uint8_t namePropertyAccessibility = GetPropertyAccessbility(nameProperty);

    if (namePropertyAccessibility & (uint8_t)Accessibility::Public)
    {
        // We can safely write a value to the field using this property
    }

}

int main()
{
    std::cout << "Mono Test Program" << std::endl;
    InitMono();
    Playground();
}