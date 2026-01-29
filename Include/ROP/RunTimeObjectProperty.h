#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <memory>
#include <typeinfo>
#include <cstddef>
#include <type_traits>
#include <stdexcept>
#include <algorithm>
#include <map>
#include <chrono>
#include <iomanip>
#include <list>
#include <initializer_list>

namespace ROP
{

    // 前向声明
    template<typename EnumType>
    class PropertyObject;
    template<typename EnumType>
    struct PropertyMeta;
    // 属性模板类，包装属性和其枚举类型
    template<typename EnumType>
    class Property
    {
    public:
        // 默认构造函数 - 创建无效的属性
        Property() : m_type(EnumType{}), m_metaPtr(nullptr), m_objPtr(nullptr)
        {
        }

        Property(const EnumType type, const void* metaPtr, PropertyObject<EnumType>* objPtr)
            : m_type(type), m_metaPtr(metaPtr), m_objPtr(objPtr)
        {
        }

        // 判断属性是否有效
        bool IsValid() const
        {
            return m_metaPtr != nullptr && m_objPtr != nullptr;
        }

        // 获取属性枚举类型
        EnumType GetType() const
        {
            if (!IsValid())
                throw std::runtime_error("Invalid property: cannot get type");
            return m_type;
        }

        // 获取属性值（指定类型）
        template<typename T>
        T GetValue() const
        {
            if (!IsValid())
                throw std::runtime_error("Invalid property: cannot get value");
            if (!m_objPtr)
                throw std::runtime_error("Invalid property object");
            return m_objPtr->template GetPropertyValue<T>(m_metaPtr);
        }

        // 设置属性值（指定类型）
        template<typename T>
        void SetValue(const T& value)
        {
            if (!IsValid())
                throw std::runtime_error("Invalid property: cannot set value");
            if (!m_objPtr)
                throw std::runtime_error("Invalid property object");
            m_objPtr->template SetPropertyValue<T>(m_metaPtr, value);
        }

        // 获取属性元数据指针
        const void* GetMetaPtr() const
        {
            if (!IsValid())
                throw std::runtime_error("Invalid property: cannot get meta pointer");
            return m_metaPtr;
        }

        // 获取所属对象
        PropertyObject<EnumType>* GetObject() const
        {
            if (!IsValid())
                throw std::runtime_error("Invalid property: cannot get object");
            return m_objPtr;
        }

        // 获取属性描述
        std::string GetDescription() const
        {
            if (!IsValid())
                return "";

            const PropertyMeta<EnumType>* meta = static_cast<const PropertyMeta<EnumType>*>(m_metaPtr);
            return meta ? meta->description : "";
        }

        // 获取属性名称
        std::string GetName() const
        {
            if (!IsValid())
                return "";

            const PropertyMeta<EnumType>* meta = static_cast<const PropertyMeta<EnumType>*>(m_metaPtr);
            return meta ? meta->name : "";
        }

        // 获取属性所属类名
        std::string GetClassName() const
        {
            if (!IsValid())
                return "";

            const PropertyMeta<EnumType>* meta = static_cast<const PropertyMeta<EnumType>*>(m_metaPtr);
            return meta ? meta->className : "";
        }

    private:
        EnumType m_type;
        const void* m_metaPtr;
        PropertyObject<EnumType>* m_objPtr;
    };

    // 属性元数据
    template<typename EnumType>
    struct PropertyMeta
    {
        std::string name;
        EnumType enumType;
        std::string typeName;
        size_t offset;
        std::string className;
        std::function<void* (PropertyObject<EnumType>*)> getter;
        std::function<void(PropertyObject<EnumType>*, void*)> setter;
        bool isCustomAccessor;
        size_t registrationOrder = 0;

        // 新增：是否为选项属性标志
        bool isOptional = false;

        // 新增：属性描述
        std::string description;

        bool operator==(const PropertyMeta& other) const
        {
            return name == other.name && className == other.className && enumType == other.enumType;
        }

        bool operator<(const PropertyMeta& other) const
        {
            return registrationOrder < other.registrationOrder;
        }
    };

    // 为PropertyMeta提供哈希支持
    template<typename EnumType>
    struct PropertyMetaHash
    {
        size_t operator()(const PropertyMeta<EnumType>& prop) const
        {
            return std::hash<std::string>()(prop.name) ^
                (std::hash<std::string>()(prop.className) << 1) ^
                (std::hash<int>()(static_cast<int>(prop.enumType)) << 2);
        }
    };

    // 属性容器类型
    template<typename EnumType>
    using PropertyMap = std::unordered_map<std::string, PropertyMeta<EnumType>>;

    template<typename EnumType>
    using PropertyMultiMap = std::unordered_multimap<std::string, PropertyMeta<EnumType>>;

    template<typename EnumType>
    using PropertyList = std::vector<PropertyMeta<EnumType>>;

    template<typename EnumType>
    using PropertySet = std::unordered_set<PropertyMeta<EnumType>, PropertyMetaHash<EnumType>>;

    template<typename EnumType>
    using ClassNameList = std::vector<std::string>;

    // 属性数据结构体 - 将所有静态数据结构合并到这里
    template<typename EnumType>
    struct PropertyData
    {
        // 属性映射表
        PropertyMap<EnumType> ownPropertyMap;                              // 自身属性映射表
        PropertyMap<EnumType> directPropertyMap;                           // 直接属性映射表（O(1)查找）
        std::unordered_map<std::string, PropertyMap<EnumType>> parentPropertyMaps; // 父类属性映射表（类名 -> 属性表）
        PropertyMap<EnumType> combinedPropertyMap;                         // 合并后的完整属性表

