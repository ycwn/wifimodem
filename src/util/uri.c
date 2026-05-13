
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

#include "core/types.h"
#include "core/str.h"

#include "util/uri.h"



void uri_clear(uri *u)
{

	u->orig      = str_zero();
	u->scheme    = str_zero();
	u->authority = str_zero();
	u->user      = str_zero();
	u->pass      = str_zero();
	u->host      = str_zero();
	u->path      = str_zero();
	u->query     = str_zero();
	u->fragment  = str_zero();

	u->port = -1;

	u->have_scheme    = false;
	u->have_authority = false;
	u->have_query     = false;
	u->have_fragment  = false;

}



void uri_parse(uri *u, const str *s)
{

	uri_clear(u);

	u->orig = *s;

	str tok = *s;
	str tmp  = tok;
	str part = str_cspan(&tok, ":/?#");

	if (str_accept(&tok, &CSTR(":"))) {  // Scheme

		u->scheme      = part;
		u->have_scheme = true;

	} else
		tok = tmp;


	if (str_accept(&tok, &CSTR("//"))) {   // Authority

		u->authority      = str_cspan(&tok, "/");
		u->have_authority = true;

		tmp  = u->authority;
		part = str_cspan(&tmp, "@");

		if (str_accept(&tmp, &CSTR("@"))) {

			u->user = str_cspan(&part, ":");

			if (str_accept(&part, &CSTR(":")))
				u->pass = part;

		} else
			tmp = part;

		if (str_accept(&tmp, &CSTR("["))) {

			u->host = str_cspan(&tmp, "]");
			str_accept(&tmp, &CSTR("]"));

		} else
			u->host = str_cspan(&tmp, ":");

		if (str_accept(&tmp, &CSTR(":")))
			str_readuint(&tmp, (uint*)&u->port);

	}

	u->path = str_cspan(&tok, "?#");

	if (str_accept(&tok, &CSTR("?"))) {

		u->have_query = true;
		u->query      = str_cspan(&tok, "#");

	}

	if (str_accept(&tok, &CSTR("#"))) {

		u->have_fragment = true;
		u->fragment      = tok;

	}

}


