/*
 * common.h: includes common typedefs used across sclc
 */

#ifndef SCLC_COMMON_H
#define SCLC_COMMON_H

#include <stdint.h>

/*
 * Unsigned integers
 */
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

/*
 * Signed integers
 */
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

/*
 * Floating point types
 */
_Static_assert(sizeof(float) == 4, "float must be 32-bit");
_Static_assert(sizeof(double) == 8, "double must be 64-bit");

typedef float f32;
typedef double f64;

#endif // !SCLC_COMMON_H
