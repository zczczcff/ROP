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

namespace ROP
{

    // 前向声明
    template<typename EnumType>
    class PropertyObject;

    // 属性模板类，包装属性和其枚举类型
    template<typename EnumType>
    class Property
    {
    public:
        Property(const EnumType type, const void* metaPtr, PropertyObject<EnumType>* objPtr)
            : m_type(type), m_metaPtr(metaPtr), m_objPtr(objPtr)
        {
        }

        // 获取属性枚举类型
        EnumType GetType() const { return m_type; }

        // 获取属性值（指定类型）
        template<typename T>
        T GetValue() const
        {
            if (!m_objPtr) throw std::runtime_error("Invalid property object");
            return m_objPtr->template GetPropertyValue<T>(m_metaPtr);
        }

        // 设置属性值（指定类型）
        template<typename T>
        void SetValue(const T& value)
        {
            if (!m_objPtr) throw std::runtime_error("Invalid property object");
            m_objPtr->template SetPropertyValue<T>(m_metaPtr, value);
        }

        // 获取属性元数据指针
        const void* GetMetaPtr() const { return m_metaPtr; }

        // 获取所属对象
        PropertyObject<EnumType>* GetObject() const { return m_objPtr; }

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

        bool operator==(const PropertyMeta& other) const
        {
            return name == other.name && className == other.className;
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
                (std::hash<std::string>()(prop.className) << 1);
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
        PropertyList<EnumType> propertiesList;                             // 所有属性列表（按注册顺序）
        PropertyList<EnumType> ownPropertiesList;                          // 自身属性列表（按注册顺序）
        PropertyList<EnumType> allPropertiesList;                          // 所有属性列表（包括继承的）

        // 有序属性名称
        std::vector<std::string> orderedPropertyNames;                     // 自身属性按注册顺序的名称列表
        std::unordered_map<std::string, std::vector<std::string>> parentOrderedPropertyNames; // 父类有序属性名称列表

        // 父类属性列表映射
        std::unordered_map<std::string, PropertyList<EnumType>> parentPropertiesListMap;

        // 父类名称列表
        ClassNameList<EnumType> allParentsName;

        // 新增：选项列表映射（按类名和属性名存储）
        std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> optionalPropertyMap;

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

        // 构建所有属性列表（包括父类）
        static void BuildAllPropertiesList(PropertyData<EnumType>& propertyData)
        {
            propertyData.allPropertiesList.clear();

            // 获取所有父类名称（按继承顺序）
            auto& allParentsName = propertyData.allParentsName;

            // 首先添加所有父类的属性
            for (const auto& parentClassName : allParentsName)
            {
                auto it = propertyData.parentPropertiesListMap.find(parentClassName);
                if (it != propertyData.parentPropertiesListMap.end())
                {
                    for (const auto& prop : it->second)
                    {
                        propertyData.allPropertiesList.push_back(prop);
                    }
                }
            }

            // 然后添加自己的属性（会覆盖父类的同名属性）
            for (const auto& prop : propertyData.ownPropertiesList)
            {
                bool found = false;
                // 查找是否已有同名属性
                for (auto& existingProp : propertyData.allPropertiesList)
                {
                    if (existingProp.name == prop.name)
                    {
                        existingProp = prop; // 替换为子类的属性
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    propertyData.allPropertiesList.push_back(prop);
                }
            }
        }

        // 合并所有属性到完整属性表
        static void MergeAllProperties(PropertyData<EnumType>& propertyData)
        {
            propertyData.combinedPropertyMap.clear();

            // 首先添加所有父类的属性
            for (const auto& classPair : propertyData.parentPropertyMaps)
            {
                for (const auto& propPair : classPair.second)
                {
                    propertyData.combinedPropertyMap[propPair.first] = propPair.second;
                }
            }

            // 然后添加自己的属性（会覆盖父类的同名属性）
            for (const auto& propPair : propertyData.ownPropertyMap)
            {
                propertyData.combinedPropertyMap[propPair.first] = propPair.second;
            }
        }

        // 合并所有属性到多映射表（允许多个同名属性）
        static void MergeAllPropertiesMulti(PropertyData<EnumType>& propertyData)
        {
            propertyData.allPropertiesMultiMap.clear();

            // 首先添加所有父类的属性
            for (const auto& classPair : propertyData.parentPropertyMaps)
            {
                for (const auto& propPair : classPair.second)
                {
                    propertyData.allPropertiesMultiMap.insert({ propPair.first, propPair.second });
                }
            }

            // 然后添加自己的属性
            for (const auto& propPair : propertyData.ownPropertyMap)
            {
                propertyData.allPropertiesMultiMap.insert({ propPair.first, propPair.second });
            }
        }

        // 初始化直接属性映射
        static void InitDirectPropertyMap(PropertyData<EnumType>& propertyData)
        {
            propertyData.directPropertyMap.clear();

            // 只添加自己的属性
            for (const auto& propPair : propertyData.ownPropertyMap)
            {
                propertyData.directPropertyMap[propPair.first] = propPair.second;
            }
        }
    };

    // 基类模板，带枚举类型参数
    template<typename EnumType>
    class PropertyObject
    {
    public:
        using EnumClass = EnumType;
        using PropertyDataType = PropertyData<EnumType>;

        virtual ~PropertyObject() = default;

        // 虚函数：获取自身可用属性列表（包括继承的）
        virtual const PropertyList<EnumType>& GetAllPropertiesList() const = 0;

        // 获取类名
        virtual std::string GetClassName() const = 0;

        // 获取父类名列表
        virtual const ClassNameList<EnumType>& GetAllParentsName() const = 0;

        // 获取自身定义的属性（不包含继承的）
        virtual const PropertyList<EnumType>& GetOwnPropertiesList() const = 0;

        // 获取所有属性（包括继承的，允许多个同名的属性）
        virtual const PropertyMultiMap<EnumType>& GetAllPropertiesMultiMap() const = 0;

        // 获取父类属性映射表
        virtual const std::unordered_map<std::string, PropertyMap<EnumType>>& GetParentPropertiesMap() const = 0;

        // 获取直接属性映射（仅包含自身属性，O(1)查找）
        virtual const PropertyMap<EnumType>& GetDirectPropertyMap() const = 0;

        // 获取指定父类的属性列表（有序）
        virtual const PropertyList<EnumType>& GetParentPropertiesList(const std::string& parentClassName) const = 0;

        // 获取属性数据结构体（供OptionalProperty使用）
        virtual const PropertyData<EnumType>& GetPropertyData() const = 0;

        static void StaticInitializeProperties() {};

        // 通过名称获取属性包装对象
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

            throw std::runtime_error("Property not found: " + name);
        }

        // 通过名称和类名获取属性包装对象
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
            throw std::runtime_error("Property not found: " + name + " in class " + className);
        }

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

