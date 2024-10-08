#pragma once

// constexpr version of to_us_since_boot()
constexpr uint64_t to_us_since_boot_constexpr(absolute_time_t t)
{
#ifdef NDEBUG
    return t;
#else
    return t._private_us_since_boot;
#endif
}

// constexpr version of from_us_since_boot()
constexpr absolute_time_t from_us_since_boot_constexpr(uint64_t t)
{
    absolute_time_t tRet = 0;
#ifdef NDEBUG
    tRet = t;
#else
    //assert(us_since_boot <= INT64_MAX);
    tRet._private_us_since_boot = t;
#endif
    return tRet;
}

// Compare two times
// Just a simple 64-bit comparison, but works in Debug build where
// absolute_time_t is defined differently.
constexpr bool timeIsReached(absolute_time_t t, absolute_time_t tTarget)
{
    return to_us_since_boot_constexpr(t) >= to_us_since_boot_constexpr(tTarget);
}