        // 属性多映射表（允许多个同名属性）
        PropertyMultiMap<EnumType> allPropertiesMultiMap;

        // 属性列表
        PropertyList<EnumType> ownPropertiesList;                          // 自身属性列表（按注册顺序）
        PropertyList<EnumType> allPropertiesList;                          // 所有属性列表（包括继承的，允许同名）

        // 有序属性名称
        std::vector<std::string> orderedPropertyNames;                     // 自身属性按注册顺序的名称列表
        std::unordered_map<std::string, std::vector<std::string>> parentOrderedPropertyNames; // 父类有序属性名称列表

        // 父类属性列表映射
        std::unordered_map<std::string, PropertyList<EnumType>> parentPropertiesListMap;

        // 父类名称列表（按继承顺序：直接父类在前，最远祖先在后）
        ClassNameList<EnumType> allParentsName;

        // 新增：选项列表映射（按类名和属性名存储）
        std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> optionalPropertyMap;

        // 新增：描述映射表（按类名和属性名存储）
        std::unordered_map<std::string, std::unordered_map<std::string, std::string>> descriptionMap;

        // 注册计数器
        size_t registrationCounter = 0;

        // 初始化标志
        bool initialized = false;
    };

    // 可选属性类，继承自Property，提供选项相关功能
    template<typename EnumType>
    class OptionalProperty : public Property<EnumType>
    {
    public:
        // 默认构造函数 - 创建无效的可选属性
        OptionalProperty() : Property<EnumType>()
        {
        }

        OptionalProperty(const Property<EnumType>& prop)
            : Property<EnumType>(prop)
        {
            // 从PropertyData中获取选项列表
            InitializeOptionList();
        }

        OptionalProperty(const OptionalProperty& other)
            : Property<EnumType>(other), m_optionList(other.m_optionList)
        {
        }

        OptionalProperty& operator=(const OptionalProperty& other)
        {
            if (this != &other)
            {
                Property<EnumType>::operator=(other);
                m_optionList = other.m_optionList;
            }
            return *this;
        }

        // 获取当前选项的字符串
        std::string GetOptionString() const
        {
            if (!this->IsValid())
                return "";

            // 先通过getter获取选项的整数值
            int currentValue = this->template GetValue<int>();

            // 在当前属性所属类的选项列表中查找
            auto classOptions = GetOptionListForThisClass();
            if (currentValue >= 0 && currentValue < static_cast<int>(classOptions.size()))
            {
                return classOptions[currentValue];
            }

            // 如果没找到，在所有选项列表中查找
            if (currentValue >= 0 && currentValue < static_cast<int>(m_optionList.size()))
            {
                return m_optionList[currentValue];
            }

            return "";
        }

        // 获取所有选项列表（当前类+所有父类）
        const std::vector<std::string>& GetOptionList() const
        {
            return m_optionList;
        }

        // 通过字符串设置选项
        bool SetOptionByString(const std::string& optionStr)
        {
            if (!this->IsValid())
                return false;

            // 在当前属性所属类的选项列表中查找
            auto classOptions = GetOptionListForThisClass();
            for (size_t i = 0; i < classOptions.size(); ++i)
            {
                if (classOptions[i] == optionStr)
                {
                    // 找到对应索引，通过setter设置整数值
                    this->SetValue<int>(static_cast<int>(i));
                    return true;
                }
            }

            // 如果没找到，在所有选项列表中查找
            for (size_t i = 0; i < m_optionList.size(); ++i)
            {
                if (m_optionList[i] == optionStr)
                {
                    // 找到对应索引，通过setter设置整数值
                    this->SetValue<int>(static_cast<int>(i));
                    return true;
                }
            }

            return false;
        }

        // 通过索引设置选项
        bool SetOptionByIndex(int index)
        {
            if (!this->IsValid())
                return false;

            auto classOptions = GetOptionListForThisClass();
            if (index >= 0 && index < static_cast<int>(classOptions.size()))
            {
                this->SetValue<int>(index);
                return true;
            }

            if (index >= 0 && index < static_cast<int>(m_optionList.size()))
            {
                this->SetValue<int>(index);
                return true;
            }

            return false;
        }

        // 检查是否是选项属性
        bool IsOptional() const
        {
            if (!this->IsValid())
                return false;

            const PropertyMeta<EnumType>* meta = static_cast<const PropertyMeta<EnumType>*>(this->GetMetaPtr());
            return meta && meta->isOptional;
        }

        // 获取选项数量
        size_t GetOptionCount() const
        {
            return m_optionList.size();
        }

    private:
        // 获取当前属性所属类的选项列表（从PropertyData中查找）
        std::vector<std::string> GetOptionListForThisClass() const
        {
            if (!this->IsValid())
                return {};

            const PropertyMeta<EnumType>* meta = static_cast<const PropertyMeta<EnumType>*>(this->GetMetaPtr());
            if (!meta || !meta->isOptional)
                return {};

            auto* obj = this->GetObject();
            if (!obj)
                return {};

            // 通过PropertyData获取选项列表
            auto& propertyData = obj->GetPropertyData();
            auto classIt = propertyData.optionalPropertyMap.find(meta->className);
            if (classIt != propertyData.optionalPropertyMap.end())
            {
                auto propIt = classIt->second.find(meta->name);
                if (propIt != classIt->second.end())
                {
                    return propIt->second;
                }
            }

            return {};
        }

