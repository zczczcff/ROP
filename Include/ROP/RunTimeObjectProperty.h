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
#include <sstream>

namespace ROP
{
    // 默认错误输出回调
    template<typename StringType>
    struct DefaultErrorCallback
    {
        void operator()(const StringType& errorMsg) const
        {
            std::cerr << std::string(errorMsg.begin(), errorMsg.end()) << std::endl;
        }
    };

    // 前向声明
    template<typename EnumType, typename KeyType, typename KeyHash, typename KeyEqual,
        typename KeyToString, typename StringType, typename ErrorCallback>
        class PropertyObject;

    template<typename EnumType, typename KeyType, typename KeyHash, typename KeyEqual,
        typename KeyToString, typename StringType, typename ErrorCallback>
        struct PropertyMeta;

    // 属性模板类，包装属性和其枚举类型
    template<typename EnumType, typename KeyType, typename KeyHash, typename KeyEqual,
        typename KeyToString, typename StringType, typename ErrorCallback>
        class Property
    {
    public:
        using ErrorCallbackType = ErrorCallback;

        // 默认构造函数 - 创建无效的属性
        Property() : m_type(EnumType{}), m_metaPtr(nullptr), m_objPtr(nullptr)
        {
        }

        Property(const EnumType type, const void* metaPtr,
            PropertyObject<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>* objPtr)
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
            {
                ErrorCallback()("Invalid property: cannot get type");
                throw std::runtime_error("Invalid property: cannot get type");
            }
            return m_type;
        }

        // 获取属性值（指定类型）
        template<typename T>
        T GetValue() const
        {
            if (!IsValid())
            {
                ErrorCallback()("Invalid property: cannot get value");
                throw std::runtime_error("Invalid property: cannot get value");
            }
            if (!m_objPtr)
            {
                ErrorCallback()("Invalid property object");
                throw std::runtime_error("Invalid property object");
            }
            return m_objPtr->template GetPropertyValue<T>(m_metaPtr);
        }

        // 设置属性值（指定类型）
        template<typename T>
        void SetValue(const T& value)
        {
            if (!IsValid())
            {
                ErrorCallback()("Invalid property: cannot set value");
                throw std::runtime_error("Invalid property: cannot set value");
            }
            if (!m_objPtr)
            {
                ErrorCallback()("Invalid property object");
                throw std::runtime_error("Invalid property object");
            }
            m_objPtr->template SetPropertyValue<T>(m_metaPtr, value);
        }

        template<typename T>
        T* GetPointer()
        {
            if (!IsValid())
                return nullptr;
            const PropertyMeta<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>* meta =
                static_cast<const PropertyMeta<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>*>(m_metaPtr);
            void* ptr = meta->getter(const_cast<PropertyObject<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>*>(m_objPtr));
            return static_cast<T*>(ptr);
        }

        // 获取属性值的引用
        template<typename T>
        T& GetReference()
        {
            T* ptr = GetPointer<T>();
            if (!ptr)
            {
                ErrorCallback()("Failed to get property reference");
                throw std::runtime_error("Failed to get property reference");
            }
            return *ptr;
        }

        // 获取属性值的常量引用
        template<typename T>
        const T& GetConstReference() const
        {
            const T* ptr = GetConstPointer<T>();
            if (!ptr)
            {
                ErrorCallback()("Failed to get property const reference");
                throw std::runtime_error("Failed to get property const reference");
            }
            return *ptr;
        }

        template<typename T>
        const T* GetConstPointer() const
        {
            if (!IsValid())
                return nullptr;
            const PropertyMeta<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>* meta =
                static_cast<const PropertyMeta<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>*>(m_metaPtr);
            void* ptr = meta->getter(const_cast<PropertyObject<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>*>(m_objPtr));
            return static_cast<const T*>(ptr);
        }

        // 获取属性元数据指针
        const void* GetMetaPtr() const
        {
            if (!IsValid())
            {
                ErrorCallback()("Invalid property: cannot get meta pointer");
                throw std::runtime_error("Invalid property: cannot get meta pointer");
            }
            return m_metaPtr;
        }

        // 获取所属对象
        PropertyObject<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>* GetObject() const
        {
            if (!IsValid())
            {
                ErrorCallback()("Invalid property: cannot get object");
                throw std::runtime_error("Invalid property: cannot get object");
            }
            return m_objPtr;
        }

