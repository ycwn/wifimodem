
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "core/types.h"
#include "core/debug.h"
#include "core/str.h"
#include "core/mempool.h"
#include "core/util.h"

#include "net/netaddr.h"



int netaddr_from_addr(netaddr *a, const struct sockaddr *sa, uint port)
{

	memset(a, 0, sizeof(netaddr));

	switch (sa->sa_family) {

		case AF_INET:
			a->net4          = *(struct sockaddr_in*)sa;
			a->net4.sin_port = htons(port);
			a->length        = sizeof(struct sockaddr_in);
			break;

		case AF_INET6:
			a->net6           = *(struct sockaddr_in6*)sa;
			a->net6.sin6_port = htons(port);
			a->length         = sizeof(struct sockaddr_in6);
			break;

		default:
			return -1;

	}

	return 0;

}



void netaddr_from_ipv4(netaddr *a, int family, const uint ip[], uint port)
{

	memset(a, 0, sizeof(netaddr));

	a->net4.sin_family      = AF_INET;
	a->net4.sin_addr.s_addr = ntohl(mkdword4(ip[0], ip[1], ip[2], ip[3]));
	a->net4.sin_port        = htons(port);

}



int netaddr_resolve(netaddr *a, int family, const str *host, uint port)
{

	MEMBLOCK_ALLOC(char, hostname);

	memset(a, 0, sizeof(netaddr));
	memcpy(hostname, host->ptr, host->len);
	hostname[host->len] = 0;

	struct addrinfo *info = NULL;
	struct addrinfo  hint = {};

	hint.ai_flags  = AI_ADDRCONFIG;
	hint.ai_family = family;

	int rv = getaddrinfo(hostname, NULL, &hint, &info);

	if (info != NULL) {

		netaddr_from_addr(a, info->ai_addr, port);
		freeaddrinfo(info);

	}

	return rv;

}



char *netaddr_format(const netaddr *a, char *buf, uint len)
{

	const char *s = NULL;
	int         p = -1;


	switch (a->addr.sa_family) {

		case AF_INET:
			s = inet_ntop(AF_INET, &a->net4.sin_addr, buf, len);
			p = a->net4.sin_port;
			break;

		case AF_INET6:
			s = inet_ntop(AF_INET6, &a->net6.sin6_addr, buf, len);
			p = a->net6.sin6_port;
			break;

		default:
			snprintf(buf, len - 1, "<UNKNNOWN FAMILY %d>", a->addr.sa_family);
			return buf;

	}

	if (s != NULL) {

		if (p > 0) {

			const int slen = strlen(buf);
			snprintf(buf + slen, len - slen - 1, ":%d", ntohs(p));

		}

	} else
		snprintf(buf, len - 1, "<inet_ntop() failed, errno=%d>", errno);

	return buf;

}



int netaddr_compare(const netaddr *a, const netaddr *b)
{

	if (b->addr.sa_family != a->addr.sa_family)
		return b->addr.sa_family - a->addr.sa_family;

	if (a->addr.sa_family == AF_INET)
		return memcmp(&a->net4.sin_addr,  &b->net4.sin_addr,  sizeof(struct in_addr));

	if (a->addr.sa_family == AF_INET6)
		return memcmp(&a->net6.sin6_addr, &b->net6.sin6_addr, sizeof(struct in6_addr));

	return 0;

}


