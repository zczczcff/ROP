#include "Test2lib/Test2Class.h"
#include <iostream>

namespace Test2lib
{

Test2Class::Test2Class()
    : m_id(0)
    , m_name("DefaultTest2Object")
    , m_description("A test object from Test2lib")
    , m_value(0)
    , m_precision(0.0)
    , m_enabled(true)
    , m_mode(2)  // ReadWrite
    , m_priority(1)  // Normal
{
}

int Test2Class::GetObjectId() const
{
    return m_id;
}

void Test2Class::SetObjectId(int id)
{
    m_id = id;
}

std::string Test2Class::GetObjectName() const
{
    return m_name;
}

void Test2Class::SetObjectName(const std::string& name)
{
    m_name = name;
}

bool Test2Class::Validate() const
{
    return m_id >= 0 &&
           !m_name.empty() &&
           m_value >= 0;
}

std::string Test2Class::GetSummary() const
{
    std::ostringstream oss;
    oss << "Test2Class [ID=" << m_id
        << ", Name=" << m_name
        << ", Value=" << m_value
        << ", Precision=" << std::fixed << std::setprecision(4) << m_precision
        << ", Enabled=" << (m_enabled ? "true" : "false")
        << ", Mode=";

    switch (m_mode)
    {
        case 0: oss << "Read"; break;
        case 1: oss << "Write"; break;
        case 2: oss << "ReadWrite"; break;
        case 3: oss << "Append"; break;
        default: oss << "Unknown"; break;
    }

    oss << "]";
    return oss.str();
}

void Test2Class::Execute()
{
    m_value++;
    m_precision += 0.001;
}

std::vector<std::string> Test2Class::GetDetails() const
{
    return m_details;
}

void Test2Class::AddDetail(const std::string& detail)
{
    m_details.push_back(detail);
}

extern "C" {

Test2Class* CreateTest2Object()
{
    return new Test2Class();
}

void DestroyTest2Object(Test2Class* obj)
{
    delete obj;
}

const char* GetTest2LibVersion()
{
    return "Test2lib Version 1.0.0";
}

const char* GetTest2LibDescription()
{
    return "Test2lib - A test dynamic library implementing Test2Class";
}

} // extern "C"

} // namespace Test2lib