        // 初始化选项列表（合并当前类和所有父类的选项）
        void InitializeOptionList()
        {
            m_optionList.clear();

            if (!this->IsValid())
                return;

            const PropertyMeta<EnumType>* meta = static_cast<const PropertyMeta<EnumType>*>(this->GetMetaPtr());
            if (!meta || !meta->isOptional)
                return;

            auto* obj = this->GetObject();
            if (!obj)
                return;

            // 首先获取当前属性所属类的选项列表
            auto currentClassOptions = GetOptionListForThisClass();
            for (const auto& option : currentClassOptions)
            {
                m_optionList.push_back(option);
            }

            // 如果当前对象有效，还需要获取父类的同名属性的选项列表
            // 获取所有父类名称
            const auto& parentNames = obj->GetAllParentsName();

            // 对每个父类，查找同名属性
            for (const auto& parentClassName : parentNames)
            {
                // 从PropertyData中查找父类的选项列表
                auto& propertyData = obj->GetPropertyData();
                auto parentClassIt = propertyData.optionalPropertyMap.find(parentClassName);
                if (parentClassIt == propertyData.optionalPropertyMap.end())
                    continue;

                auto parentPropIt = parentClassIt->second.find(meta->name);
                if (parentPropIt == parentClassIt->second.end())
                    continue;

                // 添加父类的选项，但要避免重复
                for (const auto& parentOption : parentPropIt->second)
                {
                    // 检查是否已存在相同的选项
                    bool exists = false;
                    for (const auto& existingOption : m_optionList)
                    {
                        if (existingOption == parentOption)
                        {
                            exists = true;
                            break;
                        }
                    }

                    if (!exists)
                    {
                        m_optionList.push_back(parentOption);
                    }
                }
            }
        }

    private:
        std::vector<std::string> m_optionList; // 缓存的合并后的选项列表
    };

    // ==================== 公共逻辑提取 - 辅助函数 ====================

    // 属性系统工具类
    template<typename EnumType>
    class PropertySystemUtils
    {
    public:
        // 注册父类属性到父类映射表
        template<typename ParentClass>
        static void RegisterParentProperties(PropertyData<EnumType>& propertyData,
            const std::string& parentClassName)
        {
            auto& parentPropertyData = ParentClass::GetPropertyDataStatic();

            // 复制父类的自身属性
            PropertyMap<EnumType> parentCopy;
            for (const auto& pair : parentPropertyData.ownPropertyMap)
            {
                parentCopy[pair.first] = pair.second;
            }
            propertyData.parentPropertyMaps[parentClassName] = parentCopy;

            // 保存父类有序属性名称
            propertyData.parentOrderedPropertyNames[parentClassName] = parentPropertyData.orderedPropertyNames;

            // 递归注册祖先类的属性
            for (const auto& parentPair : parentPropertyData.parentPropertyMaps)
            {
                propertyData.parentPropertyMaps[parentPair.first] = parentPair.second;
            }

            // 递归注册祖先类的有序属性名称
            for (const auto& parentPair : parentPropertyData.parentOrderedPropertyNames)
            {
                propertyData.parentOrderedPropertyNames[parentPair.first] = parentPair.second;
            }

            // 递归注册祖先类的选项属性
            for (const auto& parentPair : parentPropertyData.optionalPropertyMap)
            {
                propertyData.optionalPropertyMap[parentPair.first] = parentPair.second;
            }

            // 递归注册祖先类的描述
            for (const auto& parentPair : parentPropertyData.descriptionMap)
            {
                propertyData.descriptionMap[parentPair.first] = parentPair.second;
            }
        }

        // 构建所有父类名称列表
        template<typename ParentClass>
        static void BuildAllParentsNameList(PropertyData<EnumType>& propertyData,
            const std::string& parentClassName)
        {
            propertyData.allParentsName.clear();

            if constexpr (!std::is_same_v<ParentClass, PropertyObject<EnumType>>)
            {
                // 添加直接父类
                propertyData.allParentsName.push_back(parentClassName);

                // 递归获取父类的父类
                auto& grandParents = ParentClass::GetPropertyDataStatic().allParentsName;
                for (const auto& grandParent : grandParents)
                {
                    propertyData.allParentsName.push_back(grandParent);
                }
            }
        }

        // 构建父类属性列表映射
        static void BuildParentPropertiesListMap(PropertyData<EnumType>& propertyData)
        {
            propertyData.parentPropertiesListMap.clear();

            // 从父类映射构建有序列表
            for (const auto& classPair : propertyData.parentPropertyMaps)
            {
                const std::string& className = classPair.first;
                const auto& propertyMap = classPair.second;
                auto orderedIt = propertyData.parentOrderedPropertyNames.find(className);

                if (orderedIt != propertyData.parentOrderedPropertyNames.end())
                {
                    // 按照注册顺序构建属性列表
                    PropertyList<EnumType> orderedList;
                    for (const auto& propName : orderedIt->second)
                    {
                        auto propIt = propertyMap.find(propName);
                        if (propIt != propertyMap.end())
                        {
                            orderedList.push_back(propIt->second);
                        }
                    }
                    propertyData.parentPropertiesListMap[className] = orderedList;
                }
            }
        }

        // 构建所有属性列表（包括父类） - 允许同名属性，先子类后父类，且每个类内有序
        static void BuildAllPropertiesList(PropertyData<EnumType>& propertyData)
        {
            propertyData.allPropertiesList.clear();
            propertyData.allPropertiesMultiMap.clear();

            // 获取所有父类名称（按继承顺序：直接父类在前，最远祖先在后）
            auto& allParentsName = propertyData.allParentsName;

            // 第一步：首先添加自己的属性（按注册顺序）
            for (const auto& prop : propertyData.ownPropertiesList)
            {
                propertyData.allPropertiesList.push_back(prop);
                propertyData.allPropertiesMultiMap.insert({ prop.name, prop });
            }

            // 第二步：然后按继承顺序添加父类的属性（从直接父类到最远祖先）
            for (const auto& parentClassName : allParentsName)
            {
                auto it = propertyData.parentPropertiesListMap.find(parentClassName);
                if (it != propertyData.parentPropertiesListMap.end())
                {
                    for (const auto& prop : it->second)
                    {
                        propertyData.allPropertiesList.push_back(prop);
                        propertyData.allPropertiesMultiMap.insert({ prop.name, prop });
                    }
                }
            }
        }

