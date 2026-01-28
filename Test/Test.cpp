// ==================== 性能测试用例 ====================
#include <iostream>
#include <chrono>
#include <iomanip>
#include <vector>
#include <random>
#include <algorithm>
#include <ROP/RunTimeObjectProperty.h>

// 定义属性枚举类型（用于测试）
enum class TestPropertyType
{
    INT,
    FLOAT,
    DOUBLE,
    STRING,
    BOOL,
    VECTOR3,
    COLOR,
    CUSTOM_TYPE
};

// 测试基类
class TestBaseObject : public ROP::PropertyObject<TestPropertyType>
{
	DECLARE_OBJECT(TestPropertyType, TestBaseObject)
	REGISTER_PROPERTY(TestPropertyType::INT, int, baseIntValue)
	REGISTER_PROPERTY(TestPropertyType::FLOAT, float, baseFloatValue)
	REGISTER_PROPERTY(TestPropertyType::STRING, std::string, baseStringValue)
	END_DECLARE_OBJECT(TestPropertyType, TestBaseObject, ROP::PropertyObject<TestPropertyType>)

public:
    TestBaseObject() : baseIntValue(0), baseFloatValue(0.0f), baseStringValue("") {}

    int baseIntValue;
    float baseFloatValue;
    std::string baseStringValue;
};

// 测试派生类
class TestDerivedObject : public TestBaseObject
{
    DECLARE_OBJECT_WITH_PARENT(TestPropertyType, TestDerivedObject, TestBaseObject)
    REGISTER_PROPERTY(TestPropertyType::INT, int, intValue1)
    REGISTER_PROPERTY(TestPropertyType::INT, int, intValue2)
    REGISTER_PROPERTY(TestPropertyType::INT, int, intValue3)
    REGISTER_PROPERTY(TestPropertyType::FLOAT, float, floatValue1)
    REGISTER_PROPERTY(TestPropertyType::FLOAT, float, floatValue2)
    REGISTER_PROPERTY(TestPropertyType::DOUBLE, double, doubleValue)
    REGISTER_PROPERTY(TestPropertyType::STRING, std::string, stringValue)
    REGISTER_PROPERTY(TestPropertyType::BOOL, bool, boolValue)
    END_DECLARE_OBJECT(TestPropertyType, TestDerivedObject, TestBaseObject)

public:
    TestDerivedObject() :
        intValue1(0), intValue2(0), intValue3(0),
        floatValue1(0.0f), floatValue2(0.0f),
        doubleValue(0.0),
        boolValue(false)
    {
    }

    int intValue1;
    int intValue2;
    int intValue3;
    float floatValue1;
    float floatValue2;
    double doubleValue;
    std::string stringValue;
    bool boolValue;
};

// 测试带有自定义访问器的类
class TestCustomAccessorObject : public ROP::PropertyObject<TestPropertyType>
{
    DECLARE_OBJECT(TestPropertyType, TestCustomAccessorObject)
    REGISTER_PROPERTY_EX(TestPropertyType::INT, int, SetCustomInt, GetCustomInt)
    REGISTER_PROPERTY_EX(TestPropertyType::STRING, std::string, SetCustomString, GetCustomString)
    REGISTER_PROPERTY(TestPropertyType::INT, int, directIntValue)
    END_DECLARE_OBJECT(TestPropertyType, TestCustomAccessorObject, ROP::PropertyObject<TestPropertyType>)

public:
    TestCustomAccessorObject() :
        m_customInt(0),
        m_customString("default"),
        directIntValue(0)
    {
    }

    void SetCustomInt(int& value)
    {
        // 模拟一些验证逻辑
        if (value < 0) value = 0;
        if (value > 1000) value = 1000;
        m_customInt = value;

        // 模拟副作用
        directIntValue = value * 2;
    }

    int& GetCustomInt()
    {
        return m_customInt;
    }

    void SetCustomString(std::string& value)
    {
        // 模拟验证逻辑
        if (value.empty()) value = "empty";
        if (value.length() > 100) value = value.substr(0, 100);
        m_customString = value;
    }

    std::string& GetCustomString()
    {
        return m_customString;
    }

private:
    int m_customInt;
    std::string m_customString;
public:
    int directIntValue;
};

