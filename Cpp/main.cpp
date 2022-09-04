#include "MonoUtils.hpp"
static std::vector<CsTypeInfo> s_TypeInfos = std::vector<CsTypeInfo>();

static std::string s_DomainName = "Test";
const std::string s_AppDomainName = "TestApp";
const std::string s_CsClassName = "TestLib";
static MonoDomain* s_RootDomain;
static MonoDomain* s_AppDomain;
static MonoAssembly* s_AppAssembly;



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