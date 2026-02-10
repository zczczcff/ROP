#pragma once

#include "Test1libExport.h"
#include "TestCore/CorePropertyType.h"
#include <ROP/RunTimeObjectProperty.h>
#include <string>
#include <memory>
#include <sstream>
#include <iomanip>

namespace Test1lib
{

class Test1Class :
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
    DECLARE_OBJECT(Test1Class)

    registrar
        .RegisterProperty(
            TestCore::CorePropertyType::ID, "objectId",
            &Test1Class::m_objectId,
            "Object unique identifier")
        .RegisterProperty(
            TestCore::CorePropertyType::NAME, "objectName",
            &Test1Class::m_objectName,
            "Object name")
        .RegisterProperty(
            TestCore::CorePropertyType::DESCRIPTION, "description",
            &Test1Class::m_description,
            "Object description")
        .RegisterProperty(
            TestCore::CorePropertyType::INT_VALUE, "score",
            &Test1Class::m_score,
            "Score value")
        .RegisterProperty(
            TestCore::CorePropertyType::FLOAT_VALUE, "ratio",
            &Test1Class::m_ratio,
            "Ratio value")
        .RegisterOptionalProperty(
            TestCore::CorePropertyType::STATUS, "status",
            &Test1Class::m_status,
            { "Inactive", "Active", "Paused", "Completed" },
            "Current status")
        .RegisterOptionalProperty(
            TestCore::CorePropertyType::LEVEL, "level",
            &Test1Class::m_level,
            { "Beginner", "Intermediate", "Advanced", "Expert" },
            "Skill level");

    END_DECLARE_OBJECT()

public:
    Test1Class();
    virtual ~Test1Class() = default;

    int GetObjectId() const;
    void SetObjectId(int id);

    std::string GetObjectName() const;
    void SetObjectName(const std::string& name);

    bool Validate() const;
    std::string GetSummary() const;

    void Process();
    double CalculateResult() const;

private:
    int m_objectId;
    std::string m_objectName;
    std::string m_description;
    int m_score;
    float m_ratio;
    int m_status;
    int m_level;
};

extern "C" {
    TEST1LIB_EXPORT Test1Class* CreateTest1Object();
    TEST1LIB_EXPORT void DestroyTest1Object(Test1Class* obj);
    TEST1LIB_EXPORT const char* GetTest1LibVersion();
    TEST1LIB_EXPORT const char* GetTest1LibDescription();
}

}
