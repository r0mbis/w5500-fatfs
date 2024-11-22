#pragma once

#if defined(__clang__)

#define NO_OPT __attribute__((optnone))

#define MALLOC_LIKE_FUNC __attribute__((ownership_returns(malloc)))

//! Note: Second Argument is the argument index for
//! the pointer being free'd. For simplicity, let's assume
//! the pointer is always in the 1st argument
#define FREE_LIKE_FUNC __attribute__((ownership_takes(malloc, 1)))

//! The Thread Safety Analyzers need an identifier to track things by
//! When using C++, this capability attribute can be bound to a Class.
//! For C, we'll just create a dummy variable that is not referenced
//! by actual code so it gets optimized away during compilation.
typedef int __attribute__((capability("mutex"))) _ClangThreadSafetyLockReference;
#define INTERNAL_DECLARE_LOCK_TRACKER(name) extern _ClangThreadSafetyLockReference _##name

//! Flags a function that acquires a resource. In our example
//! we'll want to apply this attribute to flash_lock() and
//! accel_lock().
#define FUNC_ACQUIRES_LOCK(name)         \
    INTERNAL_DECLARE_LOCK_TRACKER(name); \
    __attribute__((acquire_capability(_##name)))

//! Flags a function that releases a resource. For our example,
//! the accel_unlock() and flash_unlock() functions need this.
#define FUNC_RELEASES_LOCK(name) __attribute__((release_capability(_##name)))

//! Flags a function as requiring a lock be held by the time
//! it is invoked. For example, an "accel_read()" function.
#define FUNC_REQUIRES_LOCK_HELD(name) __attribute__((requires_capability(_##name)))

//! Disables thread safety checks for a function
//! This is required for the *_lock and *_unlock functions
//! in our example to prevent False positives.
#define FUNC_DISABLE_LOCK_CHECKS __attribute__((no_thread_safety_analysis))

#elif defined(__GNUC__)

#define NO_OPT __attribute__((optimize("O0")))
#define MALLOC_LIKE_FUNC
#define FREE_LIKE_FUNC
#define INTERNAL_DECLARE_LOCK_TRACKER(name)
#define FUNC_ACQUIRES_LOCK(name)
#define INTERNAL_DECLARE_LOCK_TRACKER(name)
#define FUNC_RELEASES_LOCK(name)
#define FUNC_REQUIRES_LOCK_HELD(name)
#define FUNC_DISABLE_LOCK_CHECKS

#else
#error "Unsupported Compiler"
#endif

#ifndef COMPILE_DATE
#define COMPILE_DATE __DATE__
#endif
#ifndef COMPILE_TIME
#define COMPILE_TIME __TIME__
#endif

#define COMPILE_YEAR_INT                                                  \
    ((((COMPILE_DATE[7u] - '0') * 10u + (COMPILE_DATE[8u] - '0')) * 10u + \
      (COMPILE_DATE[9u] - '0')) *                                         \
         10u +                                                            \
     (COMPILE_DATE[10u] - '0'))

#define COMPILE_MONTH_INT                                              \
    ((COMPILE_DATE[2u] == 'n' && COMPILE_DATE[1u] == 'a') ? 1u /*Jan*/ \
     :                                                                 \
     (COMPILE_DATE[2u] == 'b') ? 2u /*Feb*/                            \
     :                                                                 \
     (COMPILE_DATE[2u] == 'r' && COMPILE_DATE[1u] == 'a') ? 3u /*Mar*/ \
     :                                                                 \
     (COMPILE_DATE[2u] == 'r') ? 4u /*Apr*/                            \
     :                                                                 \
     (COMPILE_DATE[2u] == 'y') ? 5u /*May*/                            \
     :                                                                 \
     (COMPILE_DATE[2u] == 'n') ? 6u /*Jun*/                            \
     :                                                                 \
     (COMPILE_DATE[2u] == 'l') ? 7u /*Jul*/                            \
     :                                                                 \
     (COMPILE_DATE[2u] == 'g') ? 8u /*Aug*/                            \
     :                                                                 \
     (COMPILE_DATE[2u] == 'p') ? 9u /*Sep*/                            \
     :                                                                 \
     (COMPILE_DATE[2u] == 't') ? 10u /*Oct*/                           \
     :                                                                 \
     (COMPILE_DATE[2u] == 'v') ? 11u /*Nov*/                           \
                                 :                                     \
                                 12u /*Dec*/)

#define COMPILE_DAY_INT \
    ((COMPILE_DATE[4u] == ' ' ? 0 : COMPILE_DATE[4u] - '0') * 10u + (COMPILE_DATE[5u] - '0'))

// __TIME__ expands to an eight-character string constant
// "23:59:01", or (if cannot determine time) "??:??:??"
#define COMPILE_HOUR_INT                                            \
    ((COMPILE_TIME[0u] == '?' ? 0 : COMPILE_TIME[0u] - '0') * 10u + \
     (COMPILE_TIME[1u] == '?' ? 0 : COMPILE_TIME[1u] - '0'))

#define COMPILE_MINUTE_INT                                          \
    ((COMPILE_TIME[3u] == '?' ? 0 : COMPILE_TIME[3u] - '0') * 10u + \
     (COMPILE_TIME[4u] == '?' ? 0 : COMPILE_TIME[4u] - '0'))

#define COMPILE_SECONDS_INT                                         \
    ((COMPILE_TIME[6u] == '?' ? 0 : COMPILE_TIME[6u] - '0') * 10u + \
     (COMPILE_TIME[7u] == '?' ? 0 : COMPILE_TIME[7u] - '0'))

#define COMPILE_DOS_DATE \
    (((COMPILE_YEAR_INT - 1980u) << 9u) | (COMPILE_MONTH_INT << 5u) | (COMPILE_DAY_INT << 0u))

#define COMPILE_DOS_TIME \
    ((COMPILE_HOUR_INT << 11u) | (COMPILE_MINUTE_INT << 5u) | (COMPILE_SECONDS_INT << 0u))

// version
#define VERSION_SHORT GIT_VERSION
#if GIT_DIRTY
#define GIT_COMMIT_INFO GIT_COMMIT " " GIT_BRANCH " v. " GIT_VERSION " dirty"
#else
#define GIT_COMMIT_INFO GIT_COMMIT " " GIT_BRANCH " v. " GIT_VERSION " release"
#endif

#if GIT_DIRTY
#define VESRION_STRING GIT_VERSION " " GIT_COMMIT " dirty"
#define VESRION_STRING_FULL GIT_VERSION " " GIT_BRANCH " " GIT_COMMIT " dirty"
#else
#define VESRION_STRING GIT_VERSION " " GIT_COMMIT " release"
#define VESRION_STRING_FULL GIT_VERSION " " GIT_BRANCH " " GIT_COMMIT " release"
#endif
