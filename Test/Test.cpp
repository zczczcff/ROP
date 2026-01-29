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
    CUSTOM_TYPE,
    OPTIONAL
};

// 测试基类
class TestBaseObject : public ROP::PropertyObject<TestPropertyType>
{
    DECLARE_OBJECT(TestPropertyType, TestBaseObject)
    registrar
        .RegisterProperty(
            TestPropertyType::INT, "baseIntValue", &TestBaseObject::baseIntValue,
            "基类整数属性")
        .RegisterProperty(
            TestPropertyType::FLOAT, "baseFloatValue", &TestBaseObject::baseFloatValue,
            "基类浮点数属性")
        .RegisterProperty(
            TestPropertyType::STRING, "baseStringValue", &TestBaseObject::baseStringValue,
            "基类字符串属性");
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
    registrar
        .RegisterProperty(
            TestPropertyType::INT, "intValue1", &TestDerivedObject::intValue1,
            "派生类整数属性1")
        .RegisterProperty(
            TestPropertyType::INT, "intValue2", &TestDerivedObject::intValue2,
            "派生类整数属性2")
        .RegisterProperty(
            TestPropertyType::INT, "intValue3", &TestDerivedObject::intValue3,
            "派生类整数属性3")
        .RegisterProperty(
            TestPropertyType::FLOAT, "floatValue1", &TestDerivedObject::floatValue1,
            "派生类浮点数属性1")
        .RegisterProperty(
            TestPropertyType::FLOAT, "floatValue2", &TestDerivedObject::floatValue2,
            "派生类浮点数属性2")
        .RegisterProperty(
            TestPropertyType::DOUBLE, "doubleValue", &TestDerivedObject::doubleValue,
            "派生类双精度属性")
        .RegisterProperty(
            TestPropertyType::STRING, "stringValue", &TestDerivedObject::stringValue,
            "派生类字符串属性")
        .RegisterProperty(
            TestPropertyType::BOOL, "boolValue", &TestDerivedObject::boolValue,
            "派生类布尔属性");
    END_DECLARE_OBJECT(TestPropertyType, TestDerivedObject, TestBaseObject)

public:
    TestDerivedObject() :
        intValue1(0), intValue2(0), intValue3(0),
        floatValue1(0.0f), floatValue2(0.0f),
        doubleValue(0.0),
        stringValue(""),
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
    registrar
        .RegisterProperty(
            TestPropertyType::INT, "customInt",
            static_cast<void (TestCustomAccessorObject::*)(int&)>(&TestCustomAccessorObject::SetCustomInt),
            static_cast<int& (TestCustomAccessorObject::*)()>(&TestCustomAccessorObject::GetCustomInt),
            "自定义整数属性")
        .RegisterProperty(
            TestPropertyType::STRING, "customString",
            static_cast<void (TestCustomAccessorObject::*)(std::string&)>(&TestCustomAccessorObject::SetCustomString),
            static_cast<std::string& (TestCustomAccessorObject::*)()>(&TestCustomAccessorObject::GetCustomString),
            "自定义字符串属性")
        .RegisterProperty(
            TestPropertyType::INT, "directIntValue", &TestCustomAccessorObject::directIntValue,
            "直接整数属性");
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
        ROP::Property<TestPropertyType> customIntProp = obj.GetProperty("customInt");

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


        for (int i = 0; i < TEST_ITERATIONS; ++i)
        {
            // 缓存Property对象（最佳实践）
            ROP::Property<TestPropertyType> cachedProp1 = obj.GetProperty("intValue1");
            ROP::Property<TestPropertyType> cachedProp2 = obj.GetProperty("intValue2");

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

    // ==================== 测试5.5: 属性查找性能测试 ====================
    {
        std::cout << "\n" << std::string(50, '-') << std::endl;
        std::cout << "测试5.5: 属性查找性能测试 (GetProperty()查找开销)" << std::endl;
        std::cout << std::string(50, '-') << std::endl;

        TestDerivedObject obj;
        obj.intValue1 = 42;
        obj.floatValue1 = 3.14f;
        obj.stringValue = "test_string";
        obj.boolValue = true;

        const int LOOKUP_ITERATIONS = 1000000; // 100万次查找

        // 1. 直接访问性能基准
        {
            auto start = Clock::now();

            int sum = 0;
            for (int i = 0; i < LOOKUP_ITERATIONS; ++i)
            {
                // 直接访问多个不同类型的属性
                sum += obj.intValue1;
                sum += static_cast<int>(obj.floatValue1);
                sum += obj.stringValue.length();
                sum += (obj.boolValue ? 1 : 0);
            }

            auto end = Clock::now();
            auto duration = std::chrono::duration_cast<Duration>(end - start);

            std::cout << "1. 直接访问基准 (4个属性/迭代):" << std::endl;
            std::cout << "   耗时: " << duration.count() << " ns" << std::endl;
            std::cout << "   每次迭代平均耗时: " << duration.count() / static_cast<double>(LOOKUP_ITERATIONS) << " ns" << std::endl;
            std::cout << "   每次属性访问平均耗时: " << duration.count() / (LOOKUP_ITERATIONS * 4.0) << " ns" << std::endl;
            std::cout << "   验证和: " << sum << std::endl;

            double directTimePerAccess = duration.count() / (LOOKUP_ITERATIONS * 4.0);
            std::cout << "   基准时间/属性: " << directTimePerAccess << " ns" << std::endl;
        }

        // 2. 每次调用GetProperty()的性能（查找开销）
        {
            auto start = Clock::now();

            int sum = 0;
            for (int i = 0; i < LOOKUP_ITERATIONS; ++i)
            {
                // 每次迭代都调用GetProperty()（包含查找开销）
                ROP::Property<TestPropertyType> intProp = obj.GetProperty("intValue1");
                ROP::Property<TestPropertyType> floatProp = obj.GetProperty("floatValue1");
                ROP::Property<TestPropertyType> stringProp = obj.GetProperty("stringValue");
                ROP::Property<TestPropertyType> boolProp = obj.GetProperty("boolValue");

                // 通过Property获取值
                sum += intProp.GetValue<int>();
                sum += static_cast<int>(floatProp.GetValue<float>());
                sum += stringProp.GetValue<std::string>().length();
                sum += (boolProp.GetValue<bool>() ? 1 : 0);
            }

            auto end = Clock::now();
            auto duration = std::chrono::duration_cast<Duration>(end - start);

            std::cout << "\n2. GetProperty()调用 (每次迭代都查找):" << std::endl;
            std::cout << "   耗时: " << duration.count() << " ns" << std::endl;
            std::cout << "   每次迭代平均耗时: " << duration.count() / static_cast<double>(LOOKUP_ITERATIONS) << " ns" << std::endl;
            std::cout << "   每次属性访问平均耗时: " << duration.count() / (LOOKUP_ITERATIONS * 4.0) << " ns" << std::endl;
            std::cout << "   验证和: " << sum << std::endl;

            double lookupTimePerAccess = duration.count() / (LOOKUP_ITERATIONS * 4.0);
            std::cout << "   查找开销倍数: N/A (这是总开销)" << std::endl;
        }

        // 3. 仅测试GetProperty()调用（不获取值）
        {
            auto start = Clock::now();

            int sum = 0;
            for (int i = 0; i < LOOKUP_ITERATIONS; ++i)
            {
                // 仅调用GetProperty()，不获取值
                ROP::Property<TestPropertyType> intProp = obj.GetProperty("intValue1");
                ROP::Property<TestPropertyType> floatProp = obj.GetProperty("floatValue1");
                ROP::Property<TestPropertyType> stringProp = obj.GetProperty("stringValue");
                ROP::Property<TestPropertyType> boolProp = obj.GetProperty("boolValue");

                // 累加属性指针地址的低位，防止编译器优化
                sum += reinterpret_cast<uintptr_t>(&intProp) & 0xFF;
            }

            auto end = Clock::now();
            auto duration = std::chrono::duration_cast<Duration>(end - start);

            std::cout << "\n3. 仅GetProperty()调用 (不获取值):" << std::endl;
            std::cout << "   耗时: " << duration.count() << " ns" << std::endl;
            std::cout << "   每次GetProperty()调用平均耗时: " << duration.count() / (LOOKUP_ITERATIONS * 4.0) << " ns" << std::endl;
            std::cout << "   验证和: " << sum << std::endl;
        }

        // 4. 分解测试：单独测试查找开销和访问开销
        {
            std::cout << "\n4. 分解测试:" << std::endl;

            // 4.1 缓存Property对象后的访问开销
            {
                auto start = Clock::now();

                int sum = 0;
                // 缓存Property对象
                ROP::Property<TestPropertyType> intProp = obj.GetProperty("intValue1");
                ROP::Property<TestPropertyType> floatProp = obj.GetProperty("floatValue1");
                ROP::Property<TestPropertyType> stringProp = obj.GetProperty("stringValue");
                ROP::Property<TestPropertyType> boolProp = obj.GetProperty("boolValue");

                for (int i = 0; i < LOOKUP_ITERATIONS; ++i)
                {
                    // 仅通过缓存的Property对象获取值
                    sum += intProp.GetValue<int>();
                    sum += static_cast<int>(floatProp.GetValue<float>());
                    sum += stringProp.GetValue<std::string>().length();
                    sum += (boolProp.GetValue<bool>() ? 1 : 0);
                }

                auto end = Clock::now();
                auto duration = std::chrono::duration_cast<Duration>(end - start);

                std::cout << "   a) 缓存Property后仅访问值:" << std::endl;
                std::cout << "      耗时: " << duration.count() << " ns" << std::endl;
                std::cout << "      每次属性访问平均耗时: " << duration.count() / (LOOKUP_ITERATIONS * 4.0) << " ns" << std::endl;

                double cachedAccessTime = duration.count() / (LOOKUP_ITERATIONS * 4.0);
                std::cout << "      访问开销/属性: " << cachedAccessTime << " ns" << std::endl;
            }

            // 4.2 仅查找开销（估算）
            {
                // 通过从完整时间中减去访问时间来估算查找开销
                // 完整时间 = 查找时间 + 访问时间
                // 查找时间 ≈ 完整时间 - 访问时间

                // 完整时间（从测试2获取，但需要重新计算或使用之前的结果）
                // 这里我们重新运行一个简化的测试
                const int ITERATIONS = LOOKUP_ITERATIONS / 10; // 减少迭代次数

                auto start = Clock::now();

                int sum = 0;
                for (int i = 0; i < ITERATIONS; ++i)
                {
                    ROP::Property<TestPropertyType> intProp = obj.GetProperty("intValue1");
                    sum += reinterpret_cast<uintptr_t>(&intProp) & 0xFF;
                }

                auto end = Clock::now();
                auto duration = std::chrono::duration_cast<Duration>(end - start);

                double lookupTimePerCall = duration.count() / (ITERATIONS * 1.0);

                std::cout << "\n   b) 仅GetProperty()查找开销估算:" << std::endl;
                std::cout << "      每次GetProperty()调用: " << lookupTimePerCall << " ns" << std::endl;
                std::cout << "      其中大部分是哈希表查找开销" << std::endl;
            }
        }

        // 5. 不同类型属性的查找性能比较
        {
            std::cout << "\n5. 不同类型属性的查找性能比较:" << std::endl;

            const int ITERATIONS = LOOKUP_ITERATIONS / 10;

            // 测试直接属性（自身属性）
            {
                auto start = Clock::now();

                int sum = 0;
                for (int i = 0; i < ITERATIONS; ++i)
                {
                    ROP::Property<TestPropertyType> prop = obj.GetProperty("intValue1");
                    sum += reinterpret_cast<uintptr_t>(&prop) & 0xFF;
                }

                auto end = Clock::now();
                auto duration = std::chrono::duration_cast<Duration>(end - start);

                std::cout << "   a) 直接属性 (intValue1): " << duration.count() / (ITERATIONS * 1.0) << " ns/查找" << std::endl;
            }

            // 测试继承属性
            {
                auto start = Clock::now();

                int sum = 0;
                for (int i = 0; i < ITERATIONS; ++i)
                {
                    ROP::Property<TestPropertyType> prop = obj.GetProperty("baseIntValue");
                    sum += reinterpret_cast<uintptr_t>(&prop) & 0xFF;
                }

                auto end = Clock::now();
                auto duration = std::chrono::duration_cast<Duration>(end - start);

                std::cout << "   b) 继承属性 (baseIntValue): " << duration.count() / (ITERATIONS * 1.0) << " ns/查找" << std::endl;
            }

            // 测试不存在的属性
            {
                auto start = Clock::now();

                int sum = 0;
                for (int i = 0; i < ITERATIONS; ++i)
                {
                    ROP::Property<TestPropertyType> prop = obj.GetProperty("nonExistentProperty");
                    sum += reinterpret_cast<uintptr_t>(&prop) & 0xFF;
                }

                auto end = Clock::now();
                auto duration = std::chrono::duration_cast<Duration>(end - start);

                std::cout << "   c) 不存在属性: " << duration.count() / (ITERATIONS * 1.0) << " ns/查找" << std::endl;
                std::cout << "      (查找失败也需要遍历多映射表)" << std::endl;
            }
        }

        // 6. 性能总结和优化建议
        {
            std::cout << "\n6. 属性查找性能总结和建议:" << std::endl;
            std::cout << std::string(40, '-') << std::endl;

            std::cout << "关键发现:" << std::endl;
            std::cout << "1. GetProperty()调用主要开销在哈希表查找" << std::endl;
            std::cout << "2. 直接属性查找比继承属性查找稍快" << std::endl;
            std::cout << "3. 查找不存在属性也有开销（需要检查整个多映射表）" << std::endl;
            std::cout << "4. 缓存Property对象可消除查找开销" << std::endl;

            std::cout << "\n性能对比 (估算):" << std::endl;
            std::cout << "  - 直接访问: ~1-5 ns/属性" << std::endl;
            std::cout << "  - 缓存Property访问: ~10-30 ns/属性" << std::endl;
            std::cout << "  - GetProperty()查找 + 访问: ~50-200 ns/属性" << std::endl;

            std::cout << "\n优化建议:" << std::endl;
            std::cout << "1. 性能关键路径: 避免在循环中调用GetProperty()" << std::endl;
            std::cout << "2. 最佳实践: 在初始化阶段缓存频繁访问的属性" << std::endl;
            std::cout << "3. 批量操作: 如果需要访问多个属性，批量获取Property对象" << std::endl;
            std::cout << "4. 模式选择:" << std::endl;
            std::cout << "   - 静态访问: 直接访问成员变量" << std::endl;
            std::cout << "   - 动态但已知属性: 缓存Property对象" << std::endl;
            std::cout << "   - 完全动态: 使用GetProperty()" << std::endl;

            std::cout << "\n代码示例:" << std::endl;
            std::cout << "  // 反模式: 在循环中频繁调用GetProperty()" << std::endl;
            std::cout << "  for (int i = 0; i < N; ++i) {" << std::endl;
            std::cout << "      obj.GetProperty(\"name\").SetValue(i);  // 每次都有查找开销" << std::endl;
            std::cout << "  }" << std::endl;

            std::cout << "\n  // 最佳实践: 缓存Property对象" << std::endl;
            std::cout << "  auto nameProp = obj.GetProperty(\"name\");  // 一次性查找" << std::endl;
            std::cout << "  for (int i = 0; i < N; ++i) {" << std::endl;
            std::cout << "      nameProp.SetValue(i);  // 无查找开销" << std::endl;
            std::cout << "  }" << std::endl;
        }
    }
}

// ==================== 设计有较多属性的类层次结构 ====================

// 基类：包含20个属性
class LargeBaseObject : public ROP::PropertyObject<TestPropertyType>
{
    DECLARE_OBJECT(TestPropertyType, LargeBaseObject)
    registrar
        // 整数属性组 (10个)
        .RegisterProperty(TestPropertyType::INT, "base_int_1", &LargeBaseObject::base_int_1, "基础整数1")
        .RegisterProperty(TestPropertyType::INT, "base_int_2", &LargeBaseObject::base_int_2, "基础整数2")
        .RegisterProperty(TestPropertyType::INT, "base_int_3", &LargeBaseObject::base_int_3, "基础整数3")
        .RegisterProperty(TestPropertyType::INT, "base_int_4", &LargeBaseObject::base_int_4, "基础整数4")
        .RegisterProperty(TestPropertyType::INT, "base_int_5", &LargeBaseObject::base_int_5, "基础整数5")
        .RegisterProperty(TestPropertyType::INT, "base_int_6", &LargeBaseObject::base_int_6, "基础整数6")
        .RegisterProperty(TestPropertyType::INT, "base_int_7", &LargeBaseObject::base_int_7, "基础整数7")
        .RegisterProperty(TestPropertyType::INT, "base_int_8", &LargeBaseObject::base_int_8, "基础整数8")
        .RegisterProperty(TestPropertyType::INT, "base_int_9", &LargeBaseObject::base_int_9, "基础整数9")
        .RegisterProperty(TestPropertyType::INT, "base_int_10", &LargeBaseObject::base_int_10, "基础整数10")