        // 通过名称获取属性值（兼容旧代码）
        template<typename T>
        T GetPropertyValueByName(const std::string& name) const
        {
            // 优先从直接属性映射中查找（O(1)）
            const auto& directMap = GetDirectPropertyMap();
            auto it = directMap.find(name);
            if (it != directMap.end())
            {
                void* ptr = it->second.getter(const_cast<PropertyObject<EnumType>*>(this));
                return *reinterpret_cast<T*>(ptr);
            }

            // 如果直接映射中没找到，再从所有属性中查找
            auto& allProps = GetAllPropertiesMultiMap();
            auto range = allProps.equal_range(name);

            if (range.first != range.second)
            {
                void* ptr = range.first->second.getter(const_cast<PropertyObject<EnumType>*>(this));
                return *reinterpret_cast<T*>(ptr);
            }

            throw std::runtime_error("Property not found: " + name);
        }

        // 通过名称设置属性值（兼容旧代码）
        template<typename T>
        void SetPropertyValueByName(const std::string& name, const T& value)
        {
            // 优先从直接属性映射中查找（O(1)）
            const auto& directMap = GetDirectPropertyMap();
            auto it = directMap.find(name);
            if (it != directMap.end())
            {
                T temp = value;
                it->second.setter(const_cast<PropertyObject<EnumType>*>(this), &temp);
                return;
            }

            // 如果直接映射中没找到，再从所有属性中查找
            auto& allProps = GetAllPropertiesMultiMap();
            auto range = allProps.equal_range(name);

            if (range.first != range.second)
            {
                T temp = value;
                range.first->second.setter(const_cast<PropertyObject<EnumType>*>(this), &temp);
                return;
            }

            throw std::runtime_error("Property not found: " + name);
        }

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
            return ToOptionalProperty(prop);
        }

