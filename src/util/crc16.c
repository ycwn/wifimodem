
#include <stddef.h>
#include <stdint.h>

#include "core/types.h"
#include "core/debug.h"

#include "util/crc16.h"



u16 crc16_next(u16 crc, u8 ch)
{

	crc = crc ^ (uint)ch << 8;

	for (int n=0; n < 8; n++)
		crc = (crc & 0x8000)? crc << 1 ^ 0x1021: crc << 1;

	return crc & 0xffffu;

}



u16 crc16_buf(u16 crc, const u8 *buf, uint len)
{

	for (int n=0; n < len; n++)
		crc = crc16_next(crc, buf[n]);

	return crc;

}