        // 获取属性描述
        StringType GetDescription() const
        {
            if (!IsValid())
                return StringType{};

            const PropertyMeta<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>* meta =
                static_cast<const PropertyMeta<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>*>(m_metaPtr);
            return meta ? meta->description : StringType{};
        }

        // 获取属性名称（返回KeyType）
        KeyType GetName() const
        {
            if (!IsValid())
                return KeyType{};

            const PropertyMeta<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>* meta =
                static_cast<const PropertyMeta<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>*>(m_metaPtr);
            return meta ? meta->name : KeyType{};
        }

        // 获取属性名称字符串（使用KeyToString转换）
        StringType GetNameString() const
        {
            if (!IsValid())
                return StringType{};

            KeyType name = GetName();
            return KeyToString()(name);
        }

        // 获取属性所属类名
        StringType GetClassName() const
        {
            if (!IsValid())
                return StringType{};

            const PropertyMeta<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>* meta =
                static_cast<const PropertyMeta<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>*>(m_metaPtr);
            return meta ? meta->className : StringType{};
        }

    private:
        EnumType m_type;
        const void* m_metaPtr;
        PropertyObject<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>* m_objPtr;
    };

    // 属性元数据
    template<typename EnumType, typename KeyType, typename KeyHash, typename KeyEqual,
        typename KeyToString, typename StringType, typename ErrorCallback>
        struct PropertyMeta
    {
        KeyType name;
        EnumType enumType;
        StringType typeName;
        size_t offset;
        StringType className;
        std::function<void* (PropertyObject<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>*)> getter;
        std::function<void(PropertyObject<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>*, void*)> setter;
        bool isCustomAccessor;
        size_t registrationOrder = 0;

        // 新增：是否为选项属性标志
        bool isOptional = false;

        // 新增：属性描述
        StringType description;

        bool operator==(const PropertyMeta& other) const
        {
            KeyEqual equal;
            return equal(name, other.name) && className == other.className && enumType == other.enumType;
        }

        bool operator<(const PropertyMeta& other) const
        {
            return registrationOrder < other.registrationOrder;
        }
    };

    // 为PropertyMeta提供哈希支持
    template<typename EnumType, typename KeyType, typename KeyHash, typename KeyEqual,
        typename KeyToString, typename StringType, typename ErrorCallback>
        struct PropertyMetaHash
    {
        size_t operator()(const PropertyMeta<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>& prop) const
        {
            KeyHash hash;
            return hash(prop.name) ^
                (std::hash<StringType>()(prop.className) << 1) ^
                (std::hash<int>()(static_cast<int>(prop.enumType)) << 2);
        }
    };

    // 属性容器类型
    template<typename EnumType, typename KeyType, typename KeyHash, typename KeyEqual,
        typename KeyToString, typename StringType, typename ErrorCallback>
        using PropertyMap = std::unordered_map<KeyType,
        PropertyMeta<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>, KeyHash, KeyEqual>;

    template<typename EnumType, typename KeyType, typename KeyHash, typename KeyEqual,
        typename KeyToString, typename StringType, typename ErrorCallback>
        using PropertyMultiMap = std::unordered_multimap<KeyType,
        PropertyMeta<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>, KeyHash, KeyEqual>;

    template<typename EnumType, typename KeyType, typename KeyHash, typename KeyEqual,
        typename KeyToString, typename StringType, typename ErrorCallback>
        using PropertyList = std::vector<PropertyMeta<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>>;

    template<typename EnumType, typename KeyType, typename KeyHash, typename KeyEqual,
        typename KeyToString, typename StringType, typename ErrorCallback>
        using PropertySet = std::unordered_set<
        PropertyMeta<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>,
        PropertyMetaHash<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>>;

    template<typename EnumType, typename KeyType, typename KeyHash, typename KeyEqual,
        typename KeyToString, typename StringType, typename ErrorCallback>
        using ClassNameList = std::vector<StringType>;

    // 属性数据结构体 - 将所有静态数据结构合并到这里
    template<typename EnumType, typename KeyType, typename KeyHash, typename KeyEqual,
        typename KeyToString, typename StringType, typename ErrorCallback>
        struct PropertyData
    {
        // 属性映射表
        PropertyMap<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback> ownPropertyMap;          // 自身属性映射表
        PropertyMap<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback> directPropertyMap;       // 直接属性映射表（O(1)查找）
        std::unordered_map<StringType,
            PropertyMap<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>> parentPropertyMaps; // 父类属性映射表（类名 -> 属性表）
        PropertyMap<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback> combinedPropertyMap;     // 合并后的完整属性表

        // 属性多映射表（允许多个同名属性）
        PropertyMultiMap<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback> allPropertiesMultiMap;

        // 属性列表
        PropertyList<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback> ownPropertiesList;      // 自身属性列表（按注册顺序）
        PropertyList<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback> allPropertiesList;      // 所有属性列表（包括继承的，允许同名）

        // 有序属性名称
        std::vector<KeyType> orderedPropertyNames;                                             // 自身属性按注册顺序的名称列表
        std::unordered_map<StringType, std::vector<KeyType>> parentOrderedPropertyNames;      // 父类有序属性名称列表

        // 父类属性列表映射
        std::unordered_map<StringType,
            PropertyList<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>> parentPropertiesListMap;

        // 父类名称列表（按继承顺序：直接父类在前，最远祖先在后）
        ClassNameList<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback> allParentsName;

        // 新增：选项列表映射（按类名和属性名存储）
        std::unordered_map<StringType, std::unordered_map<KeyType, std::vector<StringType>>> optionalPropertyMap;

        // 新增：描述映射表（按类名和属性名存储）
        std::unordered_map<StringType, std::unordered_map<KeyType, StringType>> descriptionMap;

        // 注册计数器
        size_t registrationCounter = 0;

        // 初始化标志
        bool initialized = false;
    };

    // 可选属性类，继承自Property，提供选项相关功能
    template<typename EnumType, typename KeyType, typename KeyHash, typename KeyEqual,
        typename KeyToString, typename StringType, typename ErrorCallback>
        class OptionalProperty : public Property<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>
    {
    public:
        using BasePropertyType = Property<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>;

        // 默认构造函数 - 创建无效的可选属性
        OptionalProperty() : BasePropertyType()
        {
        }

        OptionalProperty(const BasePropertyType& prop)
            : BasePropertyType(prop)
        {
            // 从PropertyData中获取选项列表
            InitializeOptionList();
        }

        OptionalProperty(const OptionalProperty& other)
            : BasePropertyType(other), m_optionList(other.m_optionList)
        {
        }

        OptionalProperty& operator=(const OptionalProperty& other)
        {
            if (this != &other)
            {
                BasePropertyType::operator=(other);
                m_optionList = other.m_optionList;
            }
            return *this;
        }

        // 获取当前选项的字符串
        StringType GetOptionString() const
        {
            if (!this->IsValid())
                return StringType{};

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

            return StringType{};
        }

        // 获取所有选项列表（当前类+所有父类）
        const std::vector<StringType>& GetOptionList() const
        {
            return m_optionList;
        }

        // 通过字符串设置选项
        bool SetOptionByString(const StringType& optionStr)
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

            const PropertyMeta<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>* meta =
                static_cast<const PropertyMeta<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>*>(this->GetMetaPtr());
            return meta && meta->isOptional;
        }

        // 获取选项数量
        size_t GetOptionCount() const
        {
            return m_optionList.size();
        }

    private:
        // 获取当前属性所属类的选项列表（从PropertyData中查找）
        std::vector<StringType> GetOptionListForThisClass() const
        {
            if (!this->IsValid())
                return {};

            const PropertyMeta<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>* meta =
                static_cast<const PropertyMeta<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>*>(this->GetMetaPtr());
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

            const PropertyMeta<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>* meta =
                static_cast<const PropertyMeta<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>*>(this->GetMetaPtr());
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
        std::vector<StringType> m_optionList; // 缓存的合并后的选项列表
    };

    // ==================== 公共逻辑提取 - 辅助函数 ====================

    // 属性系统工具类
    template<typename EnumType, typename KeyType, typename KeyHash, typename KeyEqual,
        typename KeyToString, typename StringType, typename ErrorCallback>
        class PropertySystemUtils
    {
    public:
        // 注册父类属性到父类映射表
        template<typename ParentClass>
        static void RegisterParentProperties(
            PropertyData<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>& propertyData,
            const StringType& parentClassName)
        {
            auto& parentPropertyData = ParentClass::GetPropertyDataStatic();

            // 复制父类的自身属性
            PropertyMap<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback> parentCopy;
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
        static void BuildAllParentsNameList(
            PropertyData<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>& propertyData,
            const StringType& parentClassName)
        {
            propertyData.allParentsName.clear();

            if constexpr (!std::is_same_v<ParentClass, PropertyObject<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>>)
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
        static void BuildParentPropertiesListMap(
            PropertyData<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>& propertyData)
        {
            propertyData.parentPropertiesListMap.clear();

            // 从父类映射构建有序列表
            for (const auto& classPair : propertyData.parentPropertyMaps)
            {
                const StringType& className = classPair.first;
                const auto& propertyMap = classPair.second;
                auto orderedIt = propertyData.parentOrderedPropertyNames.find(className);

                if (orderedIt != propertyData.parentOrderedPropertyNames.end())
                {
                    // 按照注册顺序构建属性列表
                    PropertyList<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback> orderedList;
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
        static void BuildAllPropertiesList(
            PropertyData<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>& propertyData)
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
        static void InitializePropertyData(
            PropertyData<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>& propertyData)
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

    // 基类模板，带枚举类型参数和键值类型参数
    template<typename EnumType,
        typename KeyType = std::string,
        typename KeyHash = std::hash<KeyType>,
        typename KeyEqual = std::equal_to<KeyType>,
        typename KeyToString = std::function<std::string(const KeyType&)>,
        typename StringType = std::string,
        typename ErrorCallback = DefaultErrorCallback<StringType>>
        class PropertyObject
    {
        friend class Property<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>;
    public:
        using ROPEnumClass = EnumType;
        using ROPKeyType = KeyType;
        using ROPKeyHash = KeyHash;
        using ROPKeyEqual = KeyEqual;
        using ROPKeyToString = KeyToString;
        using ROPStringType = StringType;
        using ROPErrorCallback = ErrorCallback;
        using ROPPropertyDataType = PropertyData<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>;
        using ROPObjectType = PropertyObject<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>;
        using ROPProperty = Property<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>;
        using ROPOptionalProperty = OptionalProperty<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>;

        virtual ~PropertyObject() = default;

        // 获取类名
        virtual StringType GetClassName() const = 0;

        // 获取属性数据结构体（供OptionalProperty使用）
        virtual const PropertyData<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>& GetPropertyData() const = 0;

        static void StaticInitializeProperties() {};

        // 静态错误输出函数
        static void ReportError(const StringType& errorMsg)
        {
            static ROPErrorCallback s_errorCallback = ROPErrorCallback();
            s_errorCallback(errorMsg);
        }

        // 设置错误回调（可选）
        static void SetErrorCallback(const ROPErrorCallback& callback)
        {
            static ROPErrorCallback s_errorCallback = callback;
            s_errorCallback = callback;
        }

        // 公共成员函数 - 通过GetPropertyData()统一访问

        // 获取父类名列表
        const ClassNameList<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>& GetAllParentsName() const
        {
            return GetPropertyData().allParentsName;
        }

        // 获取自身定义的属性（不包含继承的）
        const PropertyList<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>& GetOwnPropertiesList() const
        {
            return GetPropertyData().ownPropertiesList;
        }

        // 获取所有属性（包括继承的，允许多个同名的属性）
        const PropertyMultiMap<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>& GetAllPropertiesMultiMap() const
        {
            return GetPropertyData().allPropertiesMultiMap;
        }

        // 获取父类属性映射表
        const std::unordered_map<StringType,
            PropertyMap<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>>&GetParentPropertiesMap() const
        {
            return GetPropertyData().parentPropertyMaps;
        }

        // 获取直接属性映射（仅包含自身属性，O(1)查找）
        const PropertyMap<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>& GetDirectPropertyMap() const
        {
            return GetPropertyData().directPropertyMap;
        }

        // 获取指定父类的属性列表（有序）
        const PropertyList<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>& GetParentPropertiesList(const StringType& parentClassName) const
        {
            const auto& parentPropertiesListMap = GetPropertyData().parentPropertiesListMap;
            auto it = parentPropertiesListMap.find(parentClassName);
            if (it != parentPropertiesListMap.end())
            {
                return it->second;
            }
            static PropertyList<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback> emptyList;
            return emptyList;
        }

        // 获取自身可用属性列表（包括继承的，允许同名）
        const PropertyList<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>& GetAllPropertiesList() const
        {
            return GetPropertyData().allPropertiesList;
        }

        // 通过名称获取属性包装对象 - 如果有多个同名属性，返回第一个（子类的）
        Property<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback> GetProperty(const KeyType& name) const
        {
            // 优先从直接属性映射中查找（O(1)）
            const auto& directMap = GetDirectPropertyMap();
            auto it = directMap.find(name);
            if (it != directMap.end())
            {
                return Property<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>(
                    it->second.enumType, &it->second, const_cast<PropertyObject<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>*>(this));
            }

            // 如果直接映射中没找到，再从所有属性中查找
            auto& allProps = GetAllPropertiesMultiMap();
            auto range = allProps.equal_range(name);

            if (range.first != range.second)
            {
                return Property<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>(
                    range.first->second.enumType, &range.first->second, const_cast<PropertyObject<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>*>(this));
            }

            // 找不到时返回无效的Property对象
            return Property<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>();
        }

        // 通过名称和类名获取属性包装对象 - 精确查找特定类的属性
        Property<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback> GetProperty(const KeyType& name, const StringType& className) const
        {
            auto& allProps = GetAllPropertiesMultiMap();
            auto range = allProps.equal_range(name);

            for (auto it = range.first; it != range.second; ++it)
            {
                if (it->second.className == className)
                {
                    return Property<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>(
                        it->second.enumType, &it->second, const_cast<PropertyObject<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>*>(this));
                }
            }

            // 找不到时返回无效的Property对象
            return Property<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>();
        }

        // 获取所有同名属性（返回列表）
        std::vector<Property<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>> GetAllPropertiesByName(const KeyType& name) const
        {
            std::vector<Property<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>> result;
            auto& allProps = GetAllPropertiesMultiMap();
            auto range = allProps.equal_range(name);

            for (auto it = range.first; it != range.second; ++it)
            {
                result.push_back(Property<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>(
                    it->second.enumType, &it->second, const_cast<PropertyObject<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>*>(this)));
            }

            return result;
        }

        // 获取所有属性（包括继承的），按顺序（先子类后父类，每个类内按注册顺序）
        std::vector<Property<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>> GetAllPropertiesOrdered() const
        {
            std::vector<Property<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>> result;
            const auto& allPropsList = GetAllPropertiesList();

            for (const auto& meta : allPropsList)
            {
                result.push_back(Property<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>(
                    meta.enumType, &meta, const_cast<PropertyObject<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>*>(this)));
            }

            return result;
        }

    protected:
        // 内部方法：通过属性元数据指针获取属性值
        template<typename T>
        T GetPropertyValue(const void* metaPtr) const
        {
            const PropertyMeta<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>* meta =
                static_cast<const PropertyMeta<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>*>(metaPtr);
            if (!meta)
            {
                ReportError(StringType("Invalid property meta pointer"));
                throw std::runtime_error("Invalid property meta pointer");
            }

            void* ptr = meta->getter(const_cast<PropertyObject<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>*>(this));
            return *reinterpret_cast<T*>(ptr);
        }

        // 内部方法：通过属性元数据指针设置属性值
        template<typename T>
        void SetPropertyValue(const void* metaPtr, const T& value)
        {
            const PropertyMeta<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>* meta =
                static_cast<const PropertyMeta<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>*>(metaPtr);
            if (!meta)
            {
                ReportError(StringType("Invalid property meta pointer"));
                throw std::runtime_error("Invalid property meta pointer");
            }

            T temp = value;
            meta->setter(const_cast<PropertyObject<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>*>(this), &temp);
        }

    public:
        // 查找属性（检查所有属性，包括继承的）
        bool HasProperty(const KeyType& name) const
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
        bool HasProperty(const KeyType& name, const StringType& className) const
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
        PropertyList<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback> GetParentClassProperties(const StringType& parentClassName) const
        {
            PropertyList<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback> result;
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
                [](const PropertyMeta<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>& a,
                    const PropertyMeta<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>& b)
                {
                    return a.registrationOrder < b.registrationOrder;
                });

            return result;
        }

        // 获取指定父类的属性映射表
        PropertyMap<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback> GetParentClassPropertyMap(const StringType& parentClassName) const
        {
            auto& parentMaps = GetParentPropertiesMap();
            auto it = parentMaps.find(parentClassName);
            if (it != parentMaps.end())
            {
                return it->second;
            }
            return PropertyMap<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>();
        }

        // 将Property转换为OptionalProperty
        OptionalProperty<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback> ToOptionalProperty(
            const Property<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>& prop) const
        {
            // 如果属性无效，返回无效的OptionalProperty
            if (!prop.IsValid())
            {
                return OptionalProperty<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>();
            }

            const PropertyMeta<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>* meta =
                static_cast<const PropertyMeta<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>*>(prop.GetMetaPtr());

            if (!meta || !meta->isOptional)
            {
                ReportError(StringType("Property is not an optional property"));
                throw std::runtime_error("Property is not an optional property");
            }

            return OptionalProperty<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>(prop);
        }

        // 通过Property获取OptionalProperty
        OptionalProperty<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback> GetPropertyAsOptional(const KeyType& name) const
        {
            auto prop = GetProperty(name);
            if (!prop.IsValid())
            {
                return OptionalProperty<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>();
            }
            return ToOptionalProperty(prop);
        }

        // 通过名称和类名获取OptionalProperty
        OptionalProperty<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback> GetPropertyAsOptional(const KeyType& name, const StringType& className) const
        {
            auto prop = GetProperty(name, className);
            if (!prop.IsValid())
            {
                return OptionalProperty<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>();
            }
            return ToOptionalProperty(prop);
        }

        // 获取属性的描述
        StringType GetPropertyDescription(const KeyType& name) const
        {
            auto prop = GetProperty(name);
            return prop.GetDescription();
        }

        // 获取属性和描述（返回包含名称和描述的字符串）
        StringType GetPropertyWithDescription(const KeyType& name) const
        {
            auto prop = GetProperty(name);
            if (!prop.IsValid())
            {
                return KeyToString()(name) + StringType(" - [Invalid Property]");
            }

            StringType description = prop.GetDescription();
            if (!description.empty())
            {
                return KeyToString()(name) + StringType(" - ") + description;
            }
            return KeyToString()(name);
        }

        // 获取所有同名属性，按顺序（先子类后父类，每个类内按注册顺序）
        std::vector<Property<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>> GetPropertiesByNameOrdered(const KeyType& name) const
        {
            std::vector<Property<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>> result;
            const auto& allPropsList = GetAllPropertiesList();

            for (const auto& meta : allPropsList)
            {
                KeyEqual equal;
                if (equal(meta.name, name))
                {
                    result.push_back(Property<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>(
                        meta.enumType, &meta, const_cast<PropertyObject<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>*>(this)));
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
        std::vector<KeyType> GetUniquePropertyNames() const
        {
            std::unordered_set<KeyType, KeyHash, KeyEqual> uniqueNames;
            const auto& allPropsList = GetAllPropertiesList();

            for (const auto& meta : allPropsList)
            {
                uniqueNames.insert(meta.name);
            }

            std::vector<KeyType> result(uniqueNames.begin(), uniqueNames.end());
            return result;
        }
    };

    // 属性注册器模板类（支持流式接口）
    template<typename EnumType, typename ClassType, typename KeyType, typename KeyHash, typename KeyEqual,
        typename KeyToString, typename StringType, typename ErrorCallback>
        class PropertyRegistrar
    {
    public:
        PropertyRegistrar(PropertyData<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>& propertyData, const StringType& className)
            : m_propertyData(propertyData), m_className(className)
        {
        }

        // 注册属性（成员变量）- 流式接口（带描述）
        template<typename PropertyType>
        PropertyRegistrar& RegisterProperty(
            EnumType enumType,
            const KeyType& name,
            PropertyType ClassType::* memberPtr,
            const StringType& description = StringType())
        {
            // 计算偏移量 - 使用空指针技巧
            size_t offset = reinterpret_cast<size_t>(
                &(reinterpret_cast<ClassType*>(0)->*memberPtr));

            // 创建getter函数
            std::function<void* (PropertyObject<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>*)> getter =
                [memberPtr](PropertyObject<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>* obj) -> void*
            {
                ClassType* derived = static_cast<ClassType*>(obj);
                return &(derived->*memberPtr);
            };

            // 创建setter函数
            std::function<void(PropertyObject<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>*, void*)> setter =
                [memberPtr](PropertyObject<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>* obj, void* value)
            {
                ClassType* derived = static_cast<ClassType*>(obj);
                derived->*memberPtr = *static_cast<PropertyType*>(value);
            };

            // 创建属性元数据
            PropertyMeta<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback> meta;
            meta.name = name;
            meta.enumType = enumType;
            // 使用类型名转换为StringType
            meta.typeName = StringType(typeid(PropertyType).name());
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
            const KeyType& name,
            void (ClassType::* setterFunc)(PropertyType&),
            PropertyType& (ClassType::* getterFunc)(),
            const StringType& description = StringType())
        {
            // 创建getter函数
            std::function<void* (PropertyObject<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>*)> getter =
                [getterFunc](PropertyObject<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>* obj) -> void*
            {
                ClassType* derived = static_cast<ClassType*>(obj);
                PropertyType& ref = (derived->*getterFunc)();
                return &ref;
            };

            // 创建setter函数
            std::function<void(PropertyObject<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>*, void*)> setter =
                [setterFunc](PropertyObject<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>* obj, void* value)
            {
                ClassType* derived = static_cast<ClassType*>(obj);
                (derived->*setterFunc)(*static_cast<PropertyType*>(value));
            };

            // 创建属性元数据
            PropertyMeta<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback> meta;
            meta.name = name;
            meta.enumType = enumType;
            meta.typeName = StringType(typeid(PropertyType).name());
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
            const KeyType& name,
            PropertyType ClassType::* memberPtr,
            std::initializer_list<const char*> options,
            const StringType& description = StringType())
        {
            // 首先注册普通属性
            RegisterProperty(enumType, name, memberPtr, description);

            // 转换选项列表
            std::vector<StringType> optionVec;
            for (const auto& option : options)
            {
                optionVec.push_back(StringType(option));
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
                std::unordered_set<StringType> optionSet;
                for (const auto& option : optionVec)
                {
                    if (!optionSet.insert(option).second)
                    {
                        // 使用错误回调输出警告
                        StringType warningMsg = StringType("Warning: Duplicate option string '") + option +
                            "' in property '" + KeyToString()(name) +
                            "' of class '" + m_className + "'";
                        ErrorCallback()(warningMsg);
                    }
                }
            }

            return *this;
        }

        // 注册选项属性（自定义getter和setter）- 流式接口（带描述）
        template<typename PropertyType>
        PropertyRegistrar& RegisterOptionalProperty(
            EnumType enumType,
            const KeyType& name,
            void (ClassType::* setterFunc)(PropertyType&),
            PropertyType& (ClassType::* getterFunc)(),
            std::initializer_list<const char*> options,
            const StringType& description = StringType())
        {
            // 首先注册自定义属性
            RegisterProperty(enumType, name, setterFunc, getterFunc, description);

            // 转换选项列表
            std::vector<StringType> optionVec;
            for (const auto& option : options)
            {
                optionVec.push_back(StringType(option));
            }

            // 然后标记为选项属性
            auto& meta = m_propertyData.ownPropertyMap[name];
            meta.isOptional = true;

            // 存储到选项映射中
            m_propertyData.optionalPropertyMap[m_className][name] = optionVec;

            // 验证选项映射值从0开始且连续
            if (!optionVec.empty())
            {
                std::unordered_set<StringType> optionSet;
                for (const auto& option : optionVec)
                {
                    if (!optionSet.insert(option).second)
                    {
                        // 使用错误回调输出警告
                        StringType warningMsg = StringType("Warning: Duplicate option string '") + option +
                            "' in property '" + KeyToString()(name) +
                            "' of class '" + m_className + "'";
                        ErrorCallback()(warningMsg);
                    }
                }
            }

            return *this;
        }

        // 设置已注册属性的描述
        PropertyRegistrar& SetDescription(const KeyType& name, const StringType& description)
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
        PropertyData<EnumType, KeyType, KeyHash, KeyEqual, KeyToString, StringType, ErrorCallback>& m_propertyData;
        StringType m_className;
    };

} // namespace ROP

// ==================== 新的流式注册宏定义 ====================

// 辅助宏：初始化属性系统
#define INIT_PROPERTY_SYSTEM(ClassName, ParentClassName) \
    { \
        auto& propertyData = GetPropertyDataStatic(); \
        if (propertyData.initialized) return true; \
        \
        const ROPStringType classnamestring = ROPStringType(#ClassName); \
        \
        /* 首先注册父类的属性到父类映射表 */ \
        if constexpr (!std::is_same_v<ParentClassName, ROP::PropertyObject<ROPEnumClass, ROPKeyType, ROPKeyHash, ROPKeyEqual, ROPKeyToString, ROPStringType, ROPErrorCallback>>) { \
            ParentClassName::StaticInitializeProperties(); \
            ROP::PropertySystemUtils<ROPEnumClass, ROPKeyType, ROPKeyHash, ROPKeyEqual, ROPKeyToString, ROPStringType, ROPErrorCallback>::RegisterParentProperties<ParentClassName>( \
                propertyData, ROPStringType(#ParentClassName)); \
        } \
        \
        ROPStringType ParentClassNameString = ROPStringType(#ParentClassName);

// 辅助宏：完成属性系统初始化（合并后的版本）
#define FINALIZE_PROPERTY_SYSTEM() \
        /* 构建父类名称列表 */ \
        ROP::PropertySystemUtils<ROPEnumClass, ROPKeyType, ROPKeyHash, ROPKeyEqual, ROPKeyToString, ROPStringType, ROPErrorCallback>::BuildAllParentsNameList<ROPParentClassType>( \
            propertyData, ParentClassNameString); \
        \
        /* 构建父类属性列表映射 */ \
        ROP::PropertySystemUtils<ROPEnumClass, ROPKeyType, ROPKeyHash, ROPKeyEqual, ROPKeyToString, ROPStringType, ROPErrorCallback>::BuildParentPropertiesListMap(propertyData); \
        \
        /* 使用合并函数初始化属性数据 */ \
        ROP::PropertySystemUtils<ROPEnumClass, ROPKeyType, ROPKeyHash, ROPKeyEqual, ROPKeyToString, ROPStringType, ROPErrorCallback>::InitializePropertyData(propertyData); \
        \
        /* 构建所有属性列表（包括父类，允许同名） */ \
        ROP::PropertySystemUtils<ROPEnumClass, ROPKeyType, ROPKeyHash, ROPKeyEqual, ROPKeyToString, ROPStringType, ROPErrorCallback>::BuildAllPropertiesList(propertyData); \
        \
        propertyData.initialized = true; \
        return true; \
    }

// 主宏：声明并定义所有必要函数
#define DECLARE_OBJECT_WITH_PARENT(ClassName, ParentClassName) \
public:\
    virtual ROPStringType GetClassName() const override { \
        return ROPStringType(#ClassName); \
    } \
    virtual const ROPPropertyDataType& GetPropertyData() const override { \
        EnsurePropertySystemInitialized(); \
        return GetPropertyDataStatic(); \
    } \
    /* 获取属性数据结构体（静态版本） */ \
    static ROPPropertyDataType& GetPropertyDataStatic() { \
        static ROPPropertyDataType s_propertyData; \
        return s_propertyData; \
    } \
protected: \
    using ROPClassType = ClassName;\
    using ROPParentClassType = ParentClassName;\
    \
    /* 静态初始化函数 */ \
    static bool StaticInitializeProperties() { \
        static bool s_initialized = []() -> bool { \
            INIT_PROPERTY_SYSTEM(ClassName, ParentClassName) \
            \
            /* 创建注册器对象 */ \
            ROP::PropertyRegistrar<ROPEnumClass, ClassName, ROPKeyType, ROPKeyHash, ROPKeyEqual, ROPKeyToString, ROPStringType, ROPErrorCallback> \
                registrar(propertyData, classnamestring); 

// 简化宏：用于没有父类的情况
#define DECLARE_OBJECT(ClassName) DECLARE_OBJECT_WITH_PARENT(ClassName, ROPObjectType)

// 结束宏
#define END_DECLARE_OBJECT() \
            /* 完成属性系统初始化 */ \
            FINALIZE_PROPERTY_SYSTEM() \
        }(); \
        return s_initialized; \
    } \
    \
    /* 确保属性系统已初始化 */ \
    void EnsurePropertySystemInitialized() const { \
        static_cast<const ROPClassType*>(this)->StaticInitializeProperties(); \
    } \
    \
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
//    DECLARE_OBJECT(Device)
//    registrar
//        .RegisterProperty(DeviceProperty::ID, "deviceId", &Device::deviceId, "设备ID")
//        .RegisterProperty(DeviceProperty::NAME, "deviceName", &Device::deviceName, "设备名称")
//        .RegisterOptionalProperty(
//            DeviceProperty::OPTIONAL, "status", &Device::status,
//            { "Offline", "Online", "Error", "Maintenance" },
//            "设备状态");
//    END_DECLARE_OBJECT()
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
//    DECLARE_OBJECT_WITH_PARENT(TemperatureSensor, Device)
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
//    END_DECLARE_OBJECT()
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