        // 浮点数属性组 (5个)
        .RegisterProperty(TestPropertyType::FLOAT, "base_float_1", &LargeBaseObject::base_float_1, "基础浮点数1")
        .RegisterProperty(TestPropertyType::FLOAT, "base_float_2", &LargeBaseObject::base_float_2, "基础浮点数2")
        .RegisterProperty(TestPropertyType::FLOAT, "base_float_3", &LargeBaseObject::base_float_3, "基础浮点数3")
        .RegisterProperty(TestPropertyType::FLOAT, "base_float_4", &LargeBaseObject::base_float_4, "基础浮点数4")
        .RegisterProperty(TestPropertyType::FLOAT, "base_float_5", &LargeBaseObject::base_float_5, "基础浮点数5")

        // 字符串属性组 (3个)
        .RegisterProperty(TestPropertyType::STRING, "base_string_1", &LargeBaseObject::base_string_1, "基础字符串1")
        .RegisterProperty(TestPropertyType::STRING, "base_string_2", &LargeBaseObject::base_string_2, "基础字符串2")
        .RegisterProperty(TestPropertyType::STRING, "base_string_3", &LargeBaseObject::base_string_3, "基础字符串3")

        // 布尔属性组 (2个)
        .RegisterProperty(TestPropertyType::BOOL, "base_bool_1", &LargeBaseObject::base_bool_1, "基础布尔1")
        .RegisterProperty(TestPropertyType::BOOL, "base_bool_2", &LargeBaseObject::base_bool_2, "基础布尔2");
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
    registrar
        // 自定义访问器属性 (3个)
        .RegisterProperty(
            TestPropertyType::INT, "derivedInt1",
            static_cast<void (MiddleDerivedObject::*)(int&)>(&MiddleDerivedObject::SetDerivedInt1),
            static_cast<int& (MiddleDerivedObject::*)()>(&MiddleDerivedObject::GetDerivedInt1),
            "派生自定义整数1")
        .RegisterProperty(
            TestPropertyType::FLOAT, "derivedFloat1",
            static_cast<void (MiddleDerivedObject::*)(float&)>(&MiddleDerivedObject::SetDerivedFloat1),
            static_cast<float& (MiddleDerivedObject::*)()>(&MiddleDerivedObject::GetDerivedFloat1),
            "派生自定义浮点数1")
        .RegisterProperty(
            TestPropertyType::STRING, "derivedString1",
            static_cast<void (MiddleDerivedObject::*)(std::string&)>(&MiddleDerivedObject::SetDerivedString1),
            static_cast<std::string& (MiddleDerivedObject::*)()>(&MiddleDerivedObject::GetDerivedString1),
            "派生自定义字符串1")

