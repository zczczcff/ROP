#include "SharedLibTest/DynamicLoader.h"

#ifdef _WIN32
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif

namespace SharedLibTest
{

DynamicLoader::DynamicLoader()
    : m_handle(nullptr)
    , m_loaded(false)
    , m_lastError()
{
}

DynamicLoader::~DynamicLoader()
{
    Unload();
}

bool DynamicLoader::Load(const std::string& libraryPath)
{
    if (m_loaded)
    {
        m_lastError = "Library already loaded";
        return false;
    }

#ifdef _WIN32
    // Windows: Use LoadLibrary
    m_handle = LoadLibraryA(libraryPath.c_str());
    if (!m_handle)
    {
        m_lastError = "Failed to load library";
        return false;
    }
#else
    // Linux/Mac: 使用 dlopen
    m_handle = dlopen(libraryPath.c_str(), RTLD_LAZY);
    if (!m_handle)
    {
        m_lastError = dlerror();
        return false;
    }
#endif

    m_loaded = true;
    m_lastError.clear();
    return true;
}

void DynamicLoader::Unload()
{
    if (m_loaded && m_handle)
    {
#ifdef _WIN32
        FreeLibrary(static_cast<HMODULE>(m_handle));
#else
        dlclose(m_handle);
#endif
        m_handle = nullptr;
        m_loaded = false;
    }
}

bool DynamicLoader::IsLoaded() const
{
    return m_loaded;
}

void* DynamicLoader::GetFunctionInternal(const std::string& functionName)
{
    if (!m_loaded)
    {
        return nullptr;
    }

#ifdef _WIN32
    return reinterpret_cast<void*>(GetProcAddress(
        static_cast<HMODULE>(m_handle), functionName.c_str()));
#else
    return dlsym(m_handle, functionName.c_str());
#endif
}

std::string DynamicLoader::GetLastError() const
{
    return m_lastError;
}

// ScopedLibraryLoader 实现
ScopedLibraryLoader::ScopedLibraryLoader(const std::string& libraryPath)
    : m_libraryPath(libraryPath)
{
    m_loader = std::make_unique<DynamicLoader>();
    m_loader->Load(libraryPath);
}

ScopedLibraryLoader::~ScopedLibraryLoader()
{
    if (m_loader && m_loader->IsLoaded())
    {
        m_loader->Unload();
    }
}

ScopedLibraryLoader::ScopedLibraryLoader(ScopedLibraryLoader&& other) noexcept
    : m_loader(std::move(other.m_loader))
    , m_libraryPath(std::move(other.m_libraryPath))
{
}

ScopedLibraryLoader& ScopedLibraryLoader::operator=(ScopedLibraryLoader&& other) noexcept
{
    if (this != &other)
    {
        if (m_loader && m_loader->IsLoaded())
        {
            m_loader->Unload();
        }
        m_loader = std::move(other.m_loader);
        m_libraryPath = std::move(other.m_libraryPath);
    }
    return *this;
}

bool ScopedLibraryLoader::IsLoaded() const
{
    return m_loader && m_loader->IsLoaded();
}

DynamicLoader* ScopedLibraryLoader::GetLoader()
{
    return m_loader.get();
}

const std::string& ScopedLibraryLoader::GetPath() const
{
    return m_libraryPath;
}

} // namespace SharedLibTest
