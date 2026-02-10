#pragma once

#ifdef TESTLIB3_STATIC_DEFINE
#  define TESTLIB3_EXPORT
#  define TESTLIB3_NO_EXPORT
#else
#  ifndef TESTLIB3_EXPORT
#    ifdef Testlib3_EXPORTS
#      define TESTLIB3_EXPORT __declspec(dllexport)
#    else
#      define TESTLIB3_EXPORT __declspec(dllimport)
#    endif
#  endif
#  ifndef TESTLIB3_NO_EXPORT
#    define TESTLIB3_NO_EXPORT
#  endif
#endif

#ifndef TESTLIB3_EXPORT
#  if defined(__GNUC__) || defined(__clang__)
#    define TESTLIB3_EXPORT __attribute__((visibility("default")))
#    define TESTLIB3_NO_EXPORT __attribute__((visibility("hidden")))
#  else
#    define TESTLIB3_EXPORT
#    define TESTLIB3_NO_EXPORT
#  endif
#endif