        // 通过名称和类名获取OptionalProperty
        OptionalProperty<EnumType> GetPropertyAsOptional(const std::string& name, const std::string& className) const
        {
            auto prop = GetProperty(name, className);
            return ToOptionalProperty(prop);
        }
    };

    // 属性注册器模板类
    template<typename EnumType, typename ClassType>
    class PropertyRegistrar
    {
    public:
        // 注册属性（成员变量）
        template<typename PropertyType>
        static void RegisterProperty(
            EnumType enumType,
            const std::string& name,
            PropertyType ClassType::* memberPtr,
            PropertyData<EnumType>& propertyData,
            const std::string& className)
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
            meta.className = className;
            meta.getter = getter;
            meta.setter = setter;
            meta.isCustomAccessor = false;
            meta.registrationOrder = propertyData.registrationCounter++;

            // 记录注册顺序
            propertyData.orderedPropertyNames.push_back(name);

            // 注册到属性容器
            propertyData.ownPropertyMap[name] = meta;
        }

        // 注册属性（自定义getter和setter）
        template<typename PropertyType>
        static void RegisterPropertyEx(
            EnumType enumType,
            const std::string& name,
            void (ClassType::* setterFunc)(PropertyType&),
            PropertyType& (ClassType::* getterFunc)(),
            PropertyData<EnumType>& propertyData,
            const std::string& className)
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
            meta.className = className;
            meta.getter = getter;
            meta.setter = setter;
            meta.isCustomAccessor = true;
            meta.registrationOrder = propertyData.registrationCounter++;

            // 记录注册顺序
            propertyData.orderedPropertyNames.push_back(name);

            // 注册到属性容器
            propertyData.ownPropertyMap[name] = meta;
        }

        // 注册选项属性（成员变量）
        template<typename PropertyType>
        static void RegisterOptionalProperty(
            EnumType enumType,
            const std::string& name,
            PropertyType ClassType::* memberPtr,
            PropertyData<EnumType>& propertyData,
            const std::string& className,
            const std::vector<std::string>& options)
        {
            // 首先注册普通属性
            RegisterProperty(enumType, name, memberPtr, propertyData, className);

            // 然后标记为选项属性
            auto& meta = propertyData.ownPropertyMap[name];
            meta.isOptional = true;

            // 存储到选项映射中
            propertyData.optionalPropertyMap[className][name] = options;

            // 验证选项映射值从0开始且连续
            // 由于我们使用vector<string>，索引就是0,1,2...，天然满足条件
            // 这里可以添加验证逻辑（可选）
            if (!options.empty())
            {
                // 检查是否有重复的选项字符串
                std::unordered_set<std::string> optionSet;
                for (const auto& option : options)
                {
                    if (!optionSet.insert(option).second)
                    {
                        std::cerr << "Warning: Duplicate option string '" << option
                            << "' in property '" << name << "' of class '" << className << "'" << std::endl;
                    }
                }
            }
        }

        // 注册选项属性（自定义getter和setter）
        template<typename PropertyType>
        static void RegisterOptionalPropertyEx(
            EnumType enumType,
            const std::string& name,
            void (ClassType::* setterFunc)(PropertyType&),
            PropertyType& (ClassType::* getterFunc)(),
            PropertyData<EnumType>& propertyData,
            const std::string& className,
            const std::vector<std::string>& options)
        {
            // 首先注册自定义属性
            RegisterPropertyEx(enumType, name, setterFunc, getterFunc, propertyData, className);

            // 然后标记为选项属性
            auto& meta = propertyData.ownPropertyMap[name];
            meta.isOptional = true;

            // 存储到选项映射中
            propertyData.optionalPropertyMap[className][name] = options;

            // 验证选项映射值从0开始且连续（同上）
            if (!options.empty())
            {
                std::unordered_set<std::string> optionSet;
                for (const auto& option : options)
                {
                    if (!optionSet.insert(option).second)
                    {
                        std::cerr << "Warning: Duplicate option string '" << option
                            << "' in property '" << name << "' of class '" << className << "'" << std::endl;
                    }
                }
            }
        }

        // 获取类的属性列表（按照注册顺序）
        static const PropertyList<EnumType>& GetClassProperties(const PropertyMap<EnumType>& propertyMap,
            const std::vector<std::string>& orderedNames)
        {
            static PropertyList<EnumType> properties;
            properties.clear();
            properties.reserve(propertyMap.size());

            // 按照注册顺序添加属性
            for (const auto& name : orderedNames)
            {
                auto it = propertyMap.find(name);
                if (it != propertyMap.end())
                {
                    properties.push_back(it->second);
                }
            }

            return properties;
        }

        // 获取类的属性多映射
        static PropertyMultiMap<EnumType> GetClassPropertiesMulti(const PropertyMap<EnumType>& propertyMap)
        {
            PropertyMultiMap<EnumType> multiMap;

            for (const auto& pair : propertyMap)
            {
                multiMap.insert({ pair.first, pair.second });
            }

            return multiMap;
        }
    };

} // namespace ROP

