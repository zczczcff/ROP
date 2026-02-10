#pragma once

#include <string>
#include <memory>
#include <stdexcept>

namespace SharedLibTest
{

// 跨平台动态库加载器
class DynamicLoader
{
public:
    // 库句柄类型（跨平台）
    using LibraryHandle = void*;

    DynamicLoader();
    ~DynamicLoader();

    // 禁止拷贝
    DynamicLoader(const DynamicLoader&) = delete;
    DynamicLoader& operator=(const DynamicLoader&) = delete;

    // 加载动态库
    bool Load(const std::string& libraryPath);

    // 卸载动态库
    void Unload();

    // 检查是否已加载
    bool IsLoaded() const;

    // 获取函数地址
    template<typename FuncType>
    FuncType* GetFunction(const std::string& functionName)
    {
        if (!m_loaded)
        {
            throw std::runtime_error("Library not loaded");
        }

        void* funcPtr = GetFunctionInternal(functionName);
        if (!funcPtr)
        {
            throw std::runtime_error("Function not found: " + functionName);
        }

        return reinterpret_cast<FuncType*>(funcPtr);
    }

    // 获取最后的错误信息
    std::string GetLastError() const;

private:
    void* GetFunctionInternal(const std::string& functionName);

    LibraryHandle m_handle;
    bool m_loaded;
    std::string m_lastError;
};

// RAII 风格的库加载器
class ScopedLibraryLoader
{
public:
    explicit ScopedLibraryLoader(const std::string& libraryPath);
    ~ScopedLibraryLoader();

    // 禁止拷贝
    ScopedLibraryLoader(const ScopedLibraryLoader&) = delete;
    ScopedLibraryLoader& operator=(const ScopedLibraryLoader&) = delete;

    // 允许移动
    ScopedLibraryLoader(ScopedLibraryLoader&& other) noexcept;
    ScopedLibraryLoader& operator=(ScopedLibraryLoader&& other) noexcept;

    bool IsLoaded() const;
    DynamicLoader* GetLoader();
    const std::string& GetPath() const;

private:
    std::unique_ptr<DynamicLoader> m_loader;
    std::string m_libraryPath;
};

// 平台特定的实现细节
#ifdef _WIN32
    #define LIBRARY_EXTENSION ".dll"
    #define LIBRARY_PREFIX ""
#elif __APPLE__
    #define LIBRARY_EXTENSION ".dylib"
    #define LIBRARY_PREFIX "lib"
#else
    #define LIBRARY_EXTENSION ".so"
    #define LIBRARY_PREFIX "lib"
#endif

} // namespace SharedLibTest
