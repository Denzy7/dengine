/*! \file macros.h
 */


#ifndef MACROS_H
#define MACROS_H

/*! \def DENGINE_ARY_SZ(arr)
 *  \brief Returns the number of elements in \arr arr
 */
#define DENGINE_ARY_SZ(arr) sizeof(arr) / sizeof(arr[0])

/*! \def DENGINE_INLINE
 */ \brief Force compiler to inline a symbol
#if defined(_MSC_VER)
#  define DENGINE_INLINE __forceinline
#else
#  define DENGINE_INLINE static inline __attribute((always_inline))
#endif

#endif // MACROS_H