// ==================== 改进的宏定义 ====================

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
        std::string ParentClassNameString = #ParentClassName; \

// 辅助宏：完成属性系统初始化
#define FINALIZE_PROPERTY_SYSTEM(EnumType, ClassName) \
        /* 初始化直接属性映射 */ \
        ROP::PropertySystemUtils<EnumType>::InitDirectPropertyMap(propertyData); \
        \
        /* 合并所有属性 */ \
        ROP::PropertySystemUtils<EnumType>::MergeAllProperties(propertyData); \
        ROP::PropertySystemUtils<EnumType>::MergeAllPropertiesMulti(propertyData); \
        \
        /* 按照注册顺序初始化属性列表 */ \
        propertyData.propertiesList = ROP::PropertyRegistrar<EnumType, ClassName>::GetClassProperties( \
            propertyData.combinedPropertyMap, propertyData.orderedPropertyNames); \
        propertyData.ownPropertiesList = ROP::PropertyRegistrar<EnumType, ClassName>::GetClassProperties( \
            propertyData.ownPropertyMap, propertyData.orderedPropertyNames); \
        \
        /* 构建父类名称列表 */ \
        ROP::PropertySystemUtils<EnumType>::BuildAllParentsNameList<ParentClass>( \
            propertyData, ParentClassNameString); \
        \
        /* 构建父类属性列表映射 */ \
        ROP::PropertySystemUtils<EnumType>::BuildParentPropertiesListMap(propertyData); \
        \
        /* 构建所有属性列表（包括父类） */ \
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
            INIT_PROPERTY_SYSTEM(EnumType, ClassName, ParentClassName)

// 简化宏：用于没有父类的情况
#define DECLARE_OBJECT(EnumType, ClassName) DECLARE_OBJECT_WITH_PARENT(EnumType, ClassName, ROP::PropertyObject<EnumType>)

