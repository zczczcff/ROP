#pragma once

#include "Test2libExport.h"
#include "TestCore/CorePropertyType.h"
#include <ROP/RunTimeObjectProperty.h>
#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <iomanip>

namespace Test2lib
{

class Test2Class :
    public ROP::PropertyObject<
        TestCore::CorePropertyType,
        std::string,
        std::hash<std::string>,
        std::equal_to<std::string>,
        std::function<std::string(const std::string&)>,
        std::string,
        ROP::DefaultErrorCallback<std::string>
    >
{
    DECLARE_OBJECT(Test2Class)

    registrar
        .RegisterProperty(
            TestCore::CorePropertyType::ID, "objectId",
            &Test2Class::m_id,
            "Object ID")
        .RegisterProperty(
            TestCore::CorePropertyType::NAME, "name",
            &Test2Class::m_name,
            "Object name")
        .RegisterProperty(
            TestCore::CorePropertyType::DESCRIPTION, "description",
            &Test2Class::m_description,
            "Detailed description")
        .RegisterProperty(
            TestCore::CorePropertyType::INT_VALUE, "value",
            &Test2Class::m_value,
            "Integer value")
        .RegisterProperty(
            TestCore::CorePropertyType::DOUBLE_VALUE, "precision",
            &Test2Class::m_precision,
            "Precision value")
        .RegisterProperty(
            TestCore::CorePropertyType::ENABLED, "enabled",
            &Test2Class::m_enabled,
            "Is enabled")
        .RegisterOptionalProperty(
            TestCore::CorePropertyType::MODE, "mode",
            &Test2Class::m_mode,
            { "Read", "Write", "ReadWrite", "Append" },
            "Work mode")
        .RegisterOptionalProperty(
            TestCore::CorePropertyType::LEVEL, "priority",
            &Test2Class::m_priority,
            { "Low", "Normal", "High", "Critical" },
            "Priority level");

    END_DECLARE_OBJECT()

public:
    Test2Class();
    virtual ~Test2Class() = default;

    int GetObjectId() const;
    void SetObjectId(int id);

    std::string GetObjectName() const;
    void SetObjectName(const std::string& name);

    bool Validate() const;
    std::string GetSummary() const;

    void Execute();
    std::vector<std::string> GetDetails() const;
    void AddDetail(const std::string& detail);

private:
    int m_id;
    std::string m_name;
    std::string m_description;
    int m_value;
    double m_precision;
    bool m_enabled;
    int m_mode;
    int m_priority;
    std::vector<std::string> m_details;
};

extern "C" {
    TEST2LIB_EXPORT Test2Class* CreateTest2Object();
    TEST2LIB_EXPORT void DestroyTest2Object(Test2Class* obj);
    TEST2LIB_EXPORT const char* GetTest2LibVersion();
    TEST2LIB_EXPORT const char* GetTest2LibDescription();
}

}
