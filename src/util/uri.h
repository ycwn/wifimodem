

#ifndef UTIL_URI_H
#define UTIL_URI_H


typedef struct {

	str orig;
	str scheme;
	str authority;
	str user;
	str pass;
	str host;
	int port;
	str path;
	str query;
	str fragment;

	bool have_scheme;
	bool have_authority;
	bool have_query;
	bool have_fragment;

} uri;


void uri_clear(uri *u);
void uri_parse(uri *u, const str *s);



#endif


