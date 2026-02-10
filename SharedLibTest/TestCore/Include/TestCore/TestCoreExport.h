#pragma once

// 跨平台 DLL 导出/导入宏
#ifdef TESTCORE_STATIC_DEFINE
#  define TESTCORE_EXPORT
#  define TESTCORE_NO_EXPORT
#else
#  ifndef TESTCORE_EXPORT
#    ifdef TestCore_EXPORTS
        /* We are building this library */
#      define TESTCORE_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define TESTCORE_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef TESTCORE_NO_EXPORT
#    define TESTCORE_NO_EXPORT
#  endif
#endif

// 静态库/共享库兼容
#ifndef TESTCORE_DEPRECATED
#  define TESTCORE_DEPRECATED __declspec(deprecated)
#endif

#ifndef TESTCORE_DEPRECATED_EXPORT
#  define TESTCORE_DEPRECATED_EXPORT TESTCORE_EXPORT TESTCORE_DEPRECATED
#endif

#ifndef TESTCORE_DEPRECATED_NO_EXPORT
#  define TESTCORE_DEPRECATED_NO_EXPORT TESTCORE_NO_EXPORT TESTCORE_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef TESTCORE_NO_DEPRECATED
#    define TESTCORE_NO_DEPRECATED
#  endif
#endif

// 编译器通用支持
#ifndef TESTCORE_EXPORT
#  if defined(__GNUC__) || defined(__clang__)
#    define TESTCORE_EXPORT __attribute__((visibility("default")))
#    define TESTCORE_NO_EXPORT __attribute__((visibility("hidden")))
#  else
#    define TESTCORE_EXPORT
#    define TESTCORE_NO_EXPORT
#  endif
#endif
