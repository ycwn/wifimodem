
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "core/types.h"
#include "core/debug.h"
#include "core/str.h"
#include "repl.h"

#include "util/repl.h"



void repl_init(repl *r)
{

	r->p.buf = NULL;
	r->p.len = 0;

	r->p.done = false;

	r->data = NULL;
	r->eval = NULL;
	r->gets = NULL;

	r->prefix   = CSTR("");
	r->keywords = NULL;

	r->igncase = false;

}



void repl_setbuf(repl *r, char *buf, uint len)
{

	r->p.buf = buf;
	r->p.len = len;

}



void repl_run(repl *r)
{

	r->p.done = false;

	while (!r->p.done) {

		int num = -1;

		if (r->gets != NULL)
			num = (r->gets)(r->data, r->p.buf, r->p.len);

		if (num < 0)
			break;

		repl_eval(r, &CSTRN(r->p.buf, num));

	}

	r->p.done = false;

}



void repl_quit(repl *r)
{

	r->p.done = true;
}



void repl_eval(repl *r, const str *cmd)
{

	str tok = *cmd;

	int argc = 0;
	str argv[REPL_ARGV_NUM];
	int argk[REPL_ARGV_NUM];

	str_ltrim(&tok);

	if (tok.len == 0)
		return;

	if (r->prefix.len == 0 || str_accept(&tok, &r->prefix)) {

		while (argc < REPL_ARGV_NUM - 1) {

			// This will bite us in the ass if cmd isnt part of buf
			uint l = r->p.buf + r->p.len - tok.ptr;

			// This should be safe as we at most copy the string as is
			// In cases where quotes and escaped chars are involved we write fewer chars
			// Its horribly innefficient though
			if (!str_readqstr(&tok, &argv[argc], (char*)tok.ptr, l)) {

				str_readstr(&tok, &argv[argc], (char*)tok.ptr, l, " \t");

				if (argv[argc].len == 0)
					break;

			}

			argc++;

			str_ltrim(&tok);

		}

		str_trim(&tok);

		if (tok.len > 0) {

			argv[argc] = tok;
			argc++;

		}

		if (r->keywords != NULL)
			for (int i=0; i < argc; i++)
				argk[i] = r->igncase?
					str_matchiv(&argv[i], r->keywords):
					str_matchv( &argv[i], r->keywords);

		else
			for (int i=0; i < argc; i++)
				argk[i] = REPL_UNKNOWN;

	} else {

		argc    = 1;
		argv[0] = tok;
		argk[0] = REPL_TEXT;

	}

	if (r->eval != NULL)
		r->eval(r->data, argc, (const str*)argv, argk);

}


