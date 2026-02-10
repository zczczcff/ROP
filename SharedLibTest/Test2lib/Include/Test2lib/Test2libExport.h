#pragma once

#ifdef TEST2LIB_STATIC_DEFINE
#  define TEST2LIB_EXPORT
#  define TEST2LIB_NO_EXPORT
#else
#  ifndef TEST2LIB_EXPORT
#    ifdef Test2lib_EXPORTS
#      define TEST2LIB_EXPORT __declspec(dllexport)
#    else
#      define TEST2LIB_EXPORT __declspec(dllimport)
#    endif
#  endif
#  ifndef TEST2LIB_NO_EXPORT
#    define TEST2LIB_NO_EXPORT
#  endif
#endif

#ifndef TEST2LIB_EXPORT
#  if defined(__GNUC__) || defined(__clang__)
#    define TEST2LIB_EXPORT __attribute__((visibility("default")))
#    define TEST2LIB_NO_EXPORT __attribute__((visibility("hidden")))
#  else
#    define TEST2LIB_EXPORT
#    define TEST2LIB_NO_EXPORT
#  endif
#endif

#ifndef TEST2LIB_DEPRECATED
#  define TEST2LIB_DEPRECATED __declspec(deprecated)
#endif
