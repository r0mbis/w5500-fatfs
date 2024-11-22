#pragma once

#include <stdint.h>
#include "arm_math.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef M_PI_2
#define M_PI_2 (1.570796326794897f)
#endif

#if defined(_MSC_VER)
#define FORCE_INLINE __forceinline /**< & */
#elif defined(__clang__)
#define FORCE_INLINE inline __attribute__((always_inline)) /**< & */
#define FORCE_NO_INLINE  __attribute__((noinline)) /**< & */
// #pragma clang diagnostic ignored "-Wduplicate-decl-specifier"
#elif defined(__GNUC__)
#define FORCE_INLINE inline __attribute__((always_inline)) /**< & */
#define FORCE_NO_INLINE   __attribute__((noinline)) /**< & */
#else
#error unknown compiler
#endif

#if 0
#ifndef assert_param
#ifdef USE_FULL_ASSERT
/**
  * @brief  The assert_param macro is used for function's parameters check.
  * @param  expr: If expr is false, it calls assert_failed function
  *         which reports the name of the source file and the source
  *         line number of the call that failed.
  *         If expr is true, it returns no value.
  * @retval None
  */
#define assert_param(expr) ((expr) ? (void)0U : assert_failed((uint8_t*)__FILE__, __LINE__))
/* Exported functions ------------------------------------------------------- */
void assert_failed(uint8_t* file, uint32_t line);
#else
#define assert_param(expr) ((void)0U)
#endif /* USE_FULL_ASSERT */
#endif /* assert */
#endif 

#define TWO_PI   (6.28318530717958f)
#define HALF_PI  (1.57079632679489f)
#define HALF3_PI (4.71238898038469f)

/* approximation taken from http://www.ganssle.com/approx.htm */ /* formatting looks so cool, i'll just leave it here */
#define cosapprox(xx)                                               \
    (0.99999999999999806767 +                                       \
     xx * (-0.4999999999998996568 +                                 \
           xx * (0.04166666666581174292 +                           \
                 xx * (-0.001388888886113613522 +                   \
                       xx * (0.000024801582876042427 +              \
                             xx * (-0.0000002755693576863181 +      \
                                   xx * (0.0000000020858327958707 + \
                                         xx * -0.000000000011080716368)))))))

#define msqr(x) ((x) * (x))
#define mabsf(x) ((x) < 0.0f ? -(x) : (x))
#define quad1(x) (cosapprox(msqr(x)))
#define quad2(x) (-cosapprox(msqr(PI - x)))
#define quad3(x) (-cosapprox(msqr(x - PI)))
#define quad4(x) (cosapprox(msqr(TWO_PI - x)))
#define mcos(x) \
    ((x) < HALF_PI ? quad1(x) : (x) < PI ? quad2(x) : (x) < HALF3_PI ? quad3(x) : quad4(x))

#define TRIG_TABLE_SIZE 0x0100
#define TRIG_TABLE_MASK (TRIG_TABLE_SIZE - 1)
#define RADIANS_TO_TABLE_INDEX (TRIG_TABLE_SIZE / TWO_PI)

#ifndef LUTCOS
#define LUTCOS

void util_prepare_lut();

float lutsin(float rad);
float lutcos(float rad);

void util_calc_radiant( float* xy, float value, uint8_t scale, uint8_t size);

#endif

#ifndef MAX

#define MAX(max_a, max_b)                   \
    ({                                      \
        __typeof__(max_a) _max_a = (max_a); \
        __typeof__(max_b) _max_b = (max_b); \
        _max_a > _max_b ? _max_a : _max_b;  \
    })
#endif

#ifndef MIN
#define MIN(min_a, min_b)                   \
    ({                                      \
        __typeof__(min_a) _min_a = (min_a); \
        __typeof__(min_b) _min_b = (min_b); \
        _min_a < _min_b ? _min_a : _min_b;  \
    })
#endif

#ifndef ROUND_UP_TO
#define ROUND_UP_TO(round_in, round_to)                    \
    ({                                                     \
        __typeof__(round_in) _round_in = (round_in);       \
        __typeof__(round_to) _round_to = (round_to);       \
        _round_in / _round_to + !!(_round_in % _round_to); \
    })
#endif

#ifndef MAP_VAL_T
#define MAP_VAL_T(in_map, imin, imax, omin, omax)                      \
    ({                                                                 \
        __typeof__(in_map) _in_map = (in_map);                         \
        __typeof__(imin) _imin = (imin);                               \
        __typeof__(imax) _imax = (imax);                               \
        __typeof__(omin) _omin = (omin);                               \
        __typeof__(omax) _omax = (omax);                               \
        (_in_map - _imin) * (_omax - _omin) / (_imax - _imin) + _omin; \
    })