        // 初始化属性数据（合并多个步骤）
        static void InitializePropertyData(PropertyData<EnumType>& propertyData)
        {
            // 初始化直接属性映射
            propertyData.directPropertyMap.clear();
            for (const auto& propPair : propertyData.ownPropertyMap)
            {
                propertyData.directPropertyMap[propPair.first] = propPair.second;
            }

            // 合并所有属性到完整属性表（同名属性只保留一个，优先子类）
            propertyData.combinedPropertyMap.clear();

            // 先添加所有父类的属性
            for (const auto& classPair : propertyData.parentPropertyMaps)
            {
                for (const auto& propPair : classPair.second)
                {
                    // 如果还没有这个属性（同名），则添加
                    if (propertyData.combinedPropertyMap.find(propPair.first) == propertyData.combinedPropertyMap.end())
                    {
                        propertyData.combinedPropertyMap[propPair.first] = propPair.second;
                    }
                }
            }

            // 然后添加自己的属性（会覆盖父类的同名属性）
            for (const auto& propPair : propertyData.ownPropertyMap)
            {
                propertyData.combinedPropertyMap[propPair.first] = propPair.second;
            }

            // 按照注册顺序初始化自身属性列表
            propertyData.ownPropertiesList.clear();
            for (const auto& name : propertyData.orderedPropertyNames)
            {
                auto it = propertyData.ownPropertyMap.find(name);
                if (it != propertyData.ownPropertyMap.end())
                {
                    propertyData.ownPropertiesList.push_back(it->second);
                }
            }
        }
    };

    // 基类模板，带枚举类型参数
    template<typename EnumType>
    class PropertyObject
    {
        friend class Property<EnumType>;
    public:
        using EnumClass = EnumType;
        using PropertyDataType = PropertyData<EnumType>;

        virtual ~PropertyObject() = default;

        // 获取类名
        virtual std::string GetClassName() const = 0;

        // 获取属性数据结构体（供OptionalProperty使用）
        virtual const PropertyData<EnumType>& GetPropertyData() const = 0;

        static void StaticInitializeProperties() {};

        // 公共成员函数 - 通过GetPropertyData()统一访问

        // 获取父类名列表
        const ClassNameList<EnumType>& GetAllParentsName() const
        {
            return GetPropertyData().allParentsName;
        }

        // 获取自身定义的属性（不包含继承的）
        const PropertyList<EnumType>& GetOwnPropertiesList() const
        {
            return GetPropertyData().ownPropertiesList;
        }

        // 获取所有属性（包括继承的，允许多个同名的属性）
        const PropertyMultiMap<EnumType>& GetAllPropertiesMultiMap() const
        {
            return GetPropertyData().allPropertiesMultiMap;
        }

        // 获取父类属性映射表
        const std::unordered_map<std::string, PropertyMap<EnumType>>& GetParentPropertiesMap() const
        {
            return GetPropertyData().parentPropertyMaps;
        }

        // 获取直接属性映射（仅包含自身属性，O(1)查找）
        const PropertyMap<EnumType>& GetDirectPropertyMap() const
        {
            return GetPropertyData().directPropertyMap;
        }

        // 获取指定父类的属性列表（有序）
        const PropertyList<EnumType>& GetParentPropertiesList(const std::string& parentClassName) const
        {
            const auto& parentPropertiesListMap = GetPropertyData().parentPropertiesListMap;
            auto it = parentPropertiesListMap.find(parentClassName);
            if (it != parentPropertiesListMap.end())
            {
                return it->second;
            }
            static PropertyList<EnumType> emptyList;
            return emptyList;
        }

        // 获取自身可用属性列表（包括继承的，允许同名）
        const PropertyList<EnumType>& GetAllPropertiesList() const
        {
            return GetPropertyData().allPropertiesList;
        }

        // 通过名称获取属性包装对象 - 如果有多个同名属性，返回第一个（子类的）
        Property<EnumType> GetProperty(const std::string& name) const
        {
            // 优先从直接属性映射中查找（O(1)）
            const auto& directMap = GetDirectPropertyMap();
            auto it = directMap.find(name);
            if (it != directMap.end())
            {
                return Property<EnumType>(it->second.enumType, &it->second, const_cast<PropertyObject<EnumType>*>(this));
            }

            // 如果直接映射中没找到，再从所有属性中查找
            auto& allProps = GetAllPropertiesMultiMap();
            auto range = allProps.equal_range(name);

            if (range.first != range.second)
            {
                return Property<EnumType>(range.first->second.enumType, &range.first->second, const_cast<PropertyObject<EnumType>*>(this));
            }

            // 找不到时返回无效的Property对象
            return Property<EnumType>();
        }

        // 通过名称和类名获取属性包装对象 - 精确查找特定类的属性
        Property<EnumType> GetProperty(const std::string& name, const std::string& className) const
        {
            auto& allProps = GetAllPropertiesMultiMap();
            auto range = allProps.equal_range(name);

            for (auto it = range.first; it != range.second; ++it)
            {
                if (it->second.className == className)
                {
                    return Property<EnumType>(it->second.enumType, &it->second, const_cast<PropertyObject<EnumType>*>(this));
                }
            }

            // 找不到时返回无效的Property对象
            return Property<EnumType>();
        }

