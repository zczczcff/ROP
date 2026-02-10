#pragma once

// 核心属性枚举类型
// 这个枚举需要在所有动态库间共享
namespace TestCore
{

enum class CorePropertyType
{
    // 基础属性
    ID,
    NAME,
    DESCRIPTION,

    // 数值属性
    INT_VALUE,
    FLOAT_VALUE,
    DOUBLE_VALUE,

    // 状态属性
    STATUS,
    ENABLED,

    // 可选属性
    MODE,
    LEVEL,

    // 扩展预留
    CUSTOM_DATA
};

} // namespace TestCore
