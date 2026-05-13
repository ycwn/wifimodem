

#ifndef UTIL_CRC16_H
#define UTIL_CRC16_H


static inline uint crc16_init() {
	return 0;
}

u16 crc16_next(u16 crc, u8 ch);
u16 crc16_buf(u16 crc, const u8 *buf, uint len);


#endif