        // 获取所有同名属性（返回列表）
        std::vector<Property<EnumType>> GetAllPropertiesByName(const std::string& name) const
        {
            std::vector<Property<EnumType>> result;
            auto& allProps = GetAllPropertiesMultiMap();
            auto range = allProps.equal_range(name);

            for (auto it = range.first; it != range.second; ++it)
            {
                result.push_back(Property<EnumType>(it->second.enumType, &it->second, const_cast<PropertyObject<EnumType>*>(this)));
            }

            return result;
        }

        // 获取所有属性（包括继承的），按顺序（先子类后父类，每个类内按注册顺序）
        std::vector<Property<EnumType>> GetAllPropertiesOrdered() const
        {
            std::vector<Property<EnumType>> result;
            const auto& allPropsList = GetAllPropertiesList();

            for (const auto& meta : allPropsList)
            {
                result.push_back(Property<EnumType>(meta.enumType, &meta, const_cast<PropertyObject<EnumType>*>(this)));
            }

            return result;
        }

    protected:
        // 内部方法：通过属性元数据指针获取属性值
        template<typename T>
        T GetPropertyValue(const void* metaPtr) const
        {
            const PropertyMeta<EnumType>* meta = static_cast<const PropertyMeta<EnumType>*>(metaPtr);
            if (!meta) throw std::runtime_error("Invalid property meta pointer");

            void* ptr = meta->getter(const_cast<PropertyObject<EnumType>*>(this));
            return *reinterpret_cast<T*>(ptr);
        }

        // 内部方法：通过属性元数据指针设置属性值
        template<typename T>
        void SetPropertyValue(const void* metaPtr, const T& value)
        {
            const PropertyMeta<EnumType>* meta = static_cast<const PropertyMeta<EnumType>*>(metaPtr);
            if (!meta) throw std::runtime_error("Invalid property meta pointer");

            T temp = value;
            meta->setter(const_cast<PropertyObject<EnumType>*>(this), &temp);
        }
    public:
        // 查找属性（检查所有属性，包括继承的）
        bool HasProperty(const std::string& name) const
        {
            // 优先检查直接属性映射（O(1)）
            const auto& directMap = GetDirectPropertyMap();
            if (directMap.find(name) != directMap.end())
                return true;

            // 检查所有属性
            auto& allProps = GetAllPropertiesMultiMap();
            return allProps.find(name) != allProps.end();
        }

        // 检查特定类中是否有指定属性
        bool HasProperty(const std::string& name, const std::string& className) const
        {
            auto& allProps = GetAllPropertiesMultiMap();
            auto range = allProps.equal_range(name);

            for (auto it = range.first; it != range.second; ++it)
            {
                if (it->second.className == className)
                    return true;
            }
            return false;
        }

        // 获取指定父类的属性列表（按照注册顺序）
        PropertyList<EnumType> GetParentClassProperties(const std::string& parentClassName) const
        {
            PropertyList<EnumType> result;
            auto& allProps = GetAllPropertiesMultiMap();

            for (const auto& pair : allProps)
            {
                if (pair.second.className == parentClassName)
                {
                    result.push_back(pair.second);
                }
            }

            // 按照注册顺序排序
            std::sort(result.begin(), result.end(),
                [](const PropertyMeta<EnumType>& a, const PropertyMeta<EnumType>& b)
                {
                    return a.registrationOrder < b.registrationOrder;
                });

            return result;
        }

        // 获取指定父类的属性映射表
        PropertyMap<EnumType> GetParentClassPropertyMap(const std::string& parentClassName) const
        {
            auto& parentMaps = GetParentPropertiesMap();
            auto it = parentMaps.find(parentClassName);
            if (it != parentMaps.end())
            {
                return it->second;
            }
            return PropertyMap<EnumType>();
        }

        // 将Property转换为OptionalProperty
        OptionalProperty<EnumType> ToOptionalProperty(const Property<EnumType>& prop) const
        {
            // 如果属性无效，返回无效的OptionalProperty
            if (!prop.IsValid())
            {
                return OptionalProperty<EnumType>();
            }

            const PropertyMeta<EnumType>* meta = static_cast<const PropertyMeta<EnumType>*>(prop.GetMetaPtr());

            if (!meta || !meta->isOptional)
            {
                throw std::runtime_error("Property is not an optional property");
            }

            return OptionalProperty<EnumType>(prop);
        }

        // 通过Property获取OptionalProperty
        OptionalProperty<EnumType> GetPropertyAsOptional(const std::string& name) const
        {
            auto prop = GetProperty(name);
            if (!prop.IsValid())
            {
                return OptionalProperty<EnumType>();
            }
            return ToOptionalProperty(prop);
        }

        // 通过名称和类名获取OptionalProperty
        OptionalProperty<EnumType> GetPropertyAsOptional(const std::string& name, const std::string& className) const
        {
            auto prop = GetProperty(name, className);
            if (!prop.IsValid())
            {
                return OptionalProperty<EnumType>();
            }
            return ToOptionalProperty(prop);
        }

        // 获取属性的描述
        std::string GetPropertyDescription(const std::string& name) const
        {
            auto prop = GetProperty(name);
            return prop.GetDescription();
        }

        // 获取属性和描述（返回包含名称和描述的字符串）
        std::string GetPropertyWithDescription(const std::string& name) const
        {
            auto prop = GetProperty(name);
            if (!prop.IsValid())
            {
                return name + " - [Invalid Property]";
            }

            std::string description = prop.GetDescription();
            if (!description.empty())
            {
                return name + " - " + description;
            }
            return name;
        }

