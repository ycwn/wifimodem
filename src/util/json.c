

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "core/algorithm.h"
#include "core/types.h"
#include "core/debug.h"
#include "core/coroutine.h"
#include "core/util.h"

#include "util/buffer.h"
#include "util/json.h"


#define ADVANCE()   do { CO_YIELD(JSON_OK); } while (0)
#define ACCEPT(c)   do { buffer_putc(&jsn->buffer, c); ADVANCE(); } while (0)
#define SKIPSPACE() do { while (isspace(ch)) ADVANCE(); } while (0)
#define EXPECT(c)   do { if (ch != (c)) return (jsn->final = JSON_ERROR); } while (0)


static void json_stack_push(json *jsn);
static bool json_stack_pop( json *jsn);



void json_init(json *jsn)
{

	jsn->top   = 0;
	jsn->final = JSON_OK;

	CO_RESET(jsn->state);

}



/**
 *
 * JSON-DOCUMENT    ::= JSON-VALUE
 * JSON-VALUE       ::= JSON-NULL | JSON-BOOLEAN | JSON-NUMBER | JSON-STRING | JSON-OBJECT | JSON-ARRAY
 * JSON-NULL        ::= 'null'
 * JSON-BOOLEAN     ::= 'true' | 'false'
 * JSON-NUMBER      ::= [+-]?[0-9]*[.][0-9]+([eE][+-]?[0-9]+)?
 * JSON-STRING      ::= ["][^"]*["]
 * JSON-OBJECT      ::= '{' JSON-OBJECT-LIST? '}'
 * JSON-OBJECT-LIST ::= JSON-STRING ':' JSON-VALUE (',' JSON-OBJECT-LIST)?
 * JSON-ARRAY       ::= '[' JSON-ARRAY-LIST? ']'
 * JSON-ARRAY-LIST  ::= JSON-VALUE (',' JSON-ARRAY-LIST)?
 *
 **/
int json_parse(json *jsn, char ch)
{

	if (jsn->final != JSON_OK)
		return jsn->final;

	CO_BEGIN(jsn->state);

	SKIPSPACE();

	if (ch == 'n') {

		ADVANCE(); EXPECT('u');
		ADVANCE(); EXPECT('l');
		ADVANCE(); EXPECT('l');

		if (!json_stack_pop(jsn))
			jsn->final = JSON_DONE;

		return JSON_NULL;

	} else if (ch == 't') {

		ADVANCE(); EXPECT('r');
		ADVANCE(); EXPECT('u');
		ADVANCE(); EXPECT('e');

		if (!json_stack_pop(jsn))
			jsn->final = JSON_DONE;

		buffer_clear(&jsn->buffer);
		buffer_putsz(&jsn->buffer, "1");

		return JSON_BOOL;

	} else if (ch == 'f') {

		ADVANCE(); EXPECT('a');
		ADVANCE(); EXPECT('l');
		ADVANCE(); EXPECT('s');
		ADVANCE(); EXPECT('e');

		if (!json_stack_pop(jsn))
			jsn->final = JSON_DONE;

		buffer_clear(&jsn->buffer);
		buffer_putsz(&jsn->buffer, "0");

		return JSON_BOOL;

	} else if (ch == '"') {

		buffer_clear(&jsn->buffer);
		ADVANCE();

		while (ch != '"')
			ACCEPT(ch);

		buffer_putc(&jsn->buffer, 0);

		if (!json_stack_pop(jsn))
			jsn->final = JSON_DONE;

		return JSON_STRING;

	} else if (ch == '[') {

		CO_YIELD(JSON_ARRAY_BEGIN);
		SKIPSPACE();

		while (ch != ']') {

			CO_YIELD((json_stack_push(jsn), json_parse(jsn, ch)));
			SKIPSPACE();

			if (ch != ',')
				break;

			ADVANCE();

		}

		EXPECT(']');

		if (!json_stack_pop(jsn))
			jsn->final = JSON_DONE;

		return JSON_ARRAY_END;

	} else if (ch == '{') {

		CO_YIELD(JSON_OBJECT_BEGIN);
		SKIPSPACE();

		while (ch != '}') {

			SKIPSPACE();
			EXPECT('"');

			CO_YIELD((json_stack_push(jsn), json_parse(jsn, ch), JSON_OBJECT_KEY));

			SKIPSPACE();
			EXPECT(':');

			CO_YIELD(JSON_OBJECT_VALUE);

			CO_YIELD((json_stack_push(jsn), json_parse(jsn, ch)));
			SKIPSPACE();

			if (ch != ',')
				break;

			ADVANCE();

		}

		EXPECT('}');

		if (!json_stack_pop(jsn))
			jsn->final = JSON_DONE;

		return JSON_OBJECT_END;

	} else if (ch == '+' || ch == '-' || ch == '.' || isdigit(ch)) {

		buffer_clear(&jsn->buffer);

		if (ch == '+' || ch == '-')
			ACCEPT(ch);

		while (isdigit(ch))
			ACCEPT(ch);

		if (ch == '.') {

			ACCEPT(ch);

			while (isdigit(ch))
				ACCEPT(ch);

			if (ch == 'e' || ch == 'E') {

				ACCEPT(ch);

				if (ch == '+' || ch == '-')
					ACCEPT(ch);

				while (isdigit(ch))
					ACCEPT(ch);

			}

		}

		if (!json_stack_pop(jsn))
			jsn->final = JSON_DONE;

		buffer_putc(&jsn->buffer, 0);
		json_parse(jsn, ch);

		return JSON_NUMBER;

	}

	CO_END();

	jsn->final = JSON_ERROR;
	return JSON_ERROR;

}



const char *json_token(int tok)
{

	static const char *token_name[JSON_TOKEN_MAX] = {
		"DONE",         "OK",
		"BOOL",         "NUMBER",     "STRING",     "NULL",
		"ARRAY-BEGIN",  "ARRAY-END",
		"OBJECT-BEGIN", "OBJECT-END", "OBJECT-KEY", "OBJECT-VALUE"
	};

	if (tok < 0)
		return "ERROR";

	if (tok > JSON_TOKEN_MAX)
		return "UNKNOWN";

	return token_name[tok];

}



static void json_stack_push(json *jsn)
{

	if (jsn->top < PP_ARRAYCOUNT(jsn->stack)) {

		jsn->stack[jsn->top++] = jsn->state;
		CO_RESET(jsn->state);

		return;

	}

	jsn->final = JSON_ERROR;

}



static bool json_stack_pop(json *jsn)
{

	if (jsn->top > 0) {

		jsn->state = jsn->stack[--jsn->top];
		return true;

	}

	return false;

}


