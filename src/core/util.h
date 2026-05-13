

#ifndef CORE_UTIL_H
#define CORE_UTIL_H



static inline int  mini(int  a, int  b) { return a < b? a: b; }
static inline uint minu(uint a, uint b) { return a < b? a: b; }

static inline int  maxi(int  a, int  b) { return a > b? a: b; }
static inline uint maxu(uint a, uint b) { return a > b? a: b; }

static inline int  clampi(int  a, int  b, int  x) { return mini(maxi(x, a), b); }
static inline uint clampu(uint a, uint b, uint x) { return minu(maxu(x, a), b); }


#define va_get(va, x) (PP_ISPROMOTED(x)? (x)va_arg(va, int): (x)va_arg(va, x))


#define byte(x, n)   (((x) >> ( 8*(n))) & UINT8_MAX)
#define word(x, n)   (((x) >> (16*(n))) & UINT16_MAX)
#define dword(x, n)  (((x) >> (32*(n))) & UINT32_MAX)


static inline u16 mkword2(uint hi, uint lo) {
	return ((hi & UINT8_MAX) << 8) | ((lo & UINT8_MAX) << 0);
}

static inline u32 mkdword2(uint hi, uint lo) {
	return ((hi & UINT16_MAX) << 16) | ((lo & UINT16_MAX) << 0);
}

static inline u32 mkdword4(uint hi, uint mh, uint ml, uint lo) {
	return ((hi & UINT8_MAX) << 24) | ((mh & UINT8_MAX) << 16) | ((ml & UINT8_MAX) << 8) | ((lo & UINT8_MAX) << 0);
}


#endif


