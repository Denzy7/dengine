/*! \file macros.h
 */


#ifndef MACROS_H
#define MACROS_H
#include "dengine_config.h"

/*! \def DENGINE_ARY_SZ(arr)
 *  \brief Returns the number of elements in \arr arr
 */
#define DENGINE_ARY_SZ(arr) sizeof(arr) / sizeof(arr[0])

/*! \def DENGINE_INLINE
 *  \brief Force compiler to inline a symbol
 */

#if defined(_MSC_VER)
#  define DENGINE_INLINE __forceinline
#else
#  define DENGINE_INLINE static inline __attribute((always_inline))
#endif

#if defined(DENGINE_WIN32)
#define DENGINE_ALIGN(t, n) __declspec(align(n)) t 
#else
#define DENGINE_ALIGN(t, n) t __attribute__ ((aligned (n)))
#endif

#define DENGINE_ISBITSET(var, bit) (var & (1<<bit))

/* this BS is only needed for MinGW where functions for no 
 * particular reason refuse to export.
 * This occurs in dengine-script when python is being used*/
#ifdef _WIN32
#define DENGINE_EXPORT __declspec(dllexport)
#else
#define DENGINE_EXPORT
#endif
#ifndef BUILD_SHARED_LIBS
#undef DENGINE_EXPORT 
#define DENGINE_EXPORT
#endif

#endif // MACROS_H