        // 双精度属性组 (5个)
        .RegisterProperty(TestPropertyType::DOUBLE, "derived_double_1", &MiddleDerivedObject::derived_double_1, "派生双精度1")
        .RegisterProperty(TestPropertyType::DOUBLE, "derived_double_2", &MiddleDerivedObject::derived_double_2, "派生双精度2")
        .RegisterProperty(TestPropertyType::DOUBLE, "derived_double_3", &MiddleDerivedObject::derived_double_3, "派生双精度3")
        .RegisterProperty(TestPropertyType::DOUBLE, "derived_double_4", &MiddleDerivedObject::derived_double_4, "派生双精度4")
        .RegisterProperty(TestPropertyType::DOUBLE, "derived_double_5", &MiddleDerivedObject::derived_double_5, "派生双精度5")

        // 整数属性组 (4个)
        .RegisterProperty(TestPropertyType::INT, "derived_int_1", &MiddleDerivedObject::derived_int_1, "派生整数1")
        .RegisterProperty(TestPropertyType::INT, "derived_int_2", &MiddleDerivedObject::derived_int_2, "派生整数2")
        .RegisterProperty(TestPropertyType::INT, "derived_int_3", &MiddleDerivedObject::derived_int_3, "派生整数3")
        .RegisterProperty(TestPropertyType::INT, "derived_int_4", &MiddleDerivedObject::derived_int_4, "派生整数4")

