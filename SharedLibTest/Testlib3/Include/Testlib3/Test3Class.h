#pragma once

#include "Testlib3Export.h"
#include "TestCore/CorePropertyType.h"
#include <ROP/RunTimeObjectProperty.h>
#include <string>
#include <memory>
#include <sstream>
#include <iomanip>
#include <vector>

namespace Testlib3
{

class TESTLIB3_EXPORT Test3Class :
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
    DECLARE_OBJECT(Test3Class)

    registrar
        .RegisterProperty(
            TestCore::CorePropertyType::ID, "entityId",
            &Test3Class::m_entityId,
            "Entity unique identifier")
        .RegisterProperty(
            TestCore::CorePropertyType::NAME, "entityName",
            &Test3Class::m_entityName,
            "Entity name")
        .RegisterProperty(
            TestCore::CorePropertyType::DESCRIPTION, "info",
            &Test3Class::m_info,
            "Entity information")
        .RegisterProperty(
            TestCore::CorePropertyType::INT_VALUE, "count",
            &Test3Class::m_count,
            "Item count")
        .RegisterProperty(
            TestCore::CorePropertyType::DOUBLE_VALUE, "factor",
            &Test3Class::m_factor,
            "Multiplier factor")
        .RegisterProperty(
            TestCore::CorePropertyType::ENABLED, "active",
            &Test3Class::m_active,
            "Is active")
        .RegisterOptionalProperty(
            TestCore::CorePropertyType::STATUS, "connectionState",
            &Test3Class::m_connectionState,
            { "Disconnected", "Connecting", "Connected", "Error" },
            "Connection status")
        .RegisterOptionalProperty(
            TestCore::CorePropertyType::LEVEL, "priority",
            &Test3Class::m_priority,
            { "Low", "Normal", "High", "Critical" },
            "Priority level");

    END_DECLARE_OBJECT()

public:
    Test3Class();
    virtual ~Test3Class() = default;

    // Direct API methods (in addition to reflection)
    int GetEntityId() const { return m_entityId; }
    void SetEntityId(int id) { m_entityId = id; }

    std::string GetEntityName() const { return m_entityName; }
    void SetEntityName(const std::string& name) { m_entityName = name; }

    bool IsActive() const { return m_active; }
    void SetActive(bool active) { m_active = active; }

    bool Validate() const
    {
        return m_entityId >= 0 && !m_entityName.empty() && m_count >= 0;
    }

    std::string GetSummary() const
    {
        std::ostringstream oss;
        oss << "Test3Class [ID=" << m_entityId
            << ", Name=" << m_entityName
            << ", Count=" << m_count
            << ", Factor=" << std::fixed << std::setprecision(2) << m_factor
            << ", Active=" << (m_active ? "yes" : "no")
            << "]";
        return oss.str();
    }

    void Process()
    {
        m_count++;
        m_factor *= 1.1;
        if (m_factor > 100.0) m_factor = 100.0;
    }

    double Calculate() const
    {
        return static_cast<double>(m_count) * m_factor;
    }

private:
    int m_entityId;
    std::string m_entityName;
    std::string m_info;
    int m_count;
    double m_factor;
    bool m_active;
    int m_connectionState;  // 0: Disconnected, 1: Connecting, 2: Connected, 3: Error
    int m_priority;         // 0: Low, 1: Normal, 2: High, 3: Critical
};

}
