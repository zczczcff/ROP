#include "TestCore/TestCore.h"

namespace TestCore
{

// 这里可以实现一些辅助功能
// 由于 ICoreObject 是接口类，主要实现由派生类提供

// 可以添加一些全局工具函数
std::string GetPropertyTypeString(CorePropertyType type)
{
    switch (type)
    {
        case CorePropertyType::ID: return "ID";
        case CorePropertyType::NAME: return "NAME";
        case CorePropertyType::DESCRIPTION: return "DESCRIPTION";
        case CorePropertyType::INT_VALUE: return "INT_VALUE";
        case CorePropertyType::FLOAT_VALUE: return "FLOAT_VALUE";
        case CorePropertyType::DOUBLE_VALUE: return "DOUBLE_VALUE";
        case CorePropertyType::STATUS: return "STATUS";
        case CorePropertyType::ENABLED: return "ENABLED";
        case CorePropertyType::MODE: return "MODE";
        case CorePropertyType::LEVEL: return "LEVEL";
        case CorePropertyType::CUSTOM_DATA: return "CUSTOM_DATA";
        default: return "UNKNOWN";
    }
}

} // namespace TestCore