        // 其他类型属性 (3个)
        .RegisterProperty(TestPropertyType::BOOL, "derived_bool_1", &MiddleDerivedObject::derived_bool_1, "派生布尔1")
        .RegisterProperty(TestPropertyType::FLOAT, "derived_float_1", &MiddleDerivedObject::derived_float_1, "派生浮点数1")
        .RegisterProperty(TestPropertyType::STRING, "derived_string_1", &MiddleDerivedObject::derived_string_1, "派生字符串1");
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
        baseObj.EnsurePropertySystemInitialized();
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
        middleObj.EnsurePropertySystemInitialized();
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
}

// ==================== 测试选项属性系统 ====================

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
    registrar
        .RegisterOptionalProperty(
            MyObjectType::OPTIONAL, "mode", &BaseObject::mode,
            { "Off", "On", "Auto" },
            "工作模式")
        .RegisterProperty(
            MyObjectType::INT, "value", &BaseObject::value,
            "基础值")
        .RegisterProperty(
            MyObjectType::STRING, "tag", &BaseObject::tag,
            "标签");
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
    registrar
        .RegisterOptionalProperty(
            MyObjectType::OPTIONAL, "mode", &DerivedObject::mode,
            { "Disabled", "Enabled", "Super" },
            "派生类工作模式")
        .RegisterOptionalProperty(
            MyObjectType::OPTIONAL, "level", &DerivedObject::level,
            { "Low", "Medium", "High" },
            "等级");
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

