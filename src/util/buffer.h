

#ifndef UTIL_BUFFER_H
#define UTIL_BUFFER_H


typedef struct {

	u8   *ptr;
	uint  cap;

	uint pos;
	uint len;

} buffer;


int buffer_read( buffer *buf, u8       *ptr, uint len);
int buffer_write(buffer *buf, const u8 *ptr, uint len);

int buffer_pack(  buffer *buf, const char *fmt, ...);
int buffer_unpack(buffer *buf, const char *fmt, ...);

int buffer_packv(  buffer *buf, const char *fmt, va_list argv);
int buffer_unpackv(buffer *buf, const char *fmt, va_list argv);

void  buffer_zero(   buffer *buf);
uint  buffer_retract(buffer *buf, uint count);
void *buffer_append( buffer *buf, const void *ptr, uint len);


static inline uint  buffer_rtell(   const buffer *buf) { return buf->pos; }
static inline uint  buffer_wtell(   const buffer *buf) { return buf->len; }
static inline uint  buffer_size(    const buffer *buf) { return buf->len - buf->pos; }
static inline uint  buffer_free(    const buffer *buf) { return buf->cap - buf->len; }
static inline uint  buffer_capacity(const buffer *buf) { return buf->cap; }
static inline bool  buffer_empty(   const buffer *buf) { return buffer_size(buf) == 0; }
static inline bool  buffer_full(    const buffer *buf) { return buffer_free(buf) == 0; }
static inline void *buffer_rdata(   const buffer *buf) { return buf->ptr + buf->pos; }
static inline void *buffer_wdata(   const buffer *buf) { return buf->ptr + buf->len; }
static inline char *buffer_rstr(    const buffer *buf) { return (char*)buf->ptr + buf->pos; }
static inline char *buffer_wstr(    const buffer *buf) { return (char*)buf->ptr + buf->len; }


static inline void buffer_init(buffer *buf, u8 *ptr, uint cap) {
	buf->ptr = ptr; buf->cap = cap;
	buf->pos = 0;   buf->len = 0;
}

static inline void buffer_clear(  buffer *buf)           { buf->pos = 0; buf->len = 0; }
static inline void buffer_rewind( buffer *buf)           { buf->pos = 0; }
static inline void buffer_unread( buffer *buf, uint len) { buf->pos -= minu(len, buffer_rtell(buf)); }
static inline uint buffer_reserve(buffer *buf, uint len) { buf->len += minu(len, buffer_free(buf)); return buffer_free(buf); }
static inline void buffer_prepare(buffer *buf, uint len) { int num = len - buffer_free(buf); if (num > 0) buffer_retract(buf, num); }

static inline int buffer_getc( buffer *buf)                { u8 c; return (buffer_read(buf, &c, sizeof(c)) > 0)? c: -1; }
static inline int buffer_putc( buffer *buf, u8 c)          { return buffer_write(buf, &c, sizeof(c)); }
static inline int buffer_puts( buffer *buf, const char *s) { return buffer_write(buf, (const u8*)s, strlen(s)); }
static inline int buffer_putsz(buffer *buf, const char *s) { return buffer_write(buf, (const u8*)s, strlen(s) + 1); }


#endif


