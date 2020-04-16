#pragma once

#include <stdint.h>

typedef ::int8_t int8;
typedef ::int16_t int16;
typedef ::int32_t int32;
typedef ::int64_t int64;

typedef ::uint8_t uint8;
typedef ::uint16_t uint16;
typedef ::uint32_t uint32;
typedef ::uint64_t uint64;

#define DISALLOW_COPY_AND_ASSIGN(Type) \
    Type(const Type&); \
    void operator=(const Type&)

template<typename To, typename From>
inline To implicit_cast(From f) {
    return (To) f;
}