        // 获取所有同名属性，按顺序（先子类后父类，每个类内按注册顺序）
        std::vector<Property<EnumType>> GetPropertiesByNameOrdered(const std::string& name) const
        {
            std::vector<Property<EnumType>> result;
            const auto& allPropsList = GetAllPropertiesList();

            for (const auto& meta : allPropsList)
            {
                if (meta.name == name)
                {
                    result.push_back(Property<EnumType>(meta.enumType, &meta, const_cast<PropertyObject<EnumType>*>(this)));
                }
            }

            return result;
        }

        // 获取属性数量（包括同名属性）
        size_t GetPropertyCount() const
        {
            return GetAllPropertiesList().size();
        }

        // 获取不重复的属性名称列表
        std::vector<std::string> GetUniquePropertyNames() const
        {
            std::unordered_set<std::string> uniqueNames;
            const auto& allPropsList = GetAllPropertiesList();

            for (const auto& meta : allPropsList)
            {
                uniqueNames.insert(meta.name);
            }

            std::vector<std::string> result(uniqueNames.begin(), uniqueNames.end());
            return result;
        }
    };

    // 属性注册器模板类（支持流式接口）
    template<typename EnumType, typename ClassType>
    class PropertyRegistrar
    {
    public:
        PropertyRegistrar(PropertyData<EnumType>& propertyData, const std::string& className)
            : m_propertyData(propertyData), m_className(className)
        {
        }

        // 注册属性（成员变量）- 流式接口（带描述）
        template<typename PropertyType>
        PropertyRegistrar& RegisterProperty(
            EnumType enumType,
            const std::string& name,
            PropertyType ClassType::* memberPtr,
            const std::string& description = "")
        {
            // 计算偏移量 - 使用空指针技巧
            size_t offset = reinterpret_cast<size_t>(
                &(reinterpret_cast<ClassType*>(0)->*memberPtr));

            // 创建getter函数
            std::function<void* (PropertyObject<EnumType>*)> getter =
                [memberPtr](PropertyObject<EnumType>* obj) -> void*
            {
                ClassType* derived = static_cast<ClassType*>(obj);
                return &(derived->*memberPtr);
            };

            // 创建setter函数
            std::function<void(PropertyObject<EnumType>*, void*)> setter =
                [memberPtr](PropertyObject<EnumType>* obj, void* value)
            {
                ClassType* derived = static_cast<ClassType*>(obj);
                derived->*memberPtr = *static_cast<PropertyType*>(value);
            };

            // 创建属性元数据
            PropertyMeta<EnumType> meta;
            meta.name = name;
            meta.enumType = enumType;
            meta.typeName = typeid(PropertyType).name();
            meta.offset = offset;
            meta.className = m_className;
            meta.getter = getter;
            meta.setter = setter;
            meta.isCustomAccessor = false;
            meta.registrationOrder = m_propertyData.registrationCounter++;
            meta.description = description;

            // 记录注册顺序
            m_propertyData.orderedPropertyNames.push_back(name);

            // 注册到属性容器
            m_propertyData.ownPropertyMap[name] = meta;

            // 存储描述信息
            if (!description.empty())
            {
                m_propertyData.descriptionMap[m_className][name] = description;
            }

            return *this;
        }


        // 注册属性（自定义getter和setter）- 流式接口（带描述）
        template<typename PropertyType>
        PropertyRegistrar& RegisterProperty(
            EnumType enumType,
            const std::string& name,
            void (ClassType::* setterFunc)(PropertyType&),
            PropertyType& (ClassType::* getterFunc)(),
            const std::string& description = "")
        {
            // 创建getter函数
            std::function<void* (PropertyObject<EnumType>*)> getter =
                [getterFunc](PropertyObject<EnumType>* obj) -> void*
            {
                ClassType* derived = static_cast<ClassType*>(obj);
                PropertyType& ref = (derived->*getterFunc)();
                return &ref;
            };

            // 创建setter函数
            std::function<void(PropertyObject<EnumType>*, void*)> setter =
                [setterFunc](PropertyObject<EnumType>* obj, void* value)
            {
                ClassType* derived = static_cast<ClassType*>(obj);
                (derived->*setterFunc)(*static_cast<PropertyType*>(value));
            };

            // 创建属性元数据
            PropertyMeta<EnumType> meta;
            meta.name = name;
            meta.enumType = enumType;
            meta.typeName = typeid(PropertyType).name();
            meta.offset = 0; // 对于自定义访问器，偏移量无意义
            meta.className = m_className;
            meta.getter = getter;
            meta.setter = setter;
            meta.isCustomAccessor = true;
            meta.registrationOrder = m_propertyData.registrationCounter++;
            meta.description = description;

            // 记录注册顺序
            m_propertyData.orderedPropertyNames.push_back(name);

            // 注册到属性容器
            m_propertyData.ownPropertyMap[name] = meta;

            // 存储描述信息
            if (!description.empty())
            {
                m_propertyData.descriptionMap[m_className][name] = description;
            }

            return *this;
        }


        // 注册选项属性（成员变量）- 流式接口（带描述）
        template<typename PropertyType>
        PropertyRegistrar& RegisterOptionalProperty(
            EnumType enumType,
            const std::string& name,
            PropertyType ClassType::* memberPtr,
            std::initializer_list<const char*> options,
            const std::string& description = "")
        {
            // 首先注册普通属性
            RegisterProperty(enumType, name, memberPtr, description);

            // 转换选项列表
            std::vector<std::string> optionVec;
            for (const auto& option : options)
            {
                optionVec.push_back(option);
            }

            // 然后标记为选项属性
            auto& meta = m_propertyData.ownPropertyMap[name];
            meta.isOptional = true;

            // 存储到选项映射中
            m_propertyData.optionalPropertyMap[m_className][name] = optionVec;

            // 验证选项映射值从0开始且连续
            if (!optionVec.empty())
            {
                // 检查是否有重复的选项字符串
                std::unordered_set<std::string> optionSet;
                for (const auto& option : optionVec)
                {
                    if (!optionSet.insert(option).second)
                    {
                        std::cerr << "Warning: Duplicate option string '" << option
                            << "' in property '" << name << "' of class '" << m_className << "'" << std::endl;
                    }
                }
            }

            return *this;
        }


