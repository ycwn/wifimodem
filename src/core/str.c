
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>

#include "core/types.h"
#include "core/str.h"



str str_left(const str *s, uint pos)
{

	if (pos > s->len)
		pos = s->len;

	return (str){ s->ptr, pos };

}



str str_right(const str *s, uint pos)
{

	if (pos > s->len)
		pos = s->len;

	return (str){ s->ptr + s->len - pos, pos };

}



str str_mid(const str *s, uint pos, uint len)
{

	if (pos > s->len)
		pos = s->len;

	const char *p = s->ptr + pos;
	uint        l = s->len - pos;

	if (l > len)
		l = len;

	return (str){ p, l };

}



void str_skip(str *s, uint num)
{

	if (num > s->len)
		num = s->len;

	s->ptr += num;
	s->len -= num;

}



void str_chop(str *s, uint num)
{

	if (num > s->len)
		num = s->len;

	s->len -= num;

}



void str_ltrim(str *s)
{

	while (s->len > 0 && isspace((int)*s->ptr)) {

		s->ptr++;
		s->len--;

	}

}



void str_rtrim(str *s)
{

	while (s->len > 0 && isspace((int)s->ptr[s->len - 1]))
		s->len--;

}



void str_pltrim(str *s, str_pred *pred)
{

	while (s->len > 0 && pred(*s->ptr)) {

		s->ptr++;
		s->len--;

	}

}



void str_prtrim(str *s, str_pred *pred)
{

	while (s->len > 0 && pred(s->ptr[s->len - 1]))
		s->len--;

}



str str_split(str *s, uint pos)
{

	if (pos > s->len)
		pos = s->len;

	const char *ptr = s->ptr;

	s->ptr = s->ptr + pos;
	s->len = s->len - pos;

	return (str){ ptr, pos };

}



str str_span(str *s, const char *p)
{

	int pos = 0;

	while ((pos < s->len) && strchr(p, s->ptr[pos]))
		pos++;

	return str_split(s, pos);

}



str str_cspan(str *s, const char *p)
{

	int pos = 0;

	while ((pos < s->len) && !strchr(p, s->ptr[pos]))
		pos++;

	return str_split(s, pos);

}



str str_pspan(str *s, str_pred *pred)
{

	int pos = 0;

	while ((pos < s->len) && pred(s->ptr[pos]))
		pos++;

	return str_split(s, pos);

}



str str_pcspan(str *s, str_pred *pred)
{

	int pos = 0;

	while ((pos < s->len) && !pred(s->ptr[pos]))
		pos++;

	return str_split(s, pos);

}



int str_cmp(const str *a, const str *b)
{

	int l = (a->len < b->len)? a->len: b->len;
	int r = strncmp(a->ptr, b->ptr, l);

	if (r != 0)
		return r;

	if (a->len > b->len) return +a->ptr[b->len];
	if (b->len > a->len) return -b->ptr[a->len];

	return 0;

}



int str_cmpi(const str *a, const str *b)
{

	int l = (a->len < b->len)? a->len: b->len;
	int r = strncasecmp(a->ptr, b->ptr, l);

	if (r != 0)
		return r;

	if (a->len > b->len) return +tolower(a->ptr[b->len]);
	if (b->len > a->len) return -tolower(b->ptr[a->len]);

	return 0;

}



int str_find(const str *s, const str *p)
{

	for (int i=0, j=s->len - p->len; i <= j; i++)
		if (!str_cmp(&(str){ .ptr=s->ptr + i, .len=p->len }, p))
			return i;

	return -1;

}



int str_findi(const str *s, const str *p)
{

	for (int i=0, j=s->len - p->len; i <= j; i++)
		if (!str_cmpi(&(str){ .ptr=s->ptr + i, .len=p->len }, p))
			return i;

	return -1;

}



int str_rfind(const str *s, const str *p)
{

	for (int i=s->len - p->len; i >= 0; i--)
		if (!str_cmp(&(str){ .ptr=s->ptr + i, .len=p->len }, p))
			return i;

	return -1;

}



int str_rfindi(const str *s, const str *p)
{

	for (int i=s->len - p->len; i >= 0; i--)
		if (!str_cmpi(&(str){ .ptr=s->ptr + i, .len=p->len }, p))
			return i;

	return -1;

}



bool str_match(const str *s, const str *p)
{

	if (p->len > s->len)
		return false;

	return !strncmp(s->ptr, p->ptr, p->len);

}



bool str_matchi(const str *s, const str *p)
{

	if (p->len > s->len)
		return false;

	return !strncasecmp(s->ptr, p->ptr, p->len);

}



int str_matchv(const str *s, const str p[])
{

	for (int i=0; p[i].len != 0; i++)
		if (str_match(s, &p[i]))
			return i;

	return -1;

}



int str_matchiv(const str *s, const str p[])
{

	for (int i=0; p[i].len != 0; i++)
		if (str_matchi(s, &p[i]))
			return i;

	return -1;

}



bool str_accept(str *s, const str *p)
{

	if (str_match(s, p)) {

		str_skip(s, p->len);
		return true;

	}

	return false;

}



bool str_accepti(str *s, const str *p)
{

	if (str_matchi(s, p)) {

		str_skip(s, p->len);
		return true;

	}

	return false;

}



int str_acceptv(str *s, const str p[])
{

	int i = str_matchv(s, p);

	if (i >= 0)
		str_skip(s, p[i].len);

	return i;

}



