#pragma once

#ifdef TEST1LIB_STATIC_DEFINE
#  define TEST1LIB_EXPORT
#  define TEST1LIB_NO_EXPORT
#else
#  ifndef TEST1LIB_EXPORT
#    ifdef Test1lib_EXPORTS
#      define TEST1LIB_EXPORT __declspec(dllexport)
#    else
#      define TEST1LIB_EXPORT __declspec(dllimport)
#    endif
#  endif
#  ifndef TEST1LIB_NO_EXPORT
#    define TEST1LIB_NO_EXPORT
#  endif
#endif

#ifndef TEST1LIB_EXPORT
#  if defined(__GNUC__) || defined(__clang__)
#    define TEST1LIB_EXPORT __attribute__((visibility("default")))
#    define TEST1LIB_NO_EXPORT __attribute__((visibility("hidden")))
#  else
#    define TEST1LIB_EXPORT
#    define TEST1LIB_NO_EXPORT
#  endif
#endif

#ifndef TEST1LIB_DEPRECATED
#  define TEST1LIB_DEPRECATED __declspec(deprecated)
#endif
