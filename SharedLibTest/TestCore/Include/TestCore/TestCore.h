#pragma once

#include "TestCoreExport.h"
#include "CorePropertyType.h"
#include <ROP/RunTimeObjectProperty.h>
#include <string>
#include <memory>

namespace TestCore
{

// 核心反射基类 - 继承自 ROP::PropertyObject
// 这个类将被导出，作为所有派生类的共同接口
class TESTCORE_EXPORT ICoreObject :
    public ROP::PropertyObject<
        CorePropertyType,
        std::string,
        std::hash<std::string>,
        std::equal_to<std::string>,
        std::function<std::string(const std::string&)>,
        std::string,
        ROP::DefaultErrorCallback<std::string>
    >
{
public:
    virtual ~ICoreObject() = default;

    // 使用 ROP 的类型别名
    using ROPEnumClass = CorePropertyType;
    using ROPKeyType = std::string;
    using ROPStringType = std::string;
    using ROPPropertyDataType = ROP::PropertyData<
        CorePropertyType,
        std::string,
        std::hash<std::string>,
        std::equal_to<std::string>,
        std::function<std::string(const std::string&)>,
        std::string,
        ROP::DefaultErrorCallback<std::string>
    >;

    // 扩展接口 - 跨 DLL 边界的实用方法
    virtual int GetObjectId() const = 0;
    virtual void SetObjectId(int id) = 0;

    virtual std::string GetObjectName() const = 0;
    virtual void SetObjectName(const std::string& name) = 0;

    // 验证对象完整性
    virtual bool Validate() const = 0;

    // 获取对象信息摘要
    virtual std::string GetSummary() const = 0;

protected:
    ICoreObject() = default;
};

// 对象智能指针类型
using ICoreObjectPtr = std::shared_ptr<ICoreObject>;

} // namespace TestCore