// 注册属性宏（在StaticInitializeProperties函数内使用）
#define REGISTER_PROPERTY(EnumValue, Type, Name) \
            ROP::PropertyRegistrar<EnumClass, CurrentClass>:: \
            RegisterProperty<Type>(EnumValue, #Name, &CurrentClass::Name, propertyData, classnamestring);

// 注册自定义getter/setter属性宏
#define REGISTER_PROPERTY_EX(EnumValue, Type, SetterFunc, GetterFunc) \
            ROP::PropertyRegistrar<EnumClass, CurrentClass>:: \
            RegisterPropertyEx<Type>(EnumValue, #GetterFunc, &CurrentClass::SetterFunc, &CurrentClass::GetterFunc, propertyData, classnamestring);

// 注册选项属性宏（通用版本，支持任何可以转换为int的类型）
#define REGISTER_OPTIONAL_PROPERTY(EnumValue, ValueType, PropertyName, ...) \
    do { \
        /* 获取字符串选项列表 */ \
        std::vector<std::string> optionStrs = {__VA_ARGS__}; \
        \
        /* 注册属性 */ \
        ROP::PropertyRegistrar<EnumClass, CurrentClass>:: \
        RegisterOptionalProperty<ValueType>( \
            EnumValue, #PropertyName, &CurrentClass::PropertyName, propertyData, classnamestring, optionStrs); \
    } while(0);

// 注册自定义getter/setter的选项属性宏
#define REGISTER_OPTIONAL_PROPERTY_EX(EnumValue, ValueType, SetterFunc, GetterFunc, ...) \
    do { \
        /* 获取字符串选项列表 */ \
        std::vector<std::string> optionStrs = {__VA_ARGS__}; \
        \
        /* 注册属性 */ \
        ROP::PropertyRegistrar<EnumClass, CurrentClass>:: \
        RegisterOptionalPropertyEx<ValueType>( \
            EnumValue, #GetterFunc, &CurrentClass::SetterFunc, &CurrentClass::GetterFunc, propertyData, classnamestring, optionStrs); \
    } while(0);

// 结束宏
#define END_DECLARE_OBJECT(EnumType, ClassName, ParentClassName) \
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
    \
    virtual const ROP::ClassNameList<EnumType>& GetAllParentsName() const override { \
        EnsurePropertySystemInitialized(); \
        return GetPropertyDataStatic().allParentsName; \
    } \
    \
    virtual const ROP::PropertyList<EnumType>& GetOwnPropertiesList() const override { \
        EnsurePropertySystemInitialized(); \
        return GetPropertyDataStatic().ownPropertiesList; \
    } \
    \
    virtual const ROP::PropertyList<EnumType>& GetAllPropertiesList() const override { \
        EnsurePropertySystemInitialized(); \
        return GetPropertyDataStatic().allPropertiesList; \
    } \
    \
    virtual const ROP::PropertyMultiMap<EnumType>& GetAllPropertiesMultiMap() const override { \
        EnsurePropertySystemInitialized(); \
        return GetPropertyDataStatic().allPropertiesMultiMap; \
    } \
    \
    virtual const std::unordered_map<std::string, ROP::PropertyMap<EnumType>>& GetParentPropertiesMap() const override { \
        EnsurePropertySystemInitialized(); \
        return GetPropertyDataStatic().parentPropertyMaps; \
    } \
    \
    virtual const ROP::PropertyMap<EnumType>& GetDirectPropertyMap() const override { \
        EnsurePropertySystemInitialized(); \
        return GetPropertyDataStatic().directPropertyMap; \
    } \
    \
    virtual const ROP::PropertyList<EnumType>& GetParentPropertiesList(const std::string& parentClassName) const override { \
        EnsurePropertySystemInitialized(); \
        auto& parentPropertiesListMap = GetPropertyDataStatic().parentPropertiesListMap; \
        auto it = parentPropertiesListMap.find(parentClassName); \
        if (it != parentPropertiesListMap.end()) { \
            return it->second; \
        } \
        static ROP::PropertyList<EnumType> emptyList; \
        return emptyList; \
    } \
private:

// ==================== 使用示例 ====================
/*
// 定义属性枚举类型
enum class MyObjectType
{
    OPTIONAL,
    INT,
    FLOAT,
    DOUBLE,
    STRING,
    BOOL,
    VECTOR3,
    COLOR,
    CUSTOM_TYPE
};

// 基类
class BaseObject : public ROP::PropertyObject<MyObjectType>
{
    DECLARE_OBJECT(MyObjectType, BaseObject)
    REGISTER_OPTIONAL_PROPERTY(MyObjectType::OPTIONAL, int, mode, "Off", "On", "Auto")
    REGISTER_PROPERTY(MyObjectType::INT, int, value)
    REGISTER_PROPERTY(MyObjectType::STRING, std::string, tag)
    END_DECLARE_OBJECT(MyObjectType, BaseObject, ROP::PropertyObject<MyObjectType>)

public:
    BaseObject() : mode(0), value(0) {}

    int mode; // 0: Off, 1: On, 2: Auto
    int value;
    std::string tag;
};

// 派生类
class DerivedObject : public BaseObject
{
    DECLARE_OBJECT_WITH_PARENT(MyObjectType, DerivedObject, BaseObject)
    REGISTER_OPTIONAL_PROPERTY(MyObjectType::OPTIONAL, int, mode, "Disabled", "Enabled", "Super") // 重写父类的mode
    REGISTER_OPTIONAL_PROPERTY(MyObjectType::OPTIONAL, int, level, "Low", "Medium", "High")
    END_DECLARE_OBJECT(MyObjectType, DerivedObject, BaseObject)

public:
    DerivedObject() : mode(0), level(0) {}

    int mode;   // 0: Disabled, 1: Enabled, 2: Super (重写父类)
    int level;  // 0: Low, 1: Medium, 2: High
};

// 测试函数
void TestOptionalPropertySystem()
{
    std::cout << "\n=== 测试选项属性系统 ===" << std::endl;

    DerivedObject obj;
    obj.mode = 1; // Enabled
    obj.level = 2; // High
    obj.value = 100;
    obj.tag = "Test";

    // 测试获取可选属性
    std::cout << "\n1. 获取DerivedObject的mode属性:" << std::endl;
    auto modeProp = obj.GetPropertyAsOptional("mode");
    std::cout << "Mode string: " << modeProp.GetOptionString()
              << " (value: " << modeProp.GetValue<int>() << ")" << std::endl;

    std::cout << "Option list (" << modeProp.GetOptionCount() << " options): ";
    auto options = modeProp.GetOptionList();
    for (size_t i = 0; i < options.size(); ++i)
    {
        std::cout << i << ":" << options[i] << " ";
    }
    std::cout << std::endl;

    // 测试通过字符串设置选项
    std::cout << "\n2. 通过字符串设置mode属性:" << std::endl;
    if (modeProp.SetOptionByString("Super"))
    {
        std::cout << "Mode changed to: " << modeProp.GetOptionString()
                  << " (value: " << obj.mode << ")" << std::endl;
    }

    // 测试获取父类的原始属性
    std::cout << "\n3. 获取BaseObject的mode属性:" << std::endl;
    auto baseModeProp = obj.GetPropertyAsOptional("mode", "BaseObject");
    std::cout << "Base mode string: " << baseModeProp.GetOptionString()
              << " (value: " << baseModeProp.GetValue<int>() << ")" << std::endl;

    std::cout << "Base option list: ";
    auto baseOptions = baseModeProp.GetOptionList();
    for (size_t i = 0; i < baseOptions.size(); ++i)
    {
        std::cout << i << ":" << baseOptions[i] << " ";
    }
    std::cout << std::endl;

    // 测试自定义属性
    std::cout << "\n4. 获取level属性:" << std::endl;
    auto levelProp = obj.GetPropertyAsOptional("level");
    std::cout << "Level string: " << levelProp.GetOptionString()
              << " (value: " << levelProp.GetValue<int>() << ")" << std::endl;

    // 测试非选项属性的转换
    std::cout << "\n5. 测试非选项属性的转换:" << std::endl;
    try
    {
        auto valueProp = obj.GetPropertyAsOptional("value");
        std::cout << "This should not print" << std::endl;
    }
    catch (const std::runtime_error& e)
    {
        std::cout << "Error correctly caught: " << e.what() << std::endl;
    }

    // 测试通过索引设置选项
    std::cout << "\n6. 通过索引设置level属性:" << std::endl;
    if (levelProp.SetOptionByIndex(0)) // 设置为Low
    {
        std::cout << "Level changed to: " << levelProp.GetOptionString()
                  << " (value: " << obj.level << ")" << std::endl;
    }

    // 测试Property和OptionalProperty的转换
    std::cout << "\n7. 测试Property和OptionalProperty的转换:" << std::endl;
    // 先获取普通Property
    auto normalProp = obj.GetProperty("level");
    std::cout << "Normal property value: " << normalProp.GetValue<int>() << std::endl;

    // 转换为OptionalProperty
    auto optionalProp = obj.ToOptionalProperty(normalProp);
    std::cout << "Converted to optional property, string: " << optionalProp.GetOptionString() << std::endl;
}
*/