    // 测试level属性
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

// ==================== 综合测试 ====================

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
    registrar
        .RegisterOptionalProperty(
            TestObjectType::OPTIONAL, "mode", &BaseTestObject::mode,
            { "Off", "On", "Auto" },
            "工作模式")
        .RegisterProperty(
            TestObjectType::INT, "baseValue", &BaseTestObject::baseValue,
            "基础值")
        .RegisterProperty(
            TestObjectType::STRING, "tag", &BaseTestObject::tag,
            "标签")
        .RegisterProperty(
            TestObjectType::FLOAT, "temperature", &BaseTestObject::temperature,
            "温度")
        .RegisterOptionalProperty(
            TestObjectType::OPTIONAL, "status", &BaseTestObject::status,
            { "Idle", "Running", "Paused", "Stopped" },
            "状态");
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
    registrar
        .RegisterOptionalProperty(
            TestObjectType::OPTIONAL, "mode", &DerivedTestObject::mode,
            { "Disabled", "Enabled", "Super" },
            "派生类工作模式")
        .RegisterProperty(
            TestObjectType::INT, "derivedValue", &DerivedTestObject::derivedValue,
            "派生值")
        .RegisterOptionalProperty(
            TestObjectType::OPTIONAL, "level", &DerivedTestObject::level,
            { "Low", "Medium", "High" },
            "等级")
        .RegisterProperty(
            TestObjectType::DOUBLE, "accuracy", &DerivedTestObject::accuracy,
            "精度")
        .RegisterProperty(
            TestObjectType::BOOL, "isActive", &DerivedTestObject::isActive,
            "是否激活");
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
    registrar
        // 使用自定义getter/setter
        .RegisterOptionalProperty(
            TestObjectType::OPTIONAL, "customMode",
            static_cast<void (CustomAccessorObject::*)(int&)>(&CustomAccessorObject::SetMode),
            static_cast<int& (CustomAccessorObject::*)()>(&CustomAccessorObject::GetMode),
            { "Cold", "Warm", "Hot" },
            "自定义模式")
        .RegisterProperty(
            TestObjectType::INT, "customCounter",
            static_cast<void (CustomAccessorObject::*)(int&)>(&CustomAccessorObject::SetCounter),
            static_cast<int& (CustomAccessorObject::*)()>(&CustomAccessorObject::GetCounter),
            "自定义计数器");
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

    std::cout << "\n1.3 获取属性描述:" << std::endl;
    std::cout << "    baseValue description: " << baseValueProp.GetDescription() << std::endl;
    std::cout << "    derivedValue description: " << derivedValueProp.GetDescription() << std::endl;

    std::cout << "\n1.4 设置属性值:" << std::endl;
    baseValueProp.SetValue(500);
    std::cout << "    baseValue after SetValue: " << obj.baseValue << std::endl;

    derivedValueProp.SetValue(800);
    std::cout << "    derivedValue after SetValue: " << obj.derivedValue << std::endl;
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
    auto baseModeProp = derivedObj.GetPropertyAsOptional("mode", "BaseTestObject");
    std::cout << "    父类mode字符串: " << baseModeProp.GetOptionString()
        << " (value: " << baseModeProp.GetValue<int>() << ")" << std::endl;

