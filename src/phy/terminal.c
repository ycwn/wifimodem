
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "core/types.h"
#include "core/debug.h"
#include "core/str.h"
#include "core/io.h"
#include "core/mempool.h"

#include "phy/terminal.h"



void terminal_init(terminal *t)
{

	io_zero(&t->io);

	terminal_set_msdos(t);

	t->chr_bs = '\b';
	t->chr_cr = '\r';
	t->chr_nl = '\n';

	t->echo = false;

}



int terminal_read(terminal *t, u8 *buf, uint len)
{

	int n=0;

	while (n < len) {

		int ch = terminal_getc(t);

		if (ch < 0)
			break;

		buf[n++] = ch;

	}

	return n;

}



int terminal_write(terminal *t, const u8 *buf, uint len)
{

	for (int n=0; n < len; n++)
		terminal_putc(t, buf[n]);

	return len;

}



int terminal_getc(terminal *t)
{

getc:
	int ch = io_getc(&t->io);

	if (ch < 0)
		return -1;

	if (ch == t->chr_cr) {

		if (t->ign_cr)
			goto getc;

		if (t->crlf)
			ch = '\n';

	} else if (ch == t->chr_nl)
		ch = '\n';

	if (t->echo)
		terminal_putc(t, ch);

	return ch;

}



int terminal_gets(terminal *t, char *buf, uint len)
{

	int n = 0;

	while (n < len) {

		int ch = terminal_getc(t);

		if (ch == t->chr_bs) {

			if (n > 0)
				buf[--n] = 0;

		} else if (ch == '\n')
			break;

		else if (ch >= 0)
			buf[n++] = ch;

	}

	return n;

}



str terminal_input(terminal *t, char *buf, uint len, const str *prompt, int hide)
{

	bool echo = t->echo;
	int  num  = 0;

	t->echo = hide < 0;

	if (prompt != NULL)
		terminal_write(t, (const u8*)prompt->ptr, prompt->len);

	while (num < len) {

		int ch = terminal_getc(t);

		if (ch < 0)
			continue;

		if (hide > 0)
			terminal_putc(t, isprint(ch)? hide: ch);

		if (ch == t->chr_bs) {

			if (num > 0)
				buf[--num] = 0;

		} else if (ch == '\n')
			break;

		else if (ch >= 0)
			buf[num++] = ch;

	}

	t->echo = echo;

	return CSTRN(buf, num);

}



void terminal_putc(terminal *t, int ch)
{

	if (ch == '\n') {

		if (t->add_cr)
			io_putc(&t->io, t->chr_cr);

		ch = t->crlf?
			t->chr_cr: t->chr_nl;

	}

	io_putc(&t->io, ch);

}



void terminal_puts(terminal *t, const char *buf)
{

	int ch;

	while ((ch = *buf++) != 0)
		terminal_putc(t, ch);

}



void terminal_printfv(terminal *t, const char *fmt, va_list argv)
{

	MEMBLOCK_ALLOC(char, buf);

	vsnprintf(buf, buf_size - 1, fmt, argv);
	terminal_puts(t, buf);

}



void terminal_printf(terminal *t, const char *fmt, ...)
{

	va_list argv;
	va_start(argv);

	terminal_printfv(t, fmt, argv);

	va_end(argv);

}


