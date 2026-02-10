#include "Test1lib/Test1Class.h"
#include <iostream>

namespace Test1lib
{

Test1Class::Test1Class()
    : m_objectId(0)
    , m_objectName("DefaultTest1Object")
    , m_description("A test object from Test1lib")
    , m_score(0)
    , m_ratio(1.0f)
    , m_status(0)
    , m_level(0)
{
}

int Test1Class::GetObjectId() const
{
    return m_objectId;
}

void Test1Class::SetObjectId(int id)
{
    m_objectId = id;
}

std::string Test1Class::GetObjectName() const
{
    return m_objectName;
}

void Test1Class::SetObjectName(const std::string& name)
{
    m_objectName = name;
}

bool Test1Class::Validate() const
{
    return m_objectId >= 0 &&
           !m_objectName.empty() &&
           m_score >= 0 &&
           m_ratio >= 0.0f;
}

std::string Test1Class::GetSummary() const
{
    std::ostringstream oss;
    oss << "Test1Class [ID=" << m_objectId
        << ", Name=" << m_objectName
        << ", Score=" << m_score
        << ", Ratio=" << std::fixed << std::setprecision(2) << m_ratio
        << ", Status=";

    switch (m_status)
    {
        case 0: oss << "Inactive"; break;
        case 1: oss << "Active"; break;
        case 2: oss << "Paused"; break;
        case 3: oss << "Completed"; break;
        default: oss << "Unknown"; break;
    }

    oss << "]";
    return oss.str();
}

void Test1Class::Process()
{
    m_score += 10;
    if (m_score > 100) m_score = 100;
}

double Test1Class::CalculateResult() const
{
    return static_cast<double>(m_score) * static_cast<double>(m_ratio);
}

extern "C" {

Test1Class* CreateTest1Object()
{
    return new Test1Class();
}

void DestroyTest1Object(Test1Class* obj)
{
    delete obj;
}

const char* GetTest1LibVersion()
{
    return "Test1lib Version 1.0.0";
}

const char* GetTest1LibDescription()
{
    return "Test1lib - A test dynamic library implementing Test1Class";
}

}

}
