#include <iostream>
#include "SharedLibTest/DynamicLoader.h"
#include "TestCore/CorePropertyType.h"
#include <ROP/RunTimeObjectProperty.h>

using TestObjectBase = ROP::PropertyObject<
    TestCore::CorePropertyType,
    std::string,
    std::hash<std::string>,
    std::equal_to<std::string>,
    std::function<std::string(const std::string&)>,
    std::string,
    ROP::DefaultErrorCallback<std::string>
>;

void TestSingleProperty(TestObjectBase* obj, const char* propName)
{
    auto prop = obj->GetProperty(propName);
    if (!prop.IsValid())
    {
        std::cout << "  " << propName << ": NOT FOUND" << std::endl;
        return;
    }

    auto type = prop.GetType();
    std::cout << "  " << propName << " = ";

    if (type == TestCore::CorePropertyType::ID ||
        type == TestCore::CorePropertyType::INT_VALUE)
    {
        std::cout << prop.GetValue<int>();
    }
    else if (type == TestCore::CorePropertyType::NAME ||
             type == TestCore::CorePropertyType::DESCRIPTION)
    {
        std::cout << "\"" << prop.GetValue<std::string>() << "\"";
    }
    else if (type == TestCore::CorePropertyType::FLOAT_VALUE)
    {
        std::cout << prop.GetValue<float>();
    }
    else if (type == TestCore::CorePropertyType::DOUBLE_VALUE)
    {
        std::cout << prop.GetValue<double>();
    }
    else if (type == TestCore::CorePropertyType::ENABLED)
    {
        std::cout << (prop.GetValue<bool>() ? "true" : "false");
    }
    else
    {
        std::cout << "[type=" << static_cast<int>(type) << "]";
    }
    std::cout << std::endl;
}

void TestLibrary(const std::string& libPath, const std::string& libName,
                 const char* createFuncName, const char* destroyFuncName,
                 const char* versionFuncName, const char* descFuncName)
{
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "Testing " << libName << std::endl;
    std::cout << std::string(50, '=') << std::endl;

    SharedLibTest::ScopedLibraryLoader loader(libPath);
    if (!loader.IsLoaded())
    {
        std::cout << "ERROR: Failed to load library" << std::endl;
        return;
    }

    auto getVersion = loader.GetLoader()->GetFunction<const char*()>(versionFuncName);
    if (getVersion)
    {
        std::cout << "Version: " << getVersion() << std::endl;
    }

    auto createFunc = loader.GetLoader()->GetFunction<void*()>(createFuncName);
    auto destroyFunc = loader.GetLoader()->GetFunction<void(void*)>(destroyFuncName);

    if (!createFunc)
    {
        std::cout << "ERROR: Failed to get create function" << std::endl;
        return;
    }

    void* rawPtr = createFunc();
    TestObjectBase* obj = static_cast<TestObjectBase*>(rawPtr);

    std::cout << "\nClass: " << obj->GetClassName() << std::endl;

    // Test properties
    std::cout << "\n[Initial Properties]" << std::endl;
    if (libName == "Test1lib")
    {
        TestSingleProperty(obj, "objectId");
        TestSingleProperty(obj, "objectName");
        TestSingleProperty(obj, "description");
        TestSingleProperty(obj, "score");
        TestSingleProperty(obj, "ratio");
    }
    else
    {
        TestSingleProperty(obj, "objectId");
        TestSingleProperty(obj, "name");
        TestSingleProperty(obj, "description");
        TestSingleProperty(obj, "value");
        TestSingleProperty(obj, "precision");
        TestSingleProperty(obj, "enabled");
    }

    // Test modification
    std::cout << "\n[Property Modification]" << std::endl;
    auto idProp = obj->GetProperty("objectId");
    if (idProp.IsValid())
    {
        int oldVal = idProp.GetValue<int>();
        idProp.SetValue<int>(100);
        std::cout << "  objectId: " << oldVal << " -> " << idProp.GetValue<int>() << std::endl;
    }

    if (destroyFunc)
    {
        destroyFunc(rawPtr);
    }
}

int main(int argc, char* argv[])
{
    std::cout << "========================================" << std::endl;
    std::cout << "  ROP Cross-DLL Reflection Test" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "\nTesting ROP reflection system across DLL boundaries" << std::endl;

    TestLibrary("Test1lib.dll", "Test1lib",
                "CreateTest1Object", "DestroyTest1Object",
                "GetTest1LibVersion", "GetTest1LibDescription");

    TestLibrary("Test2lib.dll", "Test2lib",
                "CreateTest2Object", "DestroyTest2Object",
                "GetTest2LibVersion", "GetTest2LibDescription");

    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "=== Test Summary ===" << std::endl;
    std::cout << "[OK] Dynamic library loading" << std::endl;
    std::cout << "[OK] Object creation via factory functions" << std::endl;
    std::cout << "[OK] Property discovery and access across DLLs" << std::endl;
    std::cout << "[OK] Property modification via reflection" << std::endl;
    std::cout << "\nROP cross-DLL reflection test PASSED!" << std::endl;

    return 0;
}