#endif

#ifndef XORSWAP_UNSAFE
#define XORSWAP_UNSAFE(a, b) \
    ((a) ^= (b),             \
     (b) ^= (a),             \
     (a) ^= (b)) /* Doesn't work when a and b are the same object - assigns zero \
                  (0) to the object in that case */
#endif

#ifndef XORSWAP
#define XORSWAP(a, b)                                        \
    ((&(a) == &(b)) ? (a) /* Check for distinct addresses */ \
                      :                                      \
                      XORSWAP_UNSAFE(a, b))
#endif

#ifndef MAP_VAL
#define MAP_VAL(x, imin, imax, omin, omax) ((x - imin) * (omax - omin) / (imax - imin) + omin)
#endif

#ifndef CLAMP
#define CLAMP(x, upper, lower) (MIN(upper, MAX(x, lower)))
#endif

#ifndef ARR_SIZE
#define ARR_SIZE(a) sizeof(a) / sizeof(a[0])
#endif

#define MAX_STR_NUM 32

#define NAME_EMPTY \
    { 0 }

#define EMPTY_STR "empty"

#define BIT_SET(a, b) ((a) |= (1ULL << (b)))
#define BIT_CLEAR(a, b) ((a) &= ~(1ULL << (b)))
#define BIT_FLIP(a, b) ((a) ^= (1ULL << (b)))
#define BIT_CHECK(a, b) (!!((a) & (1ULL << (b)))) // '!!' to make sure this returns 0 or 1

#define BITMASK_SET(x, mask) ((x) |= (mask))
#define BITMASK_CLEAR(x, mask) ((x) &= (~(mask)))
#define BITMASK_FLIP(x, mask) ((x) ^= (mask))
#define BITMASK_CHECK_ALL(x, mask) (!(~(x) & (mask)))
#define BITMASK_CHECK_ANY(x, mask) ((x) & (mask))

/* --- PRINTF_BYTE_TO_BINARY macro's --- */
#define PRINTF_BINARY_PATTERN_INT8 "%c%c%c%c%c%c%c%c"
#define PRINTF_BYTE_TO_BINARY_INT8(i)                                                       \
    (((i)&0x80ll) ? '1' : '0'), (((i)&0x40ll) ? '1' : '0'), (((i)&0x20ll) ? '1' : '0'),     \
        (((i)&0x10ll) ? '1' : '0'), (((i)&0x08ll) ? '1' : '0'), (((i)&0x04ll) ? '1' : '0'), \
        (((i)&0x02ll) ? '1' : '0'), (((i)&0x01ll) ? '1' : '0')

#define PRINTF_BINARY_PATTERN_INT16 PRINTF_BINARY_PATTERN_INT8 PRINTF_BINARY_PATTERN_INT8
#define PRINTF_BYTE_TO_BINARY_INT16(i) \
    PRINTF_BYTE_TO_BINARY_INT8((i) >> 8), PRINTF_BYTE_TO_BINARY_INT8(i)
#define PRINTF_BINARY_PATTERN_INT32 PRINTF_BINARY_PATTERN_INT16 PRINTF_BINARY_PATTERN_INT16
#define PRINTF_BYTE_TO_BINARY_INT32(i) \
    PRINTF_BYTE_TO_BINARY_INT16((i) >> 16), PRINTF_BYTE_TO_BINARY_INT16(i)
#define PRINTF_BINARY_PATTERN_INT64 PRINTF_BINARY_PATTERN_INT32 PRINTF_BINARY_PATTERN_INT32
#define PRINTF_BYTE_TO_BINARY_INT64(i) \
    PRINTF_BYTE_TO_BINARY_INT32((i) >> 32), PRINTF_BYTE_TO_BINARY_INT32(i)

/* --- end macros --- */

uint8_t util_rev_byte(uint8_t n);

float lerp(float k0, float k1, float t);
float lerp_precise(float k0, float k1, float t);
float map(const float x, const float imin, const float imax, const float omin, const float omax);

int sha1digest(uint8_t *digest, char *hexdigest, const uint8_t *data, size_t databytes);

float lim_f(float in, float min, float max); // saturate
float lim_f_0_1(float in); // saturate (0,1)
float lim_f_n1_1(float in); // saturate (-1,1)


#ifdef __cplusplus
}
#endif
