

#ifndef CORE_STR_H
#define CORE_STR_H


typedef struct {

	const char *ptr;
	uint        len;

} str;


typedef int (str_pred)(int ch);



static inline str str_zero()                          { return (str){ "",  0   }; }
static inline str str_setp(const char *ptr, uint len) { return (str){ ptr, len }; }
static inline str str_sets(const char *sz)            { return str_setp(sz, strlen(sz)); }

str str_left( const str *s, uint len);             // Return leftmost-len characters of string
str str_right(const str *s, uint len);             // Return rightmost-len characters of string
str str_mid(  const str *s, uint pos, uint len);   // Return len characters of string, starting at pos

void str_skip(  str *s, uint num);         // Remove first num characters
void str_chop(  str *s, uint num);         // Remove last num characters
void str_ltrim( str *s);                   // Remove leftmost whitespace
void str_rtrim( str *s);                   // Remove rightmost whitespace
void str_pltrim(str *s, str_pred *pred);   // Remove leftmost characters that match predicate
void str_prtrim(str *s, str_pred *pred);   // Remove rightmost characters that match predicate

static inline void str_trim( str *s)                 { str_ltrim(s);        str_rtrim(s); }
static inline void str_ptrim(str *s, str_pred *pred) { str_pltrim(s, pred); str_prtrim(s, pred); }

str str_split( str *s, uint pos);          // Split string at pos, remove first part and return it
str str_span(  str *s, const char *p);     // Remove matching characters and return them
str str_cspan( str *s, const char *p);     // Remove non-matching characters and return them
str str_pspan( str *s, str_pred *pred);    // Remove and return characters matching predicate
str str_pcspan(str *s, str_pred *pred);    // Remove and return characters failing predicate

int str_cmp( const str *a, const str *b);      // Compare strings, like strcmp 
int str_cmpi(const str *a, const str *b);      // Compare strings, like strcasecmp

int str_find(  const str *s, const str *p);    // Search for p in s, return offset or -1 if not found
int str_findi( const str *s, const str *p);    // Search for p in s, case-insensitive
int str_rfind( const str *s, const str *p);    // Reverse search
int str_rfindi(const str *s, const str *p);    // Reverse search, case-insensitive

bool str_match(  const str *s, const str *p);     // Check if s starts with p, return true if so
bool str_matchi( const str *s, const str *p);     // Check if s starts with p, case-insensitive
int  str_matchv( const str *s, const str p[]);    // Check if s starts with any from p, return index or -1 on error
int  str_matchiv(const str *s, const str p[]);    // Check if s starts with any from p, case-insensitive

bool str_accept(  str *s, const str *p);     // Check if s starts with p, remove it and return true
bool str_accepti( str *s, const str *p);     // Check if s starts with p, case-insensitive
int  str_acceptv( str *s, const str p[]);    // Check if s starts with any string in p, remove it, and return index
int  str_acceptiv(str *s, const str p[]);    // Check is s starts with any string in p, case-insensitive

bool str_readu8(  str *s,  u8    *v, uint r);    // Read an unsigned k-bit integer, return true on success
bool str_readu16( str *s, u16    *v, uint r);
bool str_readu32( str *s, u32    *v, uint r);
bool str_readu64( str *s, u64    *v, uint r);
bool str_readi8(  str *s,  i8    *v, uint r);    // Read a signed k-bit integer, return true on success
bool str_readi16( str *s, i16    *v, uint r);
bool str_readi32( str *s, i32    *v, uint r);
bool str_readi64( str *s, i64    *v, uint r);
bool str_readf32( str *s, float  *v);
bool str_readf64( str *s, double *v);
void str_readstr( str *s, str    *v, char *buf, uint len, const char *delm); // Read a string until delim using buf as store, and decode escape characters
bool str_readqstr(str *s, str    *v, char *buf, uint len);                   // Read a single, or double-quoted string

char *str_zcpy(const str *s, char *buf, uint len);



#define str_readint(s, v)                         \
	_Generic((v),                             \
		i8*:  str_readi8,                 \
		i16*: str_readi16,                \
		i32*: str_readi32,                \
		i64*: str_readi64,                \
		int*: str_readi32)(s, (void*)(v), 0)

#define str_readuint(s, v)                         \
	_Generic((v),                              \
		u8*:   str_readu8,                 \
		u16*:  str_readu16,                \
		u32*:  str_readu32,                \
		u64*:  str_readu64,                \
		uint*: str_readu32)(s, (void*)(v), 0)

#define str_readfloat(s, v)                   \
	_Generic((v),                         \
		float*:  str_readf32,         \
		double*: str_readf64)(s, (v), 0)



#define CSTR(s)      (str){ .ptr=(s),     .len=sizeof(s)-1 }
#define CSTRN(s, n)  (str){ .ptr=(s),     .len=(n) }
#define CSTRZ(s)     (str){ .ptr=(s),     .len=strlen(s) }
#define STR(s)       (str){ .ptr=(s).ptr, .len=(s).len }

#define STRF      "%.*s"
#define STRFW(n)  "%" #n ".*s"
#define STRV(s)   (s).len,  (s).ptr
#define STRVP(s)  (s)->len, (s)->ptr

#define TAB   "\t"
#define CRLF  "\n"


#endif