int str_acceptiv(str *s, const str p[])
{

	int i = str_matchiv(s, p);

	if (i >= 0)
		str_skip(s, p[i].len);

	return i;

}



bool str_readu8(str *s, u8 *v, uint r)
{

	u64 x;

	if (!str_readu64(s, &x, r))
		return false;

	if (x > UINT8_MAX)
		return false;

	if (v != NULL)
		*v = x;

	return true;

}



bool str_readu16(str *s, u16 *v, uint r)
{

	u64 x;

	if (!str_readu64(s, &x, r))
		return false;

	if (x > UINT16_MAX)
		return false;

	if (v != NULL)
		*v = x;

	return true;

}



bool str_readu32(str *s, u32 *v, uint r)
{

	u64 x;

	if (!str_readu64(s, &x, r))
		return false;

	if (x > UINT32_MAX)
		return false;

	if (v != NULL)
		*v = x;

	return true;

}



bool str_readu64(str *s, u64 *v, uint r)
{

	u64  value = 0ULL;
	u64  radix = 10ULL;
	bool valid = false;

	if (r > 36)
		return false;

	if (r > 0)
		radix = r;

	while (s->len > 0) {

		int c = (u8)*s->ptr;
		u64 v;

		if      (c >= '0' && c <= '9') v = c - '0';
		else if (c >= 'A' && c <= 'Z') v = c - 'A' + 10;
		else if (c >= 'a' && c <= 'z') v = c - 'a' + 10;
		else
			break;

		if (v >= radix)
			break;

		value = value * radix + v;
		valid = true;

		str_skip(s, 1);

	}

	if (v != NULL)
		*v = value;

	return valid;

}



bool str_readi8(str *s, i8 *v, uint r)
{

	i64 x;

	if (!str_readi64(s, &x, r))
		return false;

	if ((x < INT8_MIN) || (x > INT8_MAX))
		return false;

	if (v != NULL)
		*v = x;

	return true;

}



bool str_readi16(str *s, i16 *v, uint r)
{

	i64 x;

	if (!str_readi64(s, &x, r))
		return false;

	if ((x < INT16_MIN) || (x > INT16_MAX))
		return false;

	if (v != NULL)
		*v = x;

	return true;

}



bool str_readi32(str *s, i32 *v, uint r)
{

	i64 x;

	if (!str_readi64(s, &x, r))
		return false;

	if ((x < INT32_MIN) || (x > INT32_MAX))
		return false;

	if (v != NULL)
		*v = x;

	return true;

}



bool str_readi64(str *s, i64 *v, uint r)
{

	u64  value = 0;
	bool minus = false;

	if (str_accept(s, &CSTR("-")))
		minus = true;

	else
		str_accept(s, &CSTR("+"));

	if (!str_readu64(s, &value, r))
		return false;

	if (v != NULL)
		*v = minus? -value: +value;

	return true;

}



bool str_readf32(str *s, float *v)
{

	double x;

	if (!str_readf64(s, &x))
		return false;

	if (v != NULL)
		*v = x;

	return true;

}



bool str_readf64(str *s, double *v)
{

	i64 i = 0;
	u64 f = 0;
	i64 e = 0;
	int c = 0;

	if (!str_readint(s, &i))
		return false;

	if (str_accept(s, &CSTR("."))) {

		c = s->len;

		if (str_readuint(s, &f))
			c -= s->len;

		else
			c = 0;

	}

	if ((str_accept(s, &CSTR("e")) || str_accept(s, &CSTR("E"))) && !str_readint(s, &e))
		return false;

	if (v != NULL)
		*v = (((double)i) + ((double)f) * pow(10.0, -c)) * pow(10.0, e);

	return true;

}



void str_readstr(str *s, str *v, char *buf, uint len, const char *delm)
{

	if (delm == NULL)
		delm = ":;, \t\r\n";

	bool esc = false;
	uint num = 0;

	while ((s->len > 0) && (num < len)) {

		int ch = *s->ptr;

		if (esc) {

			switch (ch) {

				case 'a': case 'A': ch = '\a'; break;
				case 'b': case 'B': ch = '\b'; break;
				case 'f': case 'F': ch = '\f'; break;
				case 'n': case 'N': ch = '\n'; break;
				case 'r': case 'R': ch = '\r'; break;
				case 't': case 'T': ch = '\t'; break;
				case 'v': case 'V': ch = '\v'; break;

				case '\\': ch = '\\'; break;
				case '\'': ch = '\''; break;
				case '\"': ch = '\"'; break;

			}

			esc = false;

		} else if (ch == '\\') {

			str_skip(s, 1);
			esc = true;
			continue;

		} else if (strchr(delm, ch))
			break;

		buf[num++] = ch;
		str_skip(s, 1);

	}

	if (v != NULL)
		*v = str_setp(buf, num);

}



bool str_readqstr(str *s, str *v, char *buf, uint len)
{

	if (str_accept(s, &CSTR("\""))) {

		str_readstr(s, v, buf, len, "\"");
		return str_accept(s, &CSTR("\""));

	} else if (str_accept(s, &CSTR("\'"))) {

		str_readstr(s, v, buf, len, "\'");
		return str_accept(s, &CSTR("\'"));

	}

	return false;

}



char *str_zcpy(const str *s, char *buf, uint len)
{

	if (s->len >= len)
		return NULL;

	memcpy(buf, s->ptr, s->len);
	buf[s->len] = 0;

	return buf;

}


