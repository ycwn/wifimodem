
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include <unistd.h>

#include <esp_attr.h>
#include <esp_log.h>

#include "core/types.h"
#include "core/debug.h"
#include "core/str.h"
#include "core/util.h"
#include "core/logger.h"
#include "core/mempool.h"


enum {
	TMP_SIZE = 256,
	LOG_SIZE = 7872,

	HEXDUMP_WIDTH = 16
};


static int  esp_printfv(const char *fmt, va_list argv);
static void log_write(const char *msg, uint len);


static const str level_code[] = {
	CSTR("[ DEBUG ] "),
	CSTR("[ INFO  ] "),
	CSTR("[ WARN  ] "),
	CSTR("[ ERROR ] ")
};


static RTC_DATA_ATTR struct {

	bool enabled[LOG_NUM];
	bool direct;
	bool wrap;

	u16 length;

	char temp[TMP_SIZE];
	char text[LOG_SIZE];

} g_log;




void log_init()
{

	g_log.direct = false;

	g_log.enabled[LOG_DEBUG] = true;
	g_log.enabled[LOG_INFO]  = true;
	g_log.enabled[LOG_WARN]  = true;
	g_log.enabled[LOG_ERROR] = true;

	log_clear();

	esp_log_set_vprintf(esp_printfv);

}



void log_clear()
{

	g_log.length = 0;
	g_log.wrap   = false;

}



bool log_get(int opt)
{

	switch (opt) {

		case LOG_DEBUG:
		case LOG_INFO:
		case LOG_WARN:
		case LOG_ERROR:
			return g_log.enabled[opt];

		case LOG_DIRECT:
			return g_log.direct;

	}

	return false;

}



void log_set(int opt, bool set)
{

	switch (opt) {

		case LOG_DEBUG:
		case LOG_INFO:
		case LOG_WARN:
		case LOG_ERROR:
			g_log.enabled[opt] = set;
			break;

		case LOG_DIRECT:
			g_log.direct = set;
			break;

	}

}



str log_str()
{

	if (g_log.wrap) { // If this flag is set we need to rotate

		MEMBLOCK_ALLOC(char, tmp);

		while (g_log.length > 0) {

			uint num = minu(g_log.length, tmp_size);
			uint rem = sizeof(g_log.text) - num;

			memcpy(tmp,              g_log.text,       num);
			memcpy(g_log.text,       g_log.text + num, rem);
			memcpy(g_log.text + rem, tmp,              num);

			g_log.length -= num;

		}

		g_log.wrap   = false;
		g_log.length = sizeof(g_log.text);

	}

	return CSTRN(g_log.text, g_log.length);

}



void log_printf(uint level, const char *msg, ...)
{

	if (level >= LOG_NUM || !g_log.enabled[level])
		return;

	const str *lc = &level_code[level];

	log_write(lc->ptr, lc->len);

	va_list argv;
	va_start(argv);

	int len = vsnprintf(g_log.temp, sizeof(g_log.temp), msg, argv);
	va_end(argv);

	if (len > 0)
		log_write(g_log.temp, len);

}



void log_puts(uint level, const char *msg, uint len)
{

	if (level >= LOG_NUM || !g_log.enabled[level])
		return;

	if (len == 0)
		len = strlen(msg);

	log_write(msg, len);

}



void log_hexdump(uint level, const void *buf, size_t len)
{

	if (level >= LOG_NUM || !g_log.enabled[level])
		return;

	static const char *xdigits = "0123456789ABCDEF";

	char cv[HEXDUMP_WIDTH + 2];
	char hv[HEXDUMP_WIDTH * 3 + 2];

	auto base = (const u8*)buf;
	auto lim  = base + len;
	auto bptr = (const u8*)((uintptr_t)base - ((uintptr_t)base & (HEXDUMP_WIDTH - 1)) );
	auto eptr = (const u8*)((uintptr_t)lim  - ((uintptr_t)lim  & (HEXDUMP_WIDTH - 1)) + HEXDUMP_WIDTH);

	memset(cv, ' ', sizeof(cv) - 1); cv[sizeof(cv) - 1] = 0;
	memset(hv, ' ', sizeof(hv) - 1); hv[sizeof(hv) - 1] = 0;

	for (auto ptr=bptr; ptr < eptr; ptr += HEXDUMP_WIDTH) {

		for (int n=0; n < HEXDUMP_WIDTH; n++) {

			int m = (n >= 8)? 1: 0;
			int k = 3 * n + m;

			if ((ptr + n) >= base && (ptr + n) < lim) {

				u8 c = ptr[n];

				cv[n + m] = isprint(c)? c: '.';
				hv[k + 1] = xdigits[(c >> 4) & 15];
				hv[k + 2] = xdigits[(c >> 0) & 15];

			} else
				cv[n + m] = hv[k + 1] = hv[k + 2] = ' ';

		}

		log_printf(level, "%p |%s | %s\n", ptr, hv, cv);

	}

}



int esp_printfv(const char *fmt, va_list argv)
{

	int ofs = 0;
	int len = vsnprintf(g_log.temp, sizeof(g_log.temp), fmt, argv);

	if (len > 2) {

		int level =
			(g_log.temp[0] == 'D')? LOG_DEBUG:
			(g_log.temp[0] == 'I')? LOG_INFO:
			(g_log.temp[0] == 'W')? LOG_WARN:
			(g_log.temp[0] == 'E')? LOG_ERROR: -1;

		if (level >= 0) {

			const str *code = &level_code[level];
			log_write(code->ptr, code->len);

			ofs = 2;

		}

	}

	log_write("esp-idf: ", 9);
	log_write(g_log.temp + ofs, len - ofs);

	return len;

}


void log_write(const char *msg, uint len)
{

	if (g_log.direct)
		write(STDOUT_FILENO, msg, len);

	if (len > sizeof(g_log.text)) {

		msg += len - sizeof(g_log.text);
		len  = sizeof(g_log.text);

	}


	uint num = minu(sizeof(g_log.text) - g_log.length, len);

	if (num > 0) {

		memcpy(g_log.text + g_log.length, msg, num);

		msg += num;
		len -= num;

		g_log.length += num;

	}

	if (len > 0) { // Wrap around

		memcpy(g_log.text, msg, len);

		g_log.length += len;

	}

	if (g_log.length >= sizeof(g_log.text)) {

		g_log.length -= sizeof(g_log.text);
		g_log.wrap    = true;

	}

}


