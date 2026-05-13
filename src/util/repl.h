

#ifndef UTIL_REPL_H
#define UTIL_REPL_H


enum {
	REPL_ARGV_NUM = 16
};

enum {
	REPL_UNKNOWN,
	REPL_TEXT
};


typedef void (repl_eval_proc)(void *data, int argc, const str argv[], const int argk[]);
typedef int  (repl_gets_proc)(void *data, char *buf, uint len);


typedef struct {

	struct {

		char *buf;
		uint  len;

		bool done;

	} p;


	void *data;

	repl_eval_proc *eval;
	repl_gets_proc *gets;

	str        prefix;
	const str *keywords;

	bool igncase;

} repl;


void repl_init(repl *r);
void repl_del( repl *r);

void repl_setbuf(repl *r, char *buf, uint len);

void repl_run( repl *r);
void repl_quit(repl *r);
void repl_eval(repl *r, const str *cmd);


#endif