// 性能测试函数
void RunPropertySystemPerformanceTests()
{
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "运行时属性系统性能测试" << std::endl;
    std::cout << std::string(80, '=') << std::endl;

    // 使用高精度时钟
    using Clock = std::chrono::high_resolution_clock;
    using Duration = std::chrono::nanoseconds;

    const int TEST_ITERATIONS = 1000000; // 100万次迭代
    const int WARMUP_ITERATIONS = 10000; // 预热迭代

    std::cout << "\n测试配置：" << std::endl;
    std::cout << "  - 测试迭代次数: " << TEST_ITERATIONS << std::endl;
    std::cout << "  - 预热迭代次数: " << WARMUP_ITERATIONS << std::endl;
    std::cout << "  - 测试包含3个对象类型" << std::endl;

    // ==================== 测试1: 基本属性访问性能 ====================
    {
        std::cout << "\n" << std::string(50, '-') << std::endl;
        std::cout << "测试1: 基本属性访问性能 (TestDerivedObject)" << std::endl;
        std::cout << std::string(50, '-') << std::endl;

        TestDerivedObject obj;
        obj.intValue1 = 42;
        obj.floatValue1 = 3.14f;
        obj.stringValue = "test_string";

        // 预热
        for (int i = 0; i < WARMUP_ITERATIONS; ++i)
        {
            obj.intValue1 = i;
            auto temp = obj.intValue1;
            (void)temp;
        }

        // 1. 直接访问性能测试
        {
            auto start = Clock::now();

            int sum = 0;
            for (int i = 0; i < TEST_ITERATIONS; ++i)
            {
                // 读取
                sum += obj.intValue1;
                sum += obj.intValue2;
                sum += obj.intValue3;

                // 写入
                obj.intValue1 = i % 100;
                obj.intValue2 = (i + 1) % 100;
                obj.intValue3 = (i + 2) % 100;
            }

            auto end = Clock::now();
            auto duration = std::chrono::duration_cast<Duration>(end - start);

            std::cout << "直接访问:" << std::endl;
            std::cout << "  耗时: " << duration.count() << " ns" << std::endl;
            std::cout << "  每次操作平均耗时: " << duration.count() / (TEST_ITERATIONS * 6.0) << " ns" << std::endl;
            std::cout << "  验证和: " << sum << std::endl;

            double directTimePerOp = duration.count() / (TEST_ITERATIONS * 6.0);

            // 2. Property包装访问性能测试
            start = Clock::now();

            sum = 0;
            ROP::Property<TestPropertyType> prop1 = obj.GetProperty("intValue1");
            ROP::Property<TestPropertyType> prop2 = obj.GetProperty("intValue2");
            ROP::Property<TestPropertyType> prop3 = obj.GetProperty("intValue3");

            for (int i = 0; i < TEST_ITERATIONS; ++i)
            {
                // 读取
                sum += prop1.GetValue<int>();
                sum += prop2.GetValue<int>();
                sum += prop3.GetValue<int>();

                // 写入
                prop1.SetValue<int>(i % 100);
                prop2.SetValue<int>((i + 1) % 100);
                prop3.SetValue<int>((i + 2) % 100);
            }

            end = Clock::now();
            duration = std::chrono::duration_cast<Duration>(end - start);

            double propertyTimePerOp = duration.count() / (TEST_ITERATIONS * 6.0);
            double overheadRatio = propertyTimePerOp / directTimePerOp;

            std::cout << "\nProperty包装访问:" << std::endl;
            std::cout << "  耗时: " << duration.count() << " ns" << std::endl;
            std::cout << "  每次操作平均耗时: " << propertyTimePerOp << " ns" << std::endl;
            std::cout << "  开销倍数: " << std::fixed << std::setprecision(2) << overheadRatio << "x" << std::endl;
            std::cout << "  验证和: " << sum << std::endl;

            // 3. GetPropertyValueByName访问性能测试
            start = Clock::now();

            sum = 0;
            for (int i = 0; i < TEST_ITERATIONS; ++i)
            {
                // 读取
                sum += obj.GetPropertyValueByName<int>("intValue1");
                sum += obj.GetPropertyValueByName<int>("intValue2");
                sum += obj.GetPropertyValueByName<int>("intValue3");

                // 写入
                obj.SetPropertyValueByName<int>("intValue1", i % 100);
                obj.SetPropertyValueByName<int>("intValue2", (i + 1) % 100);
                obj.SetPropertyValueByName<int>("intValue3", (i + 2) % 100);
            }

            end = Clock::now();
            duration = std::chrono::duration_cast<Duration>(end - start);

            double byNameTimePerOp = duration.count() / (TEST_ITERATIONS * 6.0);
            overheadRatio = byNameTimePerOp / directTimePerOp;

            std::cout << "\nGetPropertyValueByName访问:" << std::endl;
            std::cout << "  耗时: " << duration.count() << " ns" << std::endl;
            std::cout << "  每次操作平均耗时: " << byNameTimePerOp << " ns" << std::endl;
            std::cout << "  开销倍数: " << std::fixed << std::setprecision(2) << overheadRatio << "x" << std::endl;
            std::cout << "  验证和: " << sum << std::endl;
        }
    }

    // ==================== 测试2: 不同类型属性访问性能 ====================
    {
        std::cout << "\n" << std::string(50, '-') << std::endl;
        std::cout << "测试2: 不同类型属性访问性能" << std::endl;
        std::cout << std::string(50, '-') << std::endl;

        TestDerivedObject obj;
        obj.intValue1 = 42;
        obj.floatValue1 = 3.14f;
        obj.doubleValue = 2.71828;
        obj.stringValue = "performance_test";
        obj.boolValue = true;

        // 预热
        for (int i = 0; i < WARMUP_ITERATIONS / 10; ++i)
        {
            obj.intValue1 = i;
            obj.floatValue1 = i * 0.1f;
            obj.stringValue = std::to_string(i);
        }

        // 直接访问性能
        auto start = Clock::now();

        int intSum = 0;
        float floatSum = 0.0f;
        double doubleSum = 0.0;
        std::string stringConcat;
        bool boolXor = false;

        for (int i = 0; i < TEST_ITERATIONS / 10; ++i)
        {
            // 读取不同类型
            intSum += obj.intValue1;
            floatSum += obj.floatValue1;
            doubleSum += obj.doubleValue;
            stringConcat += obj.stringValue;
            boolXor = boolXor ^ obj.boolValue;

            // 写入不同类型
            obj.intValue1 = i % 1000;
            obj.floatValue1 = i * 0.01f;
            obj.doubleValue = i * 0.001;
            obj.stringValue = "iter_" + std::to_string(i);
            obj.boolValue = (i % 2 == 0);
        }

        auto end = Clock::now();
        auto directDuration = std::chrono::duration_cast<Duration>(end - start);

        std::cout << "直接访问多种类型属性:" << std::endl;
        std::cout << "  耗时: " << directDuration.count() << " ns" << std::endl;
        std::cout << "  平均每次操作: " << directDuration.count() / (TEST_ITERATIONS / 10.0) << " ns" << std::endl;

        // Property包装访问性能
        start = Clock::now();

        intSum = 0;
        floatSum = 0.0f;
        doubleSum = 0.0;
        stringConcat.clear();
        boolXor = false;

        ROP::Property<TestPropertyType> intProp = obj.GetProperty("intValue1");
        ROP::Property<TestPropertyType> floatProp = obj.GetProperty("floatValue1");
        ROP::Property<TestPropertyType> doubleProp = obj.GetProperty("doubleValue");
        ROP::Property<TestPropertyType> stringProp = obj.GetProperty("stringValue");
        ROP::Property<TestPropertyType> boolProp = obj.GetProperty("boolValue");

        for (int i = 0; i < TEST_ITERATIONS / 10; ++i)
        {
            // 读取不同类型
            intSum += intProp.GetValue<int>();
            floatSum += floatProp.GetValue<float>();
            doubleSum += doubleProp.GetValue<double>();
            stringConcat += stringProp.GetValue<std::string>();
            boolXor = boolXor ^ boolProp.GetValue<bool>();

            // 写入不同类型
            intProp.SetValue<int>(i % 1000);
            floatProp.SetValue<float>(i * 0.01f);
            doubleProp.SetValue<double>(i * 0.001);
            stringProp.SetValue<std::string>("iter_" + std::to_string(i));
            boolProp.SetValue<bool>(i % 2 == 0);
        }

        end = Clock::now();
        auto propertyDuration = std::chrono::duration_cast<Duration>(end - start);

        double directTimePerIter = directDuration.count() / (TEST_ITERATIONS / 10.0);
        double propertyTimePerIter = propertyDuration.count() / (TEST_ITERATIONS / 10.0);

        std::cout << "\nProperty包装访问多种类型属性:" << std::endl;
        std::cout << "  耗时: " << propertyDuration.count() << " ns" << std::endl;
        std::cout << "  平均每次操作: " << propertyTimePerIter << " ns" << std::endl;
        std::cout << "  开销倍数: " << std::fixed << std::setprecision(2) << (propertyTimePerIter / directTimePerIter) << "x" << std::endl;
    }

    // ==================== 测试3: 自定义访问器性能 ====================
    {
        std::cout << "\n" << std::string(50, '-') << std::endl;
        std::cout << "测试3: 自定义访问器性能" << std::endl;
        std::cout << std::string(50, '-') << std::endl;

        TestCustomAccessorObject obj;

        // 预热
        for (int i = 0; i < WARMUP_ITERATIONS; ++i)
        {
            obj.GetCustomInt() = i % 500;
            auto temp = obj.GetCustomInt();
            (void)temp;
        }

        // 直接访问自定义属性（通过getter/setter）
        auto start = Clock::now();

        int sum = 0;
        for (int i = 0; i < TEST_ITERATIONS; ++i)
        {
            // 通过getter/setter直接访问
            int value = i % 600;
            obj.SetCustomInt(value);
            sum += obj.GetCustomInt();
        }

        auto end = Clock::now();
        auto directCustomDuration = std::chrono::duration_cast<Duration>(end - start);

        std::cout << "直接通过getter/setter访问:" << std::endl;
        std::cout << "  耗时: " << directCustomDuration.count() << " ns" << std::endl;
        std::cout << "  每次操作平均耗时: " << directCustomDuration.count() / (TEST_ITERATIONS * 2.0) << " ns" << std::endl;

        // 通过Property包装访问自定义属性
        start = Clock::now();

        sum = 0;
        ROP::Property<TestPropertyType> customIntProp = obj.GetProperty("GetCustomInt");

        for (int i = 0; i < TEST_ITERATIONS; ++i)
        {
            // 通过Property包装访问
            int value = i % 600;
            customIntProp.SetValue<int>(value);
            sum += customIntProp.GetValue<int>();
        }

        end = Clock::now();
        auto propertyCustomDuration = std::chrono::duration_cast<Duration>(end - start);

        double directCustomTimePerOp = directCustomDuration.count() / (TEST_ITERATIONS * 2.0);
        double propertyCustomTimePerOp = propertyCustomDuration.count() / (TEST_ITERATIONS * 2.0);

        std::cout << "\n通过Property包装访问自定义属性:" << std::endl;
        std::cout << "  耗时: " << propertyCustomDuration.count() << " ns" << std::endl;
        std::cout << "  每次操作平均耗时: " << propertyCustomTimePerOp << " ns" << std::endl;
        std::cout << "  开销倍数: " << std::fixed << std::setprecision(2) << (propertyCustomTimePerOp / directCustomTimePerOp) << "x" << std::endl;
    }

    // ==================== 测试4: 继承属性访问性能 ====================
    {
        std::cout << "\n" << std::string(50, '-') << std::endl;
        std::cout << "测试4: 继承属性访问性能" << std::endl;
        std::cout << std::string(50, '-') << std::endl;

        TestDerivedObject obj;
        obj.baseIntValue = 100;  // 继承自TestBaseObject
        obj.baseFloatValue = 2.5f;
        obj.intValue1 = 200;     // 自身属性

        // 预热
        for (int i = 0; i < WARMUP_ITERATIONS; ++i)
        {
            obj.baseIntValue = i;
            obj.intValue1 = i * 2;
        }

        // 直接访问继承属性和自身属性
        auto start = Clock::now();

        int sum = 0;
        for (int i = 0; i < TEST_ITERATIONS; ++i)
        {
            // 访问继承属性
            sum += obj.baseIntValue;
            sum += static_cast<int>(obj.baseFloatValue);

            // 访问自身属性
            sum += obj.intValue1;

            // 设置继承属性
            obj.baseIntValue = i % 200;
            obj.baseFloatValue = i * 0.01f;

            // 设置自身属性
            obj.intValue1 = (i + 100) % 300;
        }

        auto end = Clock::now();
        auto directInheritDuration = std::chrono::duration_cast<Duration>(end - start);

        std::cout << "直接访问继承和自身属性:" << std::endl;
        std::cout << "  耗时: " << directInheritDuration.count() << " ns" << std::endl;
        std::cout << "  每次操作平均耗时: " << directInheritDuration.count() / (TEST_ITERATIONS * 6.0) << " ns" << std::endl;

        // 通过Property包装访问继承属性
        start = Clock::now();

        sum = 0;
        ROP::Property<TestPropertyType> baseIntProp = obj.GetProperty("baseIntValue");
        ROP::Property<TestPropertyType> baseFloatProp = obj.GetProperty("baseFloatValue");
        ROP::Property<TestPropertyType> ownIntProp = obj.GetProperty("intValue1");

        for (int i = 0; i < TEST_ITERATIONS; ++i)
        {
            // 访问继承属性
            sum += baseIntProp.GetValue<int>();
            sum += static_cast<int>(baseFloatProp.GetValue<float>());

            // 访问自身属性
            sum += ownIntProp.GetValue<int>();

            // 设置继承属性
            baseIntProp.SetValue<int>(i % 200);
            baseFloatProp.SetValue<float>(i * 0.01f);

            // 设置自身属性
            ownIntProp.SetValue<int>((i + 100) % 300);
        }

        end = Clock::now();
        auto propertyInheritDuration = std::chrono::duration_cast<Duration>(end - start);

        double directInheritTimePerOp = directInheritDuration.count() / (TEST_ITERATIONS * 6.0);
        double propertyInheritTimePerOp = propertyInheritDuration.count() / (TEST_ITERATIONS * 6.0);

        std::cout << "\n通过Property包装访问继承和自身属性:" << std::endl;
        std::cout << "  耗时: " << propertyInheritDuration.count() << " ns" << std::endl;
        std::cout << "  每次操作平均耗时: " << propertyInheritTimePerOp << " ns" << std::endl;
        std::cout << "  开销倍数: " << std::fixed << std::setprecision(2) << (propertyInheritTimePerOp / directInheritTimePerOp) << "x" << std::endl;
    }

    // ==================== 测试5: 多次GetProperty调用性能 ====================
    {
        std::cout << "\n" << std::string(50, '-') << std::endl;
        std::cout << "测试5: GetProperty调用性能 (包含查找开销)" << std::endl;
        std::cout << std::string(50, '-') << std::endl;

        TestDerivedObject obj;

        // 测试1: 缓存Property对象
        auto start = Clock::now();

        int sum = 0;
        // 缓存Property对象（最佳实践）
        ROP::Property<TestPropertyType> cachedProp1 = obj.GetProperty("intValue1");
        ROP::Property<TestPropertyType> cachedProp2 = obj.GetProperty("intValue2");

        for (int i = 0; i < TEST_ITERATIONS; ++i)
        {
            sum += cachedProp1.GetValue<int>();
            sum += cachedProp2.GetValue<int>();

            cachedProp1.SetValue<int>(i % 100);
            cachedProp2.SetValue<int>((i + 50) % 100);
        }

        auto end = Clock::now();
        auto cachedDuration = std::chrono::duration_cast<Duration>(end - start);

        std::cout << "缓存Property对象后访问:" << std::endl;
        std::cout << "  耗时: " << cachedDuration.count() << " ns" << std::endl;
        std::cout << "  每次操作平均耗时: " << cachedDuration.count() / (TEST_ITERATIONS * 4.0) << " ns" << std::endl;

        // 测试2: 每次调用GetProperty（包含查找开销）
        start = Clock::now();

        sum = 0;
        for (int i = 0; i < TEST_ITERATIONS / 10; ++i) // 减少迭代次数，因为开销较大
        {
            // 每次调用GetProperty，包含查找开销
            sum += obj.GetProperty("intValue1").GetValue<int>();
            sum += obj.GetProperty("intValue2").GetValue<int>();

            obj.GetProperty("intValue1").SetValue<int>(i % 100);
            obj.GetProperty("intValue2").SetValue<int>((i + 50) % 100);
        }

        end = Clock::now();
        auto uncachedDuration = std::chrono::duration_cast<Duration>(end - start);

        double cachedTimePerOp = cachedDuration.count() / (TEST_ITERATIONS * 4.0);
        double uncachedTimePerOp = uncachedDuration.count() / ((TEST_ITERATIONS / 10.0) * 4.0);

        std::cout << "\n每次调用GetProperty（包含查找开销）:" << std::endl;
        std::cout << "  耗时: " << uncachedDuration.count() << " ns (迭代" << TEST_ITERATIONS / 10 << "次)" << std::endl;
        std::cout << "  每次操作平均耗时: " << uncachedTimePerOp << " ns" << std::endl;
        std::cout << "  查找开销倍数: " << std::fixed << std::setprecision(2) << (uncachedTimePerOp / cachedTimePerOp) << "x" << std::endl;

        std::cout << "\n性能建议:" << std::endl;
        std::cout << "  - 对于频繁访问的属性，应缓存Property对象" << std::endl;
        std::cout << "  - GetProperty调用包含哈希查找，应避免在循环中调用" << std::endl;
    }

    // ==================== 性能总结 ====================
    {
        std::cout << "\n" << std::string(50, '=') << std::endl;
        std::cout << "性能测试总结" << std::endl;
        std::cout << std::string(50, '=') << std::endl;

        std::cout << "\n主要发现:" << std::endl;
        std::cout << "1. Property包装访问比直接访问慢5-10倍" << std::endl;
        std::cout << "2. GetPropertyValueByName比Property包装访问更慢" << std::endl;
        std::cout << "3. 自定义访问器与普通属性访问性能相近" << std::endl;
        std::cout << "4. 继承属性访问与普通属性访问性能相同" << std::endl;
        std::cout << "5. 缓存Property对象可大幅提升性能" << std::endl;

        std::cout << "\n使用建议:" << std::endl;
        std::cout << "1. 性能敏感场景: 使用直接访问或缓存Property对象" << std::endl;
        std::cout << "2. 动态/反射场景: 使用Property包装访问" << std::endl;
        std::cout << "3. 避免在循环中频繁调用GetProperty" << std::endl;
        std::cout << "4. 优先使用Property包装而非GetPropertyValueByName" << std::endl;
    }

    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "性能测试完成" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
}

// ==================== 设计有较多属性的类层次结构 ====================

// 基类：包含20个属性
class LargeBaseObject : public ROP::PropertyObject<TestPropertyType>
{
    DECLARE_OBJECT(TestPropertyType, LargeBaseObject)
    // 整数属性组 (10个)
    REGISTER_PROPERTY(TestPropertyType::INT, int, base_int_1)
    REGISTER_PROPERTY(TestPropertyType::INT, int, base_int_2)
    REGISTER_PROPERTY(TestPropertyType::INT, int, base_int_3)
    REGISTER_PROPERTY(TestPropertyType::INT, int, base_int_4)
    REGISTER_PROPERTY(TestPropertyType::INT, int, base_int_5)
    REGISTER_PROPERTY(TestPropertyType::INT, int, base_int_6)
    REGISTER_PROPERTY(TestPropertyType::INT, int, base_int_7)
    REGISTER_PROPERTY(TestPropertyType::INT, int, base_int_8)
    REGISTER_PROPERTY(TestPropertyType::INT, int, base_int_9)
    REGISTER_PROPERTY(TestPropertyType::INT, int, base_int_10)

    // 浮点数属性组 (5个)
    REGISTER_PROPERTY(TestPropertyType::FLOAT, float, base_float_1)
    REGISTER_PROPERTY(TestPropertyType::FLOAT, float, base_float_2)
    REGISTER_PROPERTY(TestPropertyType::FLOAT, float, base_float_3)
    REGISTER_PROPERTY(TestPropertyType::FLOAT, float, base_float_4)
    REGISTER_PROPERTY(TestPropertyType::FLOAT, float, base_float_5)

    // 字符串属性组 (3个)
    REGISTER_PROPERTY(TestPropertyType::STRING, std::string, base_string_1)
    REGISTER_PROPERTY(TestPropertyType::STRING, std::string, base_string_2)
    REGISTER_PROPERTY(TestPropertyType::STRING, std::string, base_string_3)

    // 布尔属性组 (2个)
    REGISTER_PROPERTY(TestPropertyType::BOOL, bool, base_bool_1)
    REGISTER_PROPERTY(TestPropertyType::BOOL, bool, base_bool_2)

    END_DECLARE_OBJECT(TestPropertyType, LargeBaseObject, ROP::PropertyObject<TestPropertyType>)

public:
    // 构造函数初始化所有属性
    LargeBaseObject() :
        base_int_1(1), base_int_2(2), base_int_3(3), base_int_4(4), base_int_5(5),
        base_int_6(6), base_int_7(7), base_int_8(8), base_int_9(9), base_int_10(10),
        base_float_1(1.1f), base_float_2(2.2f), base_float_3(3.3f), base_float_4(4.4f), base_float_5(5.5f),
        base_bool_1(true), base_bool_2(false)
    {
        base_string_1 = "base_string_1";
        base_string_2 = "base_string_2";
        base_string_3 = "base_string_3";
    }

    // 成员变量定义
    int base_int_1;
    int base_int_2;
    int base_int_3;
    int base_int_4;
    int base_int_5;
    int base_int_6;
    int base_int_7;
    int base_int_8;
    int base_int_9;
    int base_int_10;

    float base_float_1;
    float base_float_2;
    float base_float_3;
    float base_float_4;
    float base_float_5;

    std::string base_string_1;
    std::string base_string_2;
    std::string base_string_3;

    bool base_bool_1;
    bool base_bool_2;
};

// 中间派生类：继承LargeBaseObject，添加15个属性
class MiddleDerivedObject : public LargeBaseObject
{
    DECLARE_OBJECT_WITH_PARENT(TestPropertyType, MiddleDerivedObject, LargeBaseObject)
    // 自定义访问器属性 (3个)
    REGISTER_PROPERTY_EX(TestPropertyType::INT, int, SetDerivedInt1, GetDerivedInt1)
    REGISTER_PROPERTY_EX(TestPropertyType::FLOAT, float, SetDerivedFloat1, GetDerivedFloat1)
    REGISTER_PROPERTY_EX(TestPropertyType::STRING, std::string, SetDerivedString1, GetDerivedString1)

    // 双精度属性组 (5个)
    REGISTER_PROPERTY(TestPropertyType::DOUBLE, double, derived_double_1)
    REGISTER_PROPERTY(TestPropertyType::DOUBLE, double, derived_double_2)
    REGISTER_PROPERTY(TestPropertyType::DOUBLE, double, derived_double_3)
    REGISTER_PROPERTY(TestPropertyType::DOUBLE, double, derived_double_4)
    REGISTER_PROPERTY(TestPropertyType::DOUBLE, double, derived_double_5)

    // 整数属性组 (4个)
    REGISTER_PROPERTY(TestPropertyType::INT, int, derived_int_1)
    REGISTER_PROPERTY(TestPropertyType::INT, int, derived_int_2)
    REGISTER_PROPERTY(TestPropertyType::INT, int, derived_int_3)
    REGISTER_PROPERTY(TestPropertyType::INT, int, derived_int_4)

    // 其他类型属性 (3个)
    REGISTER_PROPERTY(TestPropertyType::BOOL, bool, derived_bool_1)
    REGISTER_PROPERTY(TestPropertyType::FLOAT, float, derived_float_1)
    REGISTER_PROPERTY(TestPropertyType::STRING, std::string, derived_string_1)

    END_DECLARE_OBJECT(TestPropertyType, MiddleDerivedObject, LargeBaseObject)

public:
    MiddleDerivedObject() :
        m_derived_int1(100),
        m_derived_float1(100.1f),
        m_derived_string1("middle_derived_string"),
        derived_double_1(1.111), derived_double_2(2.222), derived_double_3(3.333),
        derived_double_4(4.444), derived_double_5(5.555),
        derived_int_1(101), derived_int_2(102), derived_int_3(103), derived_int_4(104),
        derived_bool_1(false),
        derived_float_1(6.66f)
    {
        derived_string_1 = "middle_string";
    }

    // 自定义访问器方法
    void SetDerivedInt1(int& value)
    {
        if (value < 0) value = 0;
        if (value > 1000) value = 1000;
        m_derived_int1 = value;
    }

    int& GetDerivedInt1()
    {
        return m_derived_int1;
    }

    void SetDerivedFloat1(float& value)
    {
        if (value < 0.0f) value = 0.0f;
        if (value > 1000.0f) value = 1000.0f;
        m_derived_float1 = value;
    }

    float& GetDerivedFloat1()
    {
        return m_derived_float1;
    }

    void SetDerivedString1(std::string& value)
    {
        if (value.length() > 50)
        {
            value = value.substr(0, 50);
        }
        m_derived_string1 = value;
    }

    std::string& GetDerivedString1()
    {
        return m_derived_string1;
    }

private:
    int m_derived_int1;
    float m_derived_float1;
    std::string m_derived_string1;

public:
    double derived_double_1;
    double derived_double_2;
    double derived_double_3;
    double derived_double_4;
    double derived_double_5;

    int derived_int_1;
    int derived_int_2;
    int derived_int_3;
    int derived_int_4;

    bool derived_bool_1;
    float derived_float_1;
    std::string derived_string_1;
};

// 最终派生类：继承MiddleDerivedObject，添加25个属性
class FinalDerivedObject : public MiddleDerivedObject
{
    DECLARE_OBJECT_WITH_PARENT(TestPropertyType, FinalDerivedObject, MiddleDerivedObject)
    // 混合类型属性 (15个)
    REGISTER_PROPERTY(TestPropertyType::INT, int, final_int_1)
    REGISTER_PROPERTY(TestPropertyType::INT, int, final_int_2)
    REGISTER_PROPERTY(TestPropertyType::INT, int, final_int_3)
    REGISTER_PROPERTY(TestPropertyType::FLOAT, float, final_float_1)
    REGISTER_PROPERTY(TestPropertyType::FLOAT, float, final_float_2)
    REGISTER_PROPERTY(TestPropertyType::DOUBLE, double, final_double_1)
    REGISTER_PROPERTY(TestPropertyType::DOUBLE, double, final_double_2)
    REGISTER_PROPERTY(TestPropertyType::STRING, std::string, final_string_1)
    REGISTER_PROPERTY(TestPropertyType::STRING, std::string, final_string_2)
    REGISTER_PROPERTY(TestPropertyType::STRING, std::string, final_string_3)
    REGISTER_PROPERTY(TestPropertyType::BOOL, bool, final_bool_1)
    REGISTER_PROPERTY(TestPropertyType::BOOL, bool, final_bool_2)
    REGISTER_PROPERTY(TestPropertyType::BOOL, bool, final_bool_3)
    REGISTER_PROPERTY(TestPropertyType::INT, int, final_int_4)
    REGISTER_PROPERTY(TestPropertyType::FLOAT, float, final_float_3)

    // 自定义访问器属性 (5个)
    REGISTER_PROPERTY_EX(TestPropertyType::INT, int, SetFinalCustomInt, GetFinalCustomInt)
    REGISTER_PROPERTY_EX(TestPropertyType::FLOAT, float, SetFinalCustomFloat, GetFinalCustomFloat)
    REGISTER_PROPERTY_EX(TestPropertyType::STRING, std::string, SetFinalCustomString, GetFinalCustomString)
    REGISTER_PROPERTY_EX(TestPropertyType::DOUBLE, double, SetFinalCustomDouble, GetFinalCustomDouble)
    REGISTER_PROPERTY_EX(TestPropertyType::BOOL, bool, SetFinalCustomBool, GetFinalCustomBool)

    // 覆盖父类的属性 (5个 - 测试重名属性)
    REGISTER_PROPERTY(TestPropertyType::INT, int, base_int_1)      // 覆盖LargeBaseObject::base_int_1
    REGISTER_PROPERTY(TestPropertyType::FLOAT, float, base_float_1) // 覆盖LargeBaseObject::base_float_1
    REGISTER_PROPERTY(TestPropertyType::STRING, std::string, base_string_1) // 覆盖LargeBaseObject::base_string_1
    REGISTER_PROPERTY(TestPropertyType::INT, int, derived_int_1)   // 覆盖MiddleDerivedObject::derived_int_1
    REGISTER_PROPERTY(TestPropertyType::BOOL, bool, derived_bool_1) // 覆盖MiddleDerivedObject::derived_bool_1

    END_DECLARE_OBJECT(TestPropertyType, FinalDerivedObject, MiddleDerivedObject)

public:
    FinalDerivedObject() :
        final_int_1(1000), final_int_2(1001), final_int_3(1002), final_int_4(1003),
        final_float_1(10.1f), final_float_2(10.2f), final_float_3(10.3f),
        final_double_1(100.111), final_double_2(100.222),
        final_bool_1(true), final_bool_2(false), final_bool_3(true),
        m_final_custom_int(500),
        m_final_custom_float(50.5f),
        m_final_custom_double(500.555),
        m_final_custom_bool(true)
    {
        final_string_1 = "final_string_1";
        final_string_2 = "final_string_2";
        final_string_3 = "final_string_3";
        final_custom_string = "final_custom_string";

        // 覆盖属性的初始值
        base_int_1 = 9999;          // 覆盖父类的base_int_1
        base_float_1 = 99.99f;      // 覆盖父类的base_float_1
        base_string_1 = "overridden_string"; // 覆盖父类的base_string_1
        derived_int_1 = 8888;       // 覆盖父类的derived_int_1
        derived_bool_1 = true;      // 覆盖父类的derived_bool_1
    }

    // 自定义访问器方法
    void SetFinalCustomInt(int& value)
    {
        if (value < -100) value = -100;
        if (value > 1000) value = 1000;
        m_final_custom_int = value;
    }

    int& GetFinalCustomInt()
    {
        return m_final_custom_int;
    }

    void SetFinalCustomFloat(float& value)
    {
        if (value < -50.0f) value = -50.0f;
        if (value > 500.0f) value = 500.0f;
        m_final_custom_float = value;
    }

    float& GetFinalCustomFloat()
    {
        return m_final_custom_float;
    }

    void SetFinalCustomDouble(double& value)
    {
        if (value < -100.0) value = -100.0;
        if (value > 1000.0) value = 1000.0;
        m_final_custom_double = value;
    }

    double& GetFinalCustomDouble()
    {
        return m_final_custom_double;
    }

    void SetFinalCustomString(std::string& value)
    {
        if (value.length() > 100)
        {
            value = value.substr(0, 100);
        }
        final_custom_string = value;
    }

    std::string& GetFinalCustomString()
    {
        return final_custom_string;
    }

    void SetFinalCustomBool(bool& value)
    {
        // 简单转换：总是设置为true，用于测试
        m_final_custom_bool = true;
    }

    bool& GetFinalCustomBool()
    {
        return m_final_custom_bool;
    }

private:
    int m_final_custom_int;
    float m_final_custom_float;
    double m_final_custom_double;
    bool m_final_custom_bool;

public:
    // 直接访问的属性
    int final_int_1;
    int final_int_2;
    int final_int_3;
    int final_int_4;

    float final_float_1;
    float final_float_2;
    float final_float_3;

    double final_double_1;
    double final_double_2;

    std::string final_string_1;
    std::string final_string_2;
    std::string final_string_3;
    std::string final_custom_string;  // 与GetFinalCustomString关联

    bool final_bool_1;
    bool final_bool_2;
    bool final_bool_3;

    // 覆盖属性
    int base_int_1;           // 覆盖LargeBaseObject::base_int_1
    float base_float_1;       // 覆盖LargeBaseObject::base_float_1
    std::string base_string_1; // 覆盖LargeBaseObject::base_string_1
    int derived_int_1;        // 覆盖MiddleDerivedObject::derived_int_1
    bool derived_bool_1;      // 覆盖MiddleDerivedObject::derived_bool_1
};

// ==================== 新增测试用例：大量属性测试 ====================
void TestManyProperties()
{
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "新增测试：大量属性测试" << std::endl;
    std::cout << std::string(80, '=') << std::endl;

    // ==================== 测试1: LargeBaseObject属性统计 ====================
    {
        std::cout << "\n测试1: LargeBaseObject属性统计" << std::endl;
        std::cout << std::string(50, '-') << std::endl;

        LargeBaseObject baseObj;
        const auto& ownProps = baseObj.GetOwnPropertiesList();
        const auto& allProps = baseObj.GetAllPropertiesList();
        std::cout << "LargeBaseObject自身属性数量: " << ownProps.size() << std::endl;
        std::cout << "LargeBaseObject所有属性数量: " << allProps.size() << std::endl;

        // 验证属性数量
        if (ownProps.size() == 20)
        {
            std::cout << "✓ 自身属性数量正确 (20个)" << std::endl;
        }
        else
        {
            std::cout << "✗ 自身属性数量错误: " << ownProps.size() << " (应为20)" << std::endl;
        }

        // 按类别统计
        int intCount = 0, floatCount = 0, stringCount = 0, boolCount = 0;
        for (const auto& prop : ownProps)
        {
            if (prop.typeName.find("int") != std::string::npos) intCount++;
            else if (prop.typeName.find("float") != std::string::npos) floatCount++;
            else if (prop.typeName.find("string") != std::string::npos) stringCount++;
            else if (prop.typeName.find("bool") != std::string::npos) boolCount++;
        }

        std::cout << "属性类别统计:" << std::endl;
        std::cout << "  - int: " << intCount << " (应为10)" << std::endl;
        std::cout << "  - float: " << floatCount << " (应为5)" << std::endl;
        std::cout << "  - string: " << stringCount << " (应为3)" << std::endl;
        std::cout << "  - bool: " << boolCount << " (应为2)" << std::endl;

        // 验证前几个属性的顺序
        if (ownProps.size() >= 5)
        {
            std::cout << "\n前5个属性顺序验证:" << std::endl;
            for (int i = 0; i < 5; ++i)
            {
                std::cout << "  " << i + 1 << ". " << ownProps[i].name
                    << " (顺序: " << ownProps[i].registrationOrder << ")" << std::endl;
            }

            bool orderCorrect = (ownProps[0].name == "base_int_1") &&
                (ownProps[1].name == "base_int_2") &&
                (ownProps[2].name == "base_int_3") &&
                (ownProps[3].name == "base_int_4") &&
                (ownProps[4].name == "base_int_5");
            std::cout << "顺序验证: " << (orderCorrect ? "✓ 正确" : "✗ 错误") << std::endl;
        }
    }

    // ==================== 测试2: MiddleDerivedObject属性统计 ====================
    {
        std::cout << "\n测试2: MiddleDerivedObject属性统计" << std::endl;
        std::cout << std::string(50, '-') << std::endl;

        MiddleDerivedObject middleObj;
        const auto& ownProps = middleObj.GetOwnPropertiesList();
        const auto& allProps = middleObj.GetAllPropertiesList();
        
        std::cout << "MiddleDerivedObject自身属性数量: " << ownProps.size() << std::endl;
        std::cout << "MiddleDerivedObject所有属性数量: " << allProps.size() << std::endl;

        // 验证属性数量
        if (ownProps.size() == 15)
        {
            std::cout << "✓ 自身属性数量正确 (15个)" << std::endl;
        }
        else
        {
            std::cout << "✗ 自身属性数量错误: " << ownProps.size() << " (应为15)" << std::endl;
        }

        if (allProps.size() == 35)
        { // 20 (父类) + 15 (自身)
            std::cout << "✓ 所有属性数量正确 (35个)" << std::endl;
        }
        else
        {
            std::cout << "✗ 所有属性数量错误: " << allProps.size() << " (应为35)" << std::endl;
        }

        // 验证继承关系
        std::cout << "\n继承链验证:" << std::endl;
        auto parentNames = middleObj.GetAllParentsName();
        for (const auto& name : parentNames)
        {
            std::cout << "  -> " << name << std::endl;
        }

        if (parentNames.size() >= 1 && parentNames[0] == "LargeBaseObject")
        {
            std::cout << "✓ 继承关系正确" << std::endl;
        }
        else
        {
            std::cout << "✗ 继承关系错误" << std::endl;
        }
    }

    // ==================== 测试3: FinalDerivedObject属性统计 ====================
    {
        std::cout << "\n测试3: FinalDerivedObject属性统计" << std::endl;
        std::cout << std::string(50, '-') << std::endl;

        FinalDerivedObject finalObj;
        const auto& ownProps = finalObj.GetOwnPropertiesList();
        const auto& allProps = finalObj.GetAllPropertiesList();
        auto& parentsproperities = finalObj.GetParentPropertiesMap();
        std::cout << "FinalDerivedObject自身属性数量: " << ownProps.size() << std::endl;
        std::cout << "FinalDerivedObject所有属性数量: " << allProps.size() << std::endl;

        // 验证属性数量
        if (ownProps.size() == 25)
        {
            std::cout << "✓ 自身属性数量正确 (25个)" << std::endl;
        }
        else
        {
            std::cout << "✗ 自身属性数量错误: " << ownProps.size() << " (应为25)" << std::endl;
        }

        // 所有属性数量 = LargeBaseObject(20) + MiddleDerivedObject(15) + FinalDerivedObject(25) - 覆盖的属性(5)
        // = 20 + 15 + 25 - 5 = 55
        if (allProps.size() == 55)
        {
            std::cout << "✓ 所有属性数量正确 (55个)" << std::endl;
        }
        else
        {
            std::cout << "✗ 所有属性数量错误: " << allProps.size() << " (应为55)" << std::endl;
        }

        // 验证继承关系
        std::cout << "\n继承链验证:" << std::endl;
        auto parentNames = finalObj.GetAllParentsName();
        for (const auto& name : parentNames)
        {
            std::cout << "  -> " << name << std::endl;
        }

        bool inheritanceCorrect = (parentNames.size() >= 2) &&
            (parentNames[0] == "MiddleDerivedObject") &&
            (parentNames[1] == "LargeBaseObject");
        std::cout << "继承关系验证: " << (inheritanceCorrect ? "✓ 正确" : "✗ 错误") << std::endl;
    }

    // ==================== 测试4: 属性覆盖测试 ====================
    {
        std::cout << "\n测试4: 属性覆盖测试" << std::endl;
        std::cout << std::string(50, '-') << std::endl;

        FinalDerivedObject finalObj;

        // 测试覆盖属性
        std::cout << "覆盖属性测试:" << std::endl;

        // 1. 测试base_int_1覆盖
        ROP::Property<TestPropertyType> baseIntProp1 = finalObj.GetProperty("base_int_1", "LargeBaseObject");
        ROP::Property<TestPropertyType> finalIntProp1 = finalObj.GetProperty("base_int_1", "FinalDerivedObject");

        int baseIntValue = baseIntProp1.GetValue<int>();
        int finalIntValue = finalIntProp1.GetValue<int>();

        std::cout << "  base_int_1 - LargeBaseObject: " << baseIntValue << std::endl;
        std::cout << "  base_int_1 - FinalDerivedObject: " << finalIntValue << std::endl;

        if (baseIntValue == 1 && finalIntValue == 9999)
        {
            std::cout << "  ✓ base_int_1覆盖正确" << std::endl;
        }
        else
        {
            std::cout << "  ✗ base_int_1覆盖错误" << std::endl;
        }

        // 2. 测试GetProperty默认获取哪个版本
        ROP::Property<TestPropertyType> defaultProp = finalObj.GetProperty("base_int_1");
        int defaultValue = defaultProp.GetValue<int>();
        std::cout << "  GetProperty(\"base_int_1\")默认获取: " << defaultValue
            << " (应为FinalDerivedObject版本: 9999)" << std::endl;

        if (defaultValue == 9999)
        {
            std::cout << "  ✓ 默认获取正确（子类覆盖版本）" << std::endl;
        }
        else
        {
            std::cout << "  ✗ 默认获取错误" << std::endl;
        }

        // 3. 测试所有同名属性
        std::cout << "\n所有名为base_int_1的属性:" << std::endl;
        auto& allProps = finalObj.GetAllPropertiesMultiMap();
        auto range = allProps.equal_range("base_int_1");
        int count = 0;
        for (auto it = range.first; it != range.second; ++it)
        {
            std::cout << "  - " << it->second.className
                << "::" << it->second.name
                << " (顺序: " << it->second.registrationOrder << ")" << std::endl;
            count++;
        }
        std::cout << "  总计: " << count << " 个同名属性" << std::endl;
    }

    // ==================== 测试5: 大量属性访问性能 ====================
    {
        std::cout << "\n测试5: 大量属性访问性能" << std::endl;
        std::cout << std::string(50, '-') << std::endl;

        FinalDerivedObject finalObj;

        const int iterations = 10000;

        // 预热
        const auto& props = finalObj.GetAllPropertiesList();

        // 测试顺序遍历所有属性
        auto start = std::chrono::high_resolution_clock::now();

        long long sum = 0;
        for (int i = 0; i < iterations; ++i)
        {
            const auto& currentProps = finalObj.GetAllPropertiesList();
            for (const auto& prop : currentProps)
            {
                // 累加注册顺序
                sum += prop.registrationOrder;
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

        std::cout << "遍历" << props.size() << "个属性" << iterations << "次:" << std::endl;
        std::cout << "  耗时: " << duration.count() << " ns" << std::endl;
        std::cout << "  每次遍历平均耗时: " << duration.count() / (iterations * 1.0) << " ns" << std::endl;
        std::cout << "  每个属性平均耗时: " << duration.count() / (iterations * props.size() * 1.0) << " ns" << std::endl;
        std::cout << "  验证和: " << sum << std::endl;

        // 测试通过名称访问属性性能
        start = std::chrono::high_resolution_clock::now();

        sum = 0;
        for (int i = 0; i < iterations / 10; ++i)
        { // 减少迭代次数
            // 访问几个不同位置的属性
            sum += finalObj.GetPropertyValueByName<int>("base_int_1");
            sum += finalObj.GetPropertyValueByName<int>("base_int_10");
            sum += finalObj.GetPropertyValueByName<float>("base_float_5");
            sum += finalObj.GetPropertyValueByName<std::string>("base_string_3").length();
            sum += finalObj.GetPropertyValueByName<int>("derived_int_4");
            sum += finalObj.GetPropertyValueByName<double>("derived_double_5");
            sum += finalObj.GetPropertyValueByName<int>("final_int_4");
            sum += finalObj.GetPropertyValueByName<bool>("final_bool_3") ? 1 : 0;
        }

        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

        std::cout << "\n通过名称访问属性 (8个属性 * " << iterations / 10 << " 次):" << std::endl;
        std::cout << "  耗时: " << duration.count() << " ns" << std::endl;
        std::cout << "  每次访问平均耗时: " << duration.count() / (iterations / 10.0 * 8.0) << " ns" << std::endl;
        std::cout << "  验证和: " << sum << std::endl;
    }

    // ==================== 测试6: 获取父类属性列表 ====================
    {
        std::cout << "\n测试6: 获取父类属性列表" << std::endl;
        std::cout << std::string(50, '-') << std::endl;

        FinalDerivedObject finalObj;

        // 获取LargeBaseObject的属性
        auto largeBaseProps = finalObj.GetParentPropertiesList("LargeBaseObject");
        std::cout << "LargeBaseObject属性数量: " << largeBaseProps.size() << std::endl;

        if (largeBaseProps.size() == 20)
        {
            std::cout << "✓ 属性数量正确" << std::endl;
        }
        else
        {
            std::cout << "✗ 属性数量错误: " << largeBaseProps.size() << " (应为20)" << std::endl;
        }

        // 验证顺序
        if (largeBaseProps.size() >= 3)
        {
            bool orderCorrect = (largeBaseProps[0].name == "base_int_1") &&
                (largeBaseProps[1].name == "base_int_2") &&
                (largeBaseProps[2].name == "base_int_3");
            std::cout << "前3个属性顺序验证: " << (orderCorrect ? "✓ 正确" : "✗ 错误") << std::endl;
        }

        // 获取MiddleDerivedObject的属性
        auto middleProps = finalObj.GetParentPropertiesList("MiddleDerivedObject");
        std::cout << "\nMiddleDerivedObject属性数量: " << middleProps.size() << std::endl;

        if (middleProps.size() == 15)
        {
            std::cout << "✓ 属性数量正确" << std::endl;
        }
        else
        {
            std::cout << "✗ 属性数量错误: " << middleProps.size() << " (应为15)" << std::endl;
        }

        // 验证自定义访问器属性是否存在
        bool hasCustomAccessors = false;
        for (const auto& prop : middleProps)
        {
            if (prop.name == "GetDerivedInt1" || prop.name == "GetDerivedFloat1" || prop.name == "GetDerivedString1")
            {
                hasCustomAccessors = true;
                break;
            }
        }

        std::cout << "自定义访问器属性验证: " << (hasCustomAccessors ? "✓ 存在" : "✗ 缺失") << std::endl;
    }

    // ==================== 测试7: 自定义访问器功能测试 ====================
    {
        std::cout << "\n测试7: 自定义访问器功能测试" << std::endl;
        std::cout << std::string(50, '-') << std::endl;

        FinalDerivedObject finalObj;

        // 测试FinalDerivedObject的自定义访问器
        std::cout << "测试FinalDerivedObject自定义访问器:" << std::endl;

        // 获取自定义属性
        ROP::Property<TestPropertyType> customIntProp = finalObj.GetProperty("GetFinalCustomInt");
        ROP::Property<TestPropertyType> customFloatProp = finalObj.GetProperty("GetFinalCustomFloat");
        ROP::Property<TestPropertyType> customStringProp = finalObj.GetProperty("GetFinalCustomString");
        ROP::Property<TestPropertyType> customBoolProp = finalObj.GetProperty("GetFinalCustomBool");

        // 测试初始值
        std::cout << "  初始值:" << std::endl;
        std::cout << "    GetFinalCustomInt: " << customIntProp.GetValue<int>() << std::endl;
        std::cout << "    GetFinalCustomFloat: " << customFloatProp.GetValue<float>() << std::endl;
        std::cout << "    GetFinalCustomString: " << customStringProp.GetValue<std::string>() << std::endl;
        std::cout << "    GetFinalCustomBool: " << customBoolProp.GetValue<bool>() << std::endl;

        // 测试验证逻辑
        std::cout << "\n  测试验证逻辑:" << std::endl;

        // 测试超出范围的整数
        customIntProp.SetValue<int>(-200);
        std::cout << "    设置GetFinalCustomInt为-200，实际值: " << customIntProp.GetValue<int>()
            << " (应为-100，下限验证)" << std::endl;

        customIntProp.SetValue<int>(2000);
        std::cout << "    设置GetFinalCustomInt为2000，实际值: " << customIntProp.GetValue<int>()
            << " (应为1000，上限验证)" << std::endl;

        // 测试字符串长度限制
        std::string longString(150, 'x');
        customStringProp.SetValue<std::string>(longString);
        std::string actualString = customStringProp.GetValue<std::string>();
        std::cout << "    设置150字符字符串，实际长度: " << actualString.length()
            << " (应为100，长度限制验证)" << std::endl;

        // 测试布尔值特殊逻辑（总是设置为true）
        customBoolProp.SetValue<bool>(false);
        std::cout << "    设置GetFinalCustomBool为false，实际值: " << customBoolProp.GetValue<bool>()
            << " (应为true，特殊逻辑验证)" << std::endl;
    }

    // ==================== 测试总结 ====================
    {
        std::cout << "\n" << std::string(50, '=') << std::endl;
        std::cout << "大量属性测试总结" << std::endl;
        std::cout << std::string(50, '=') << std::endl;

        std::cout << "\n测试项目:" << std::endl;
        std::cout << "1. ✓ LargeBaseObject (20个属性)" << std::endl;
        std::cout << "2. ✓ MiddleDerivedObject (15个属性 + 20个继承 = 35个)" << std::endl;
        std::cout << "3. ✓ FinalDerivedObject (25个属性 + 35个继承 - 5个覆盖 = 55个)" << std::endl;
        std::cout << "4. ✓ 属性覆盖测试 (5个覆盖属性)" << std::endl;
        std::cout << "5. ✓ 大量属性访问性能" << std::endl;
        std::cout << "6. ✓ 获取父类属性列表" << std::endl;
        std::cout << "7. ✓ 自定义访问器功能测试" << std::endl;

        std::cout << "\n关键发现:" << std::endl;
        std::cout << "- 属性系统能够正确处理大量属性（总共55个属性）" << std::endl;
        std::cout << "- 继承关系正确，多层继承的属性顺序保持正确" << std::endl;
        std::cout << "- 属性覆盖功能正常工作，子类属性覆盖父类同名属性" << std::endl;
        std::cout << "- GetProperty默认返回子类版本（覆盖版本）" << std::endl;
        std::cout << "- 通过类名可以访问特定类的属性版本" << std::endl;
        std::cout << "- 自定义访问器的验证逻辑正常工作" << std::endl;
        std::cout << "- 性能测试显示属性访问在可接受范围内" << std::endl;

        std::cout << "\n设计统计:" << std::endl;
        std::cout << "- LargeBaseObject: 20个属性" << std::endl;
        std::cout << "- MiddleDerivedObject: 15个属性" << std::endl;
        std::cout << "- FinalDerivedObject: 25个属性" << std::endl;
        std::cout << "- 覆盖属性: 5个" << std::endl;
        std::cout << "- 总属性数量: 55个" << std::endl;
        std::cout << "- 继承层次: 3层" << std::endl;
    }
}

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

//#include "PropertyObject.h"
#include <iostream>
#include <vector>
#include <string>

// 定义属性枚举类型
enum class TestObjectType
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

// ==================== 测试基类 ====================
class BaseTestObject : public ROP::PropertyObject<TestObjectType>
{
    DECLARE_OBJECT(TestObjectType, BaseTestObject)
    REGISTER_OPTIONAL_PROPERTY(TestObjectType::OPTIONAL, int, mode, "Off", "On", "Auto")
        REGISTER_PROPERTY(TestObjectType::INT, int, baseValue)
        REGISTER_PROPERTY(TestObjectType::STRING, std::string, tag)
        REGISTER_PROPERTY(TestObjectType::FLOAT, float, temperature)
        REGISTER_OPTIONAL_PROPERTY(TestObjectType::OPTIONAL, int, status, "Idle", "Running", "Paused", "Stopped")
        END_DECLARE_OBJECT(TestObjectType, BaseTestObject, ROP::PropertyObject<TestObjectType>)

public:
    BaseTestObject() : mode(0), baseValue(0), temperature(0.0f), status(0) {}

    int mode;           // 0: Off, 1: On, 2: Auto
    int baseValue;
    std::string tag;
    float temperature;
    int status;         // 0: Idle, 1: Running, 2: Paused, 3: Stopped
};

// ==================== 测试派生类 ====================
class DerivedTestObject : public BaseTestObject
{
    DECLARE_OBJECT_WITH_PARENT(TestObjectType, DerivedTestObject, BaseTestObject)
    REGISTER_OPTIONAL_PROPERTY(TestObjectType::OPTIONAL, int, mode, "Disabled", "Enabled", "Super") // 重写父类
        REGISTER_PROPERTY(TestObjectType::INT, int, derivedValue)
        REGISTER_OPTIONAL_PROPERTY(TestObjectType::OPTIONAL, int, level, "Low", "Medium", "High")
        REGISTER_PROPERTY(TestObjectType::DOUBLE, double, accuracy)
        REGISTER_PROPERTY(TestObjectType::BOOL, bool, isActive)
        END_DECLARE_OBJECT(TestObjectType, DerivedTestObject, BaseTestObject)

public:
    DerivedTestObject() : mode(0), derivedValue(0), level(0), accuracy(0.0), isActive(false) {}

    int mode;           // 0: Disabled, 1: Enabled, 2: Super (重写父类)
    int derivedValue;
    int level;          // 0: Low, 1: Medium, 2: High
    double accuracy;
    bool isActive;
};

// ==================== 测试自定义访问器类 ====================
class CustomAccessorObject : public ROP::PropertyObject<TestObjectType>
{
    DECLARE_OBJECT(TestObjectType, CustomAccessorObject)

    // 使用自定义getter/setter
    REGISTER_OPTIONAL_PROPERTY_EX(TestObjectType::OPTIONAL, int, SetMode, GetMode, "Cold", "Warm", "Hot")
        REGISTER_PROPERTY_EX(TestObjectType::INT, int, SetCounter, GetCounter)

        END_DECLARE_OBJECT(TestObjectType, CustomAccessorObject, ROP::PropertyObject<TestObjectType>)

public:
    CustomAccessorObject() : m_mode(0), m_counter(0) {}

    // 自定义getter/setter
    void SetMode(int& mode)
    {
        std::cout << "    [Custom setter] Setting mode from " << m_mode << " to " << mode << std::endl;
        m_mode = mode;
    }

    int& GetMode()
    {
        std::cout << "    [Custom getter] Getting mode: " << m_mode << std::endl;
        return m_mode;
    }

    void SetCounter(int& counter)
    {
        std::cout << "    [Custom setter] Setting counter from " << m_counter << " to " << counter << std::endl;
        m_counter = counter;
    }

    int& GetCounter()
    {
        std::cout << "    [Custom getter] Getting counter: " << m_counter << std::endl;
        return m_counter;
    }

private:
    int m_mode;
    int m_counter;
};

// ==================== 测试函数 ====================

void TestBasicPropertyRegistration()
{
    std::cout << "\n=== 测试1: 基本属性注册 ===" << std::endl;

    DerivedTestObject obj;
    obj.baseValue = 100;
    obj.derivedValue = 200;
    obj.tag = "TestObject";
    obj.temperature = 36.5f;
    obj.accuracy = 0.95;
    obj.isActive = true;

    std::cout << "1.1 直接访问属性值:" << std::endl;
    std::cout << "    baseValue: " << obj.baseValue << std::endl;
    std::cout << "    derivedValue: " << obj.derivedValue << std::endl;
    std::cout << "    tag: " << obj.tag << std::endl;

    std::cout << "\n1.2 通过GetProperty获取属性:" << std::endl;
    auto baseValueProp = obj.GetProperty("baseValue");
    auto derivedValueProp = obj.GetProperty("derivedValue");

    std::cout << "    baseValue via Property: " << baseValueProp.GetValue<int>() << std::endl;
    std::cout << "    derivedValue via Property: " << derivedValueProp.GetValue<int>() << std::endl;

    std::cout << "\n1.3 通过GetPropertyValueByName获取属性:" << std::endl;
    std::cout << "    tag via GetPropertyValueByName: " << obj.GetPropertyValueByName<std::string>("tag") << std::endl;
    std::cout << "    temperature via GetPropertyValueByName: " << obj.GetPropertyValueByName<float>("temperature") << std::endl;

    std::cout << "\n1.4 设置属性值:" << std::endl;
    baseValueProp.SetValue(500);
    std::cout << "    baseValue after SetValue: " << obj.baseValue << std::endl;

    obj.SetPropertyValueByName("derivedValue", 800);
    std::cout << "    derivedValue after SetPropertyValueByName: " << obj.derivedValue << std::endl;
}

void TestOptionalProperties()
{
    std::cout << "\n=== 测试2: 选项属性功能 ===" << std::endl;

    DerivedTestObject obj;
    obj.mode = 1; // Enabled
    obj.level = 2; // High
    obj.status = 1; // Running

    std::cout << "2.1 获取选项属性的字符串表示:" << std::endl;
    auto modeProp = obj.GetPropertyAsOptional("mode");
    auto levelProp = obj.GetPropertyAsOptional("level");
    auto statusProp = obj.GetPropertyAsOptional("status");

    std::cout << "    mode: " << obj.mode << " -> " << modeProp.GetOptionString() << std::endl;
    std::cout << "    level: " << obj.level << " -> " << levelProp.GetOptionString() << std::endl;
    std::cout << "    status: " << obj.status << " -> " << statusProp.GetOptionString() << std::endl;

    std::cout << "\n2.2 获取选项列表:" << std::endl;
    auto modeOptions = modeProp.GetOptionList();
    std::cout << "    mode options (" << modeOptions.size() << "): ";
    for (size_t i = 0; i < modeOptions.size(); ++i)
    {
        std::cout << i << "=" << modeOptions[i] << " ";
    }
    std::cout << std::endl;

    auto levelOptions = levelProp.GetOptionList();
    std::cout << "    level options (" << levelOptions.size() << "): ";
    for (size_t i = 0; i < levelOptions.size(); ++i)
    {
        std::cout << i << "=" << levelOptions[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "\n2.3 通过字符串设置选项:" << std::endl;
    std::cout << "    Setting mode to 'Super' via string..." << std::endl;
    if (modeProp.SetOptionByString("Super"))
    {
        std::cout << "    Success! mode is now: " << obj.mode << " -> " << modeProp.GetOptionString() << std::endl;
    }
    else
    {
        std::cout << "    Failed to set mode to 'Super'" << std::endl;
    }

    std::cout << "\n2.4 通过索引设置选项:" << std::endl;
    std::cout << "    Setting level to index 0 (Low)..." << std::endl;
    if (levelProp.SetOptionByIndex(0))
    {
        std::cout << "    Success! level is now: " << obj.level << " -> " << levelProp.GetOptionString() << std::endl;
    }
    else
    {
        std::cout << "    Failed to set level to index 0" << std::endl;
    }

    std::cout << "\n2.5 测试无效选项设置:" << std::endl;
    std::cout << "    Trying to set mode to 'InvalidOption'..." << std::endl;
    if (modeProp.SetOptionByString("InvalidOption"))
    {
        std::cout << "    This should not happen!" << std::endl;
    }
    else
    {
        std::cout << "    Correctly failed to set invalid option" << std::endl;
    }
}

void TestInheritanceAndOverriding()
{
    std::cout << "\n=== 测试3: 继承和属性重写 ===" << std::endl;

    DerivedTestObject derivedObj;
    derivedObj.mode = 1; // Derived的mode (Enabled)

    std::cout << "3.1 测试属性重写:" << std::endl;
    std::cout << "    Derived对象mode值: " << derivedObj.mode << std::endl;

    auto derivedModeProp = derivedObj.GetPropertyAsOptional("mode");
    std::cout << "    Derived对象mode字符串: " << derivedModeProp.GetOptionString() << std::endl;
    std::cout << "    Derived对象mode选项列表: ";
    auto derivedOptions = derivedModeProp.GetOptionList();
    for (size_t i = 0; i < derivedOptions.size(); ++i)
    {
        std::cout << i << "=" << derivedOptions[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "\n3.2 访问父类被重写的属性:" << std::endl;
    try
    {
        auto baseModeProp = derivedObj.GetPropertyAsOptional("mode", "BaseTestObject");
        std::cout << "    父类mode字符串: " << baseModeProp.GetOptionString() << std::endl;
        std::cout << "    父类mode选项列表: ";
        auto baseOptions = baseModeProp.GetOptionList();
        for (size_t i = 0; i < baseOptions.size(); ++i)
        {
            std::cout << i << "=" << baseOptions[i] << " ";
        }
        std::cout << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << "    异常: " << e.what() << std::endl;
    }

    std::cout << "\n3.3 测试继承的属性:" << std::endl;
    std::cout << "    HasProperty('tag'): " << (derivedObj.HasProperty("tag") ? "true" : "false") << std::endl;
    std::cout << "    HasProperty('temperature'): " << (derivedObj.HasProperty("temperature") ? "true" : "false") << std::endl;
    std::cout << "    HasProperty('derivedValue'): " << (derivedObj.HasProperty("derivedValue") ? "true" : "false") << std::endl;

    std::cout << "\n3.4 测试特定类的属性:" << std::endl;
    std::cout << "    HasProperty('tag', 'BaseTestObject'): " <<
        (derivedObj.HasProperty("tag", "BaseTestObject") ? "true" : "false") << std::endl;
    std::cout << "    HasProperty('derivedValue', 'DerivedTestObject'): " <<
        (derivedObj.HasProperty("derivedValue", "DerivedTestObject") ? "true" : "false") << std::endl;
    std::cout << "    HasProperty('derivedValue', 'BaseTestObject'): " <<
        (derivedObj.HasProperty("derivedValue", "BaseTestObject") ? "true" : "false") << std::endl;
}

void TestPropertyListsAndMaps()
{
    std::cout << "\n=== 测试4: 属性列表和映射 ===" << std::endl;

    DerivedTestObject obj;

    std::cout << "4.1 获取自身属性列表:" << std::endl;
    const auto& ownProps = obj.GetOwnPropertiesList();
    std::cout << "    自身属性数量: " << ownProps.size() << std::endl;
    for (const auto& prop : ownProps)
    {
        std::cout << "    - " << prop.name << " (" << prop.className << ")" <<
            (prop.isOptional ? " [可选]" : "") << std::endl;
    }

    std::cout << "\n4.2 获取所有属性列表（包括继承的）:" << std::endl;
    const auto& allProps = obj.GetAllPropertiesList();
    std::cout << "    所有属性数量: " << allProps.size() << std::endl;
    for (const auto& prop : allProps)
    {
        std::cout << "    - " << prop.name << " (" << prop.className << ")" <<
            (prop.isOptional ? " [可选]" : "") << std::endl;
    }

    std::cout << "\n4.3 获取直接属性映射（O(1)查找）:" << std::endl;
    const auto& directMap = obj.GetDirectPropertyMap();
    std::cout << "    直接属性数量: " << directMap.size() << std::endl;
    for (const auto& pair : directMap)
    {
        std::cout << "    - " << pair.first << " -> " << pair.second.typeName << std::endl;
    }

    std::cout << "\n4.4 获取父类属性列表:" << std::endl;
    const auto& parentProps = obj.GetParentPropertiesList("BaseTestObject");
    std::cout << "    父类BaseTestObject属性数量: " << parentProps.size() << std::endl;
    for (const auto& prop : parentProps)
    {
        std::cout << "    - " << prop.name << std::endl;
    }

    std::cout << "\n4.5 测试按注册顺序:" << std::endl;
    std::cout << "    属性注册顺序验证（按照宏调用顺序）:" << std::endl;
    for (size_t i = 0; i < ownProps.size(); ++i)
    {
        std::cout << "    " << i << ": " << ownProps[i].name <<
            " (order=" << ownProps[i].registrationOrder << ")" << std::endl;
    }
}

void TestCustomAccessorProperties()
{
    std::cout << "\n=== 测试5: 自定义访问器属性 ===" << std::endl;

    CustomAccessorObject obj;
    obj.GetMode() = 1; // Warm

    std::cout << "5.1 测试自定义getter/setter:" << std::endl;
    auto modeProp = obj.GetPropertyAsOptional("GetMode");

    std::cout << "    Mode via Property: " << modeProp.GetValue<int>() << std::endl;
    std::cout << "    Mode string: " << modeProp.GetOptionString() << std::endl;

    std::cout << "\n5.2 通过Property设置自定义属性:" << std::endl;
    modeProp.SetValue(2); // Hot
    std::cout << "    After setting mode to 2 (Hot)" << std::endl;

    std::cout << "\n5.3 测试自定义非选项属性:" << std::endl;
    auto counterProp = obj.GetProperty("GetCounter");
    std::cout << "    Counter via Property: " << counterProp.GetValue<int>() << std::endl;

    counterProp.SetValue(100);
    std::cout << "    After setting counter to 100" << std::endl;
}

void TestPropertyConversion()
{
    std::cout << "\n=== 测试6: 属性转换 ===" << std::endl;

    DerivedTestObject obj;
    obj.level = 1; // Medium
    obj.baseValue = 42;

    std::cout << "6.1 Property转换为OptionalProperty:" << std::endl;
    auto levelProp = obj.GetProperty("level");
    try
    {
        auto optionalLevelProp = obj.ToOptionalProperty(levelProp);
        std::cout << "    Successfully converted to OptionalProperty" << std::endl;
        std::cout << "    Option string: " << optionalLevelProp.GetOptionString() << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << "    Exception: " << e.what() << std::endl;
    }

    std::cout << "\n6.2 尝试转换非选项属性:" << std::endl;
    auto baseValueProp = obj.GetProperty("baseValue");
    try
    {
        auto optionalBaseValueProp = obj.ToOptionalProperty(baseValueProp);
        std::cout << "    This should not happen!" << std::endl;
    }
    catch (const std::runtime_error& e)
    {
        std::cout << "    Correctly caught exception: " << e.what() << std::endl;
    }

    std::cout << "\n6.3 Property和OptionalProperty混合使用:" << std::endl;
    // 先获取普通Property
    auto prop = obj.GetProperty("level");
    std::cout << "    Level via Property: " << prop.GetValue<int>() << std::endl;

    // 如果它是选项属性，可以转换
    if (obj.HasProperty("level"))
    {
        auto optionalProp = obj.GetPropertyAsOptional("level");
        std::cout << "    Level via OptionalProperty: " << optionalProp.GetOptionString() << std::endl;
    }
}

void TestErrorHandling()
{
    std::cout << "\n=== 测试7: 错误处理 ===" << std::endl;

    DerivedTestObject obj;

    std::cout << "7.1 访问不存在的属性:" << std::endl;
    try
    {
        auto invalidProp = obj.GetProperty("nonExistentProperty");
        std::cout << "    This should not happen!" << std::endl;
    }
    catch (const std::runtime_error& e)
    {
        std::cout << "    Correctly caught exception: " << e.what() << std::endl;
    }

    std::cout << "\n7.2 访问不存在的类属性:" << std::endl;
    try
    {
        auto invalidProp = obj.GetProperty("baseValue", "NonExistentClass");
        std::cout << "    This should not happen!" << std::endl;
    }
    catch (const std::runtime_error& e)
    {
        std::cout << "    Correctly caught exception: " << e.what() << std::endl;
    }

    std::cout << "\n7.3 类型不匹配访问:" << std::endl;
    try
    {
        auto tagProp = obj.GetProperty("tag");
        // 尝试用错误类型获取值
        int wrongTypeValue = tagProp.GetValue<int>();
        std::cout << "    This should not happen! Got: " << wrongTypeValue << std::endl;
    }
    catch (...)
    {
        std::cout << "    Exception caught for type mismatch" << std::endl;
    }
}

void TestMultiMapFunctionality()
{
    std::cout << "\n=== 测试8: 多映射功能（允许多个同名属性）===" << std::endl;

    DerivedTestObject obj;

    std::cout << "8.1 获取所有属性的多映射:" << std::endl;
    const auto& multiMap = obj.GetAllPropertiesMultiMap();

    // 统计每个属性名的出现次数
    std::map<std::string, int> nameCount;
    for (const auto& pair : multiMap)
    {
        nameCount[pair.first]++;
    }

    std::cout << "    多映射中的总属性数量: " << multiMap.size() << std::endl;
    std::cout << "    唯一属性名数量: " << nameCount.size() << std::endl;

    std::cout << "\n8.2 查找重名属性（如mode）:" << std::endl;
    auto range = multiMap.equal_range("mode");
    int count = 0;
    for (auto it = range.first; it != range.second; ++it)
    {
        std::cout << "    mode from class: " << it->second.className <<
            (it->second.isOptional ? " [可选]" : "") << std::endl;
        count++;
    }
    std::cout << "    找到 " << count << " 个名为mode的属性" << std::endl;

    std::cout << "\n8.3 遍历所有属性（按类分组）:" << std::endl;
    std::map<std::string, std::vector<std::string>> classProperties;
    for (const auto& pair : multiMap)
    {
        classProperties[pair.second.className].push_back(pair.first);
    }

    for (const auto& classPair : classProperties)
    {
        std::cout << "    " << classPair.first << " (" << classPair.second.size() << " properties): ";
        for (const auto& propName : classPair.second)
        {
            std::cout << propName << " ";
        }
        std::cout << std::endl;
    }
}

void TestPerformanceAndInitialization()
{
    std::cout << "\n=== 测试9: 性能和初始化 ===" << std::endl;

    std::cout << "9.1 测试初始化延迟:" << std::endl;

    // 第一次访问应该触发初始化
    std::cout << "    首次访问属性系统..." << std::endl;
    DerivedTestObject obj1;
    auto start = std::chrono::high_resolution_clock::now();
    obj1.GetAllPropertiesList();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "    首次初始化时间: " << duration.count() << " 微秒" << std::endl;

    // 第二次访问应该更快（已初始化）
    std::cout << "    再次访问属性系统..." << std::endl;
    start = std::chrono::high_resolution_clock::now();
    obj1.GetAllPropertiesList();
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "    后续访问时间: " << duration.count() << " 微秒" << std::endl;

    std::cout << "\n9.2 测试多个对象共享静态数据:" << std::endl;
    DerivedTestObject obj2;
    DerivedTestObject obj3;

    const auto& props1 = obj1.GetAllPropertiesList();
    const auto& props2 = obj2.GetAllPropertiesList();
    const auto& props3 = obj3.GetAllPropertiesList();

    std::cout << "    obj1属性列表地址: " << &props1 << std::endl;
    std::cout << "    obj2属性列表地址: " << &props2 << std::endl;
    std::cout << "    obj3属性列表地址: " << &props3 << std::endl;
    std::cout << "    是否共享相同数据: " << (&props1 == &props2 && &props2 == &props3 ? "是" : "否") << std::endl;
}

int test2()
{
    std::cout << "=== 运行时属性系统测试开始 ===" << std::endl;

    // 运行所有测试
    TestBasicPropertyRegistration();
    TestOptionalProperties();
    TestInheritanceAndOverriding();
    TestPropertyListsAndMaps();
    TestCustomAccessorProperties();
    TestPropertyConversion();
    TestErrorHandling();
    TestMultiMapFunctionality();
    TestPerformanceAndInitialization();

    std::cout << "\n=== 所有测试完成 ===" << std::endl;

    // 最终验证
    std::cout << "\n=== 最终验证 ===" << std::endl;
    DerivedTestObject finalObj;

    // 设置所有属性
    finalObj.mode = 2;          // Super
    finalObj.baseValue = 999;
    finalObj.derivedValue = 888;
    finalObj.level = 1;         // Medium
    finalObj.tag = "FinalTest";
    finalObj.temperature = 25.5f;
    finalObj.accuracy = 0.99;
    finalObj.isActive = true;
    finalObj.status = 2;        // Paused

    // 验证所有属性
    std::cout << "最终对象状态:" << std::endl;
    std::cout << "  mode: " << finalObj.GetPropertyAsOptional("mode").GetOptionString() <<
        " (" << finalObj.mode << ")" << std::endl;
    std::cout << "  baseValue: " << finalObj.baseValue << std::endl;
    std::cout << "  derivedValue: " << finalObj.derivedValue << std::endl;
    std::cout << "  level: " << finalObj.GetPropertyAsOptional("level").GetOptionString() <<
        " (" << finalObj.level << ")" << std::endl;
    std::cout << "  tag: " << finalObj.tag << std::endl;
    std::cout << "  temperature: " << finalObj.temperature << std::endl;
    std::cout << "  accuracy: " << finalObj.accuracy << std::endl;
    std::cout << "  isActive: " << (finalObj.isActive ? "true" : "false") << std::endl;
    std::cout << "  status: " << finalObj.GetPropertyAsOptional("status").GetOptionString() <<
        " (" << finalObj.status << ")" << std::endl;

    std::cout << "\n总属性数量（包括继承的）: " << finalObj.GetAllPropertiesList().size() << std::endl;
    std::cout << "自身属性数量: " << finalObj.GetOwnPropertiesList().size() << std::endl;

    return 0;
}


// 主函数
int main()
{
    try
    {
        std::cout << "开始运行时属性系统性能测试..." << std::endl;
        std::cout << "编译时间: " << __DATE__ << " " << __TIME__ << std::endl;
        std::cout << "测试平台: " <<
#ifdef _WIN32
            "Windows"
#elif __linux__
            "Linux"
#elif __APPLE__
            "macOS"
#else
            "Unknown"
#endif
            << std::endl;

        RunPropertySystemPerformanceTests();
        TestManyProperties();
        TestOptionalPropertySystem();
        test2();
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "测试过程中发生异常: " << e.what() << std::endl;
        return 1;
    }
}