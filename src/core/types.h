

#ifndef CORE_TYPES_H
#define CORE_TYPES_H


typedef unsigned int uint;

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;


#ifndef bswap_16
#define bswap_16  __builtin_bswap16
#endif

#ifndef bswap_32
#define bswap_32  __builtin_bswap32
#endif

#ifndef bswap_64
#define bswap_64  __builtin_bswap64
#endif


#endif


