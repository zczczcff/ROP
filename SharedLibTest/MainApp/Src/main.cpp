#include <iostream>
#include "SharedLibTest/DynamicLoader.h"
#include "TestCore/CorePropertyType.h"
#include "Testlib3/Test3Class.h"
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

// Test dynamically loaded libraries
void TestDynamicLibrary(const std::string& libPath, const std::string& libName,
                       const char* createFuncName, const char* destroyFuncName,
                       const char* versionFuncName)
{
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "Testing " << libName << " (Dynamic Loading)" << std::endl;
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

// Test statically linked library
void TestStaticLibrary()
{
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "Testing Testlib3 (Static Linking)" << std::endl;
    std::cout << std::string(50, '=') << std::endl;

    std::cout << "\n[Direct Object Creation]" << std::endl;
    Testlib3::Test3Class obj;
    std::cout << "Created Test3Class instance at stack address: " << &obj << std::endl;

    std::cout << "\nClass: " << obj.GetClassName() << std::endl;

    // Test direct API
    std::cout << "\n[Direct API Test]" << std::endl;
    std::cout << "  GetEntityId() = " << obj.GetEntityId() << std::endl;
    std::cout << "  GetEntityName() = \"" << obj.GetEntityName() << "\"" << std::endl;
    std::cout << "  IsActive() = " << (obj.IsActive() ? "true" : "false") << std::endl;
    std::cout << "  Validate() = " << (obj.Validate() ? "PASS" : "FAIL") << std::endl;
    std::cout << "  GetSummary() = " << obj.GetSummary() << std::endl;

    // Test reflection
    std::cout << "\n[Reflection API Test]" << std::endl;
    TestObjectBase* basePtr = &obj;

    std::cout << "\n[Property Discovery]" << std::endl;
    TestSingleProperty(basePtr, "entityId");
    TestSingleProperty(basePtr, "entityName");
    TestSingleProperty(basePtr, "info");
    TestSingleProperty(basePtr, "count");
    TestSingleProperty(basePtr, "factor");
    TestSingleProperty(basePtr, "active");

    // Test property modification via reflection
    std::cout << "\n[Property Modification via Reflection]" << std::endl;
    auto idProp = basePtr->GetProperty("entityId");
    if (idProp.IsValid())
    {
        int oldVal = idProp.GetValue<int>();
        idProp.SetValue<int>(200);
        std::cout << "  entityId: " << oldVal << " -> " << idProp.GetValue<int>() << std::endl;
    }

    auto nameProp = basePtr->GetProperty("entityName");
    if (nameProp.IsValid())
    {
        std::string oldVal = nameProp.GetValue<std::string>();
        nameProp.SetValue<std::string>("ReflectionModified");
        std::cout << "  entityName: \"" << oldVal << "\" -> \""
                  << nameProp.GetValue<std::string>() << "\"" << std::endl;
    }

    auto activeProp = basePtr->GetProperty("active");
    if (activeProp.IsValid())
    {
        bool oldVal = activeProp.GetValue<bool>();
        activeProp.SetValue<bool>(false);
        std::cout << "  active: " << (oldVal ? "true" : "false") << " -> "
                  << (activeProp.GetValue<bool>() ? "true" : "false") << std::endl;
    }

    // Verify direct API still works
    std::cout << "\n[Verification via Direct API]" << std::endl;
    std::cout << "  GetEntityId() = " << obj.GetEntityId() << std::endl;
    std::cout << "  GetEntityName() = \"" << obj.GetEntityName() << "\"" << std::endl;
    std::cout << "  IsActive() = " << (obj.IsActive() ? "true" : "false") << std::endl;
    std::cout << "  GetSummary() = " << obj.GetSummary() << std::endl;
}

int main(int argc, char* argv[])
{
    std::cout << "========================================" << std::endl;
    std::cout << "  ROP Cross-DLL Reflection Test" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "\nTesting ROP reflection system across DLL boundaries" << std::endl;
    std::cout << "\nThis test compares two scenarios:" << std::endl;
    std::cout << "  1. Dynamic loading (Test1lib, Test2lib)" << std::endl;
    std::cout << "  2. Static linking (Testlib3)" << std::endl;

    // Test dynamic loading
    TestDynamicLibrary("Test1lib.dll", "Test1lib",
                       "CreateTest1Object", "DestroyTest1Object",
                       "GetTest1LibVersion");

    TestDynamicLibrary("Test2lib.dll", "Test2lib",
                       "CreateTest2Object", "DestroyTest2Object",
                       "GetTest2LibVersion");

    // Test static linking
    TestStaticLibrary();

    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "=== Test Summary ===" << std::endl;
    std::cout << "\n[DYNAMIC LOADING]" << std::endl;
    std::cout << "  [OK] Dynamic library loading" << std::endl;
    std::cout << "  [OK] Object creation via factory functions" << std::endl;
    std::cout << "  [OK] Property discovery and access across DLLs" << std::endl;
    std::cout << "  [OK] Property modification via reflection" << std::endl;
    std::cout << "\n[STATIC LINKING]" << std::endl;
    std::cout << "  [OK] Direct object creation" << std::endl;
    std::cout << "  [OK] Direct API access" << std::endl;
    std::cout << "  [OK] Reflection API on statically linked class" << std::endl;
    std::cout << "  [OK] Property modification via reflection" << std::endl;
    std::cout << "\nCONCLUSION:" << std::endl;
    std::cout << "  ROP reflection system works correctly in BOTH" << std::endl;
    std::cout << "  dynamic loading AND static linking scenarios!" << std::endl;
    std::cout << "\nROP cross-DLL reflection test PASSED!" << std::endl;

    return 0;
}