        // 注册选项属性（自定义getter和setter）- 流式接口（带描述）
        template<typename PropertyType>
        PropertyRegistrar& RegisterOptionalProperty(
            EnumType enumType,
            const std::string& name,
            void (ClassType::* setterFunc)(PropertyType&),
            PropertyType& (ClassType::* getterFunc)(),
            std::initializer_list<const char*> options,
            const std::string& description = "")
        {
            // 首先注册自定义属性
            RegisterProperty(enumType, name, setterFunc, getterFunc, description);

            // 转换选项列表
            std::vector<std::string> optionVec;
            for (const auto& option : options)
            {
                optionVec.push_back(option);
            }

            // 然后标记为选项属性
            auto& meta = m_propertyData.ownPropertyMap[name];
            meta.isOptional = true;

            // 存储到选项映射中
            m_propertyData.optionalPropertyMap[m_className][name] = optionVec;

            // 验证选项映射值从0开始且连续（同上）
            if (!optionVec.empty())
            {
                std::unordered_set<std::string> optionSet;
                for (const auto& option : optionVec)
                {
                    if (!optionSet.insert(option).second)
                    {
                        std::cerr << "Warning: Duplicate option string '" << option
                            << "' in property '" << name << "' of class '" << m_className << "'" << std::endl;
                    }
                }
            }

            return *this;
        }

        // 设置已注册属性的描述
        PropertyRegistrar& SetDescription(const std::string& name, const std::string& description)
        {
            auto it = m_propertyData.ownPropertyMap.find(name);
            if (it != m_propertyData.ownPropertyMap.end())
            {
                it->second.description = description;
                m_propertyData.descriptionMap[m_className][name] = description;
            }
            return *this;
        }

    private:
        PropertyData<EnumType>& m_propertyData;
        std::string m_className;
    };

} // namespace ROP

// ==================== 新的流式注册宏定义 ====================