    std::cout << "    父类mode选项列表: ";
    auto baseOptions = baseModeProp.GetOptionList();
    for (size_t i = 0; i < baseOptions.size(); ++i)
    {
        std::cout << i << "=" << baseOptions[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "\n3.3 测试继承的属性:" << std::endl;
    std::cout << "    HasProperty('tag'): " << (derivedObj.HasProperty("tag") ? "true" : "false") << std::endl;
    std::cout << "    HasProperty('temperature'): " << (derivedObj.HasProperty("temperature") ? "true" : "false") << std::endl;
    std::cout << "    HasProperty('derivedValue'): " << (derivedObj.HasProperty("derivedValue") ? "true" : "false") << std::endl;
}

void TestPropertyListsAndMaps()
{
    std::cout << "\n=== 测试4: 属性列表和映射 ===" << std::endl;

    DerivedTestObject obj;
    obj.EnsurePropertySystemInitialized();

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
}

void TestCustomAccessorProperties()
{
    std::cout << "\n=== 测试5: 自定义访问器属性 ===" << std::endl;

    CustomAccessorObject obj;
    obj.GetMode() = 1; // Warm

    std::cout << "5.1 测试自定义getter/setter:" << std::endl;
    auto modeProp = obj.GetPropertyAsOptional("customMode");

    std::cout << "    Mode via Property: " << modeProp.GetValue<int>() << std::endl;
    std::cout << "    Mode string: " << modeProp.GetOptionString() << std::endl;

    std::cout << "\n5.2 通过Property设置自定义属性:" << std::endl;
    modeProp.SetValue(2); // Hot
    std::cout << "    After setting mode to 2 (Hot)" << std::endl;

    std::cout << "\n5.3 测试自定义非选项属性:" << std::endl;
    auto counterProp = obj.GetProperty("customCounter");
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
}

void TestErrorHandling()
{
    std::cout << "\n=== 测试7: 错误处理 ===" << std::endl;

    DerivedTestObject obj;

    std::cout << "7.1 访问不存在的属性:" << std::endl;
    auto invalidProp = obj.GetProperty("nonExistentProperty");
    std::cout << "    Property is valid: " << (invalidProp.IsValid() ? "true" : "false") << std::endl;

    std::cout << "\n7.2 访问不存在的类属性:" << std::endl;
    auto invalidClassProp = obj.GetProperty("baseValue", "NonExistentClass");
    std::cout << "    Property is valid: " << (invalidClassProp.IsValid() ? "true" : "false") << std::endl;

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
    obj.EnsurePropertySystemInitialized();

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

void TestGetAllPropertiesOrdered()
{
    std::cout << "\n=== 测试10: 获取有序属性列表 ===" << std::endl;

    DerivedTestObject obj;
    obj.EnsurePropertySystemInitialized();

    std::cout << "10.1 获取所有属性（按顺序：先子类后父类，每个类内按注册顺序）:" << std::endl;
    auto orderedProps = obj.GetAllPropertiesOrdered();
    std::cout << "    有序属性数量: " << orderedProps.size() << std::endl;

    for (size_t i = 0; i < orderedProps.size(); ++i)
    {
        auto& prop = orderedProps[i];
        std::cout << "    [" << i << "] " << prop.GetName()
            << " (类: " << prop.GetClassName() << ")"
            << (prop.GetDescription().empty() ? "" : " - " + prop.GetDescription()) << std::endl;
    }

    std::cout << "\n10.2 获取所有同名属性（按顺序）:" << std::endl;
    auto modeProps = obj.GetPropertiesByNameOrdered("mode");
    std::cout << "    名为'mode'的属性数量: " << modeProps.size() << std::endl;

    for (size_t i = 0; i < modeProps.size(); ++i)
    {
        auto& prop = modeProps[i];
        if (prop.IsValid())
        {
            std::cout << "    [" << i << "] " << prop.GetName()
                << " (类: " << prop.GetClassName()
                << ", 值: " << prop.GetValue<int>() << ")" << std::endl;
        }
    }
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
    TestGetAllPropertiesOrdered();

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
        std::cout << "开始运行时属性系统测试..." << std::endl;
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

        std::cout << "\n所有测试完成！" << std::endl;
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "测试过程中发生异常: " << e.what() << std::endl;
        return 1;
    }
}