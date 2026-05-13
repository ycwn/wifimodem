
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

#include <byteswap.h>

#include "core/algorithm.h"
#include "core/types.h"
#include "core/util.h"

#include "util/buffer.h"



int buffer_read(buffer *buf, u8 *ptr, uint len)
{

	if (buffer_empty(buf))
		return 0;

	if (len > buffer_size(buf))
		len = buffer_size(buf);

	memcpy(ptr, buffer_rdata(buf), len);
	buf->pos += len;

	return len;

}



int buffer_write(buffer *buf, const u8 *ptr, uint len)
{

	if (len >= buffer_free(buf))
		len = buffer_free(buf);

	memcpy(buffer_wdata(buf), ptr, len);
	buf->len += len;

	return len;

}



int buffer_pack(buffer *buf, const char *fmt, ...)
{

	va_list argv;
	va_start(argv);

	int r = buffer_packv(buf, fmt, argv);

	va_end(argv);
	return r;

}



int buffer_unpack(buffer *buf, const char *fmt, ...)
{

	va_list argv;
	va_start(argv);

	int r = buffer_unpackv(buf, fmt, argv);

	va_end(argv);
	return r;

}



int buffer_packv(buffer *buf, const char *fmt, va_list argv)
{

	union {

		u8 ub; u16 uw; u32 ud; u64 uq;
		i8 ib; i16 iw; i32 id; i64 iq;

		u8 bytes[0];

	} data;


	int  c, n;
	bool l = true;

	for (n=0; (c = *fmt++) != 0; n++) {

		uint        len = 0;
		const void *ptr = data.bytes;

		switch (c) {

			case '<': l = true;  continue; // Switch to little endian encoding
			case '>': l = false; continue; // Switch to big endian encoding

			case 'B': data.ub = va_get(argv, u8);  len = sizeof(u8);  break;
			case 'W': data.uw = va_get(argv, u16); len = sizeof(u16); if (!l) data.uw = bswap_16(data.uw); break;
			case 'D': data.ud = va_get(argv, u32); len = sizeof(u32); if (!l) data.ud = __builtin_bswap32(data.ud); break;
			case 'Q': data.uq = va_get(argv, u64); len = sizeof(u64); if (!l) data.uq = bswap_64(data.uq); break;

			case 'b': data.ib = va_get(argv, i8);  len = sizeof(i8);  break;
			case 'w': data.iw = va_get(argv, i16); len = sizeof(i16); if (!l) data.iw = bswap_16(data.iw); break;
			case 'd': data.id = va_get(argv, i32); len = sizeof(i32); if (!l) data.id = bswap_32(data.id); break;
			case 'q': data.iq = va_get(argv, i64); len = sizeof(i64); if (!l) data.iq = bswap_64(data.iq); break;

			case 's':
			case 'S':
				ptr = va_get(argv, char*);
				len = strlen((const char*)ptr);
				break;

			default:
				continue;

		}

		if (buffer_write(buf, ptr, len) < len)
			return n;

	}

	return n;

}



int buffer_unpackv(buffer *buf, const char *fmt, va_list argv)
{

	union {

		u8 ub; u16 uw; u32 ud; u64 uq;
		i8 ib; i16 iw; i32 id; i64 iq;

		u8 bytes[0];

	} data;


	int  c, n;
	bool l = true;

	for (n=0; (c = *fmt++) != 0; n++) {

		uint  len = 0;
		void *ptr = data.bytes;

		switch (c) {

			case '<': l = true;  continue;
			case '>': l = false; continue;

			case 'B': len = sizeof(u8);  break;
			case 'W': len = sizeof(u16); break;
			case 'D': len = sizeof(u32); break;
			case 'L': len = sizeof(u64); break;

			case 'b': len = sizeof(i8);  break;
			case 'w': len = sizeof(i16); break;
			case 'd': len = sizeof(i32); break;
			case 'l': len = sizeof(i64); break;

			case 's':
			case 'S':
				ptr = va_get(argv, u8*);
				len = va_get(argv, uint);
				break;

			default:
				continue;

		}


		if (buffer_read(buf, ptr, len) < len)
			return n;


		if (l) {

			switch (c) {

				case 'B': *va_get(argv, u8*)  = data.ub; break;
				case 'W': *va_get(argv, u16*) = data.uw; break;
				case 'D': *va_get(argv, u32*) = data.ud; break;
				case 'L': *va_get(argv, u64*) = data.uq; break;

				case 'b': *va_get(argv, i8*)  = data.ib; break;
				case 'w': *va_get(argv, i16*) = data.iw; break;
				case 'd': *va_get(argv, i32*) = data.id; break;
				case 'l': *va_get(argv, i64*) = data.iq; break;

				default:
					continue;

			}

		} else {

			switch (c) {

				case 'B': *va_arg(argv, u8*)  = data.ub;           break;
				case 'W': *va_arg(argv, u16*) = bswap_16(data.uw); break;
				case 'D': *va_arg(argv, u32*) = bswap_32(data.ud); break;
				case 'L': *va_arg(argv, u64*) = bswap_64(data.uq); break;

				case 'b': *va_arg(argv, i8*)  = data.ib;           break;
				case 'w': *va_arg(argv, i16*) = bswap_16(data.iw); break;
				case 'd': *va_arg(argv, i32*) = bswap_32(data.id); break;
				case 'l': *va_arg(argv, i64*) = bswap_64(data.iq); break;

				default:
					continue;

			}

		}

	}

	return n;

}



void buffer_zero(buffer *buf)
{

	buffer_clear(buf);
	memset(buf->ptr, 0, buf->cap);

}



uint buffer_retract(buffer *buf, uint count)
{

	if (count > buf->len)
		count = buf->len;

	memmove(buf->ptr, buf->ptr + count, buf->cap - count);
	buf->len -= count;

	if (count < buf->pos) buf->pos -= count;
	else                  buf->pos = 0;

	return count;

}



void *buffer_append(buffer *buf, const void *ptr, uint len)
{

	if (buffer_free(buf) < len)
		return NULL;

	if (buf != NULL && len == 0)
		len = strlen((const char*)buf) + 1;

	auto p = buffer_wdata(buf);

	if (buf != NULL) buffer_write(buf, (const u8*)ptr, len);
	else             buffer_reserve(buf, len);

	return p;

}