// 辅助宏：初始化属性系统
#define INIT_PROPERTY_SYSTEM(EnumType, ClassName, ParentClassName) \
    { \
        auto& propertyData = GetPropertyDataStatic(); \
        if (propertyData.initialized) return true; \
        \
        using CurrentClass = ClassName; \
        const std::string classnamestring = #ClassName; \
        \
        /* 首先注册父类的属性到父类映射表 */ \
        if constexpr (!std::is_same_v<ParentClassName, ROP::PropertyObject<EnumType>>) { \
            ParentClassName::StaticInitializeProperties(); \
            ROP::PropertySystemUtils<EnumType>::RegisterParentProperties<ParentClassName>( \
                propertyData, #ParentClassName); \
        } \
        \
        using ParentClass = ParentClassName; \
        std::string ParentClassNameString = #ParentClassName;

// 辅助宏：完成属性系统初始化（合并后的版本）
#define FINALIZE_PROPERTY_SYSTEM(EnumType, ClassName) \
        /* 构建父类名称列表 */ \
        ROP::PropertySystemUtils<EnumType>::BuildAllParentsNameList<ParentClass>( \
            propertyData, ParentClassNameString); \
        \
        /* 构建父类属性列表映射 */ \
        ROP::PropertySystemUtils<EnumType>::BuildParentPropertiesListMap(propertyData); \
        \
        /* 使用合并函数初始化属性数据 */ \
        ROP::PropertySystemUtils<EnumType>::InitializePropertyData(propertyData); \
        \
        /* 构建所有属性列表（包括父类，允许同名） */ \
        ROP::PropertySystemUtils<EnumType>::BuildAllPropertiesList(propertyData); \
        \
        propertyData.initialized = true; \
        return true; \
    }

// 主宏：声明并定义所有必要函数
#define DECLARE_OBJECT_WITH_PARENT(EnumType, ClassName, ParentClassName) \
public: \
    using EnumClass = EnumType; \
    using PropertyDataType = ROP::PropertyData<EnumType>; \
    \
    /* 获取属性数据结构体（静态版本） */ \
    static PropertyDataType& GetPropertyDataStatic() { \
        static PropertyDataType s_propertyData; \
        return s_propertyData; \
    } \
    \
    /* 静态初始化函数 */ \
    static bool StaticInitializeProperties() { \
        static bool s_initialized = []() -> bool { \
            INIT_PROPERTY_SYSTEM(EnumType, ClassName, ParentClassName) \
            \
            /* 创建注册器对象 */ \
            ROP::PropertyRegistrar<EnumClass, ClassName> registrar(propertyData, classnamestring); 

// 简化宏：用于没有父类的情况
#define DECLARE_OBJECT(EnumType, ClassName) DECLARE_OBJECT_WITH_PARENT(EnumType, ClassName, ROP::PropertyObject<EnumType>)

// 结束宏
#define END_DECLARE_OBJECT(EnumType, ClassName, ParentClassName) \
            /* 完成属性系统初始化 */ \
            FINALIZE_PROPERTY_SYSTEM(EnumType, ClassName) \
        }(); \
        return s_initialized; \
    } \
    \
    /* 确保属性系统已初始化 */ \
    void EnsurePropertySystemInitialized() const { \
        static_cast<const ClassName*>(this)->StaticInitializeProperties(); \
    } \
    \
    virtual std::string GetClassName() const override { \
        return #ClassName; \
    } \
    \
    virtual const ROP::PropertyData<EnumType>& GetPropertyData() const override { \
        EnsurePropertySystemInitialized(); \
        return GetPropertyDataStatic(); \
    } \
private:


////使用示例
//#include <ROP/RunTimeObjectProperty.h>
//#include <iostream>
//
//// 定义属性枚举
//enum class DeviceProperty
//{
//    ID,
//    NAME,
//    STATUS,
//    TEMPERATURE,
//    PRESSURE,
//    OPTIONAL
//};
//
//// 设备基类
//class Device : public ROP::PropertyObject<DeviceProperty>
//{
//    DECLARE_OBJECT(DeviceProperty, Device)
//    registrar
//        .RegisterProperty(DeviceProperty::ID, "deviceId", &Device::deviceId, "设备ID")
//        .RegisterProperty(DeviceProperty::NAME, "deviceName", &Device::deviceName, "设备名称")
//        .RegisterOptionalProperty(
//            DeviceProperty::OPTIONAL, "status", &Device::status,
//            { "Offline", "Online", "Error", "Maintenance" },
//            "设备状态");
//    END_DECLARE_OBJECT(DeviceProperty, Device, ROP::PropertyObject<DeviceProperty>)
//
//public:
//    Device() : deviceId(0), status(0) {}
//
//    int deviceId;
//    std::string deviceName;
//    int status;  // 0:Offline, 1:Online, 2:Error, 3:Maintenance
//};
//
//// 温度传感器类
//class TemperatureSensor : public Device
//{
//    DECLARE_OBJECT_WITH_PARENT(DeviceProperty, TemperatureSensor, Device)
//    registrar
//        .RegisterProperty(
//            DeviceProperty::TEMPERATURE, "currentTemp", &TemperatureSensor::currentTemp,
//            "当前温度 (°C)")
//        .RegisterProperty(
//            DeviceProperty::TEMPERATURE, "targetTemp", &TemperatureSensor::targetTemp,
//            "目标温度 (°C)")
//        .RegisterOptionalProperty(
//            DeviceProperty::OPTIONAL, "unit", &TemperatureSensor::unit,
//            { "Celsius", "Fahrenheit", "Kelvin" },
//            "温度单位");
//    END_DECLARE_OBJECT(DeviceProperty, TemperatureSensor, Device)
//
//public:
//    TemperatureSensor() : currentTemp(20.0f), targetTemp(22.0f), unit(0) {}
//
//    float currentTemp;
//    float targetTemp;
//    int unit;  // 0:Celsius, 1:Fahrenheit, 2:Kelvin
//};
//
//int main()
//{
//    // 创建温度传感器
//    TemperatureSensor sensor;
//    sensor.deviceId = 1001;
//    sensor.deviceName = "LabSensor_01";
//    sensor.status = 1;  // Online
//    sensor.currentTemp = 21.5f;
//    sensor.targetTemp = 22.0f;
//    sensor.unit = 0;    // Celsius
//
//    // 1. 显示设备信息
//    std::cout << "=== 设备信息 ===" << std::endl;
//    auto idProp = sensor.GetProperty("deviceId");
//    auto nameProp = sensor.GetProperty("deviceName");
//    auto statusProp = sensor.GetPropertyAsOptional("status");
//
//    std::cout << "ID: " << idProp.GetValue<int>() << std::endl;
//    std::cout << "Name: " << nameProp.GetValue<std::string>() << std::endl;
//    std::cout << "Status: " << statusProp.GetOptionString() << std::endl;
//
//    // 2. 显示温度信息
//    std::cout << "\n=== 温度信息 ===" << std::endl;
//    auto tempProp = sensor.GetProperty("currentTemp");
//    auto targetProp = sensor.GetProperty("targetTemp");
//    auto unitProp = sensor.GetPropertyAsOptional("unit");
//
//    std::cout << "Current: " << tempProp.GetValue<float>() << "°C" << std::endl;
//    std::cout << "Target: " << targetProp.GetValue<float>() << "°C" << std::endl;
//    std::cout << "Unit: " << unitProp.GetOptionString() << std::endl;
//
//    // 3. 动态更改设置
//    std::cout << "\n=== 更改设置 ===" << std::endl;
//    unitProp.SetOptionByString("Fahrenheit");
//    std::cout << "Unit changed to: " << unitProp.GetOptionString() << std::endl;
//
//    // 4. 遍历所有属性
//    std::cout << "\n=== 所有属性 ===" << std::endl;
//    sensor.EnsurePropertySystemInitialized();
//    auto allProps = sensor.GetAllPropertiesOrdered();
//
//    for (const auto& prop : allProps)
//    {
//        std::cout << prop.GetName() << " (" << prop.GetClassName() << "): ";
//
//        try
//        {
//            if (prop.GetType() == DeviceProperty::ID)
//            {
//                std::cout << prop.GetValue<int>();
//            }
//            else if (prop.GetType() == DeviceProperty::NAME)
//            {
//                std::cout << "'" << prop.GetValue<std::string>() << "'";
//            }
//            else if (prop.GetType() == DeviceProperty::TEMPERATURE)
//            {
//                std::cout << prop.GetValue<float>();
//            }
//            else if (prop.GetType() == DeviceProperty::OPTIONAL)
//            {
//                auto optionalProp = sensor.ToOptionalProperty(prop);
//                std::cout << optionalProp.GetOptionString();
//            }
//        }
//        catch (...)
//        {
//            std::cout << "[Error reading value]";
//        }
//
//        std::cout << std::endl;
//    }
//
//    return 0;
//}