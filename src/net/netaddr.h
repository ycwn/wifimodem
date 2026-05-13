

#ifndef NET_NETADDR_H
#define NET_NETADDR_H


typedef struct {

	union {

		struct sockaddr     addr;
		struct sockaddr_in  net4;
		struct sockaddr_in6 net6;

	};

	socklen_t length;

} netaddr;



int  netaddr_from_addr(netaddr *a, const struct sockaddr *sa, uint port);
void netaddr_from_ipv4(netaddr *a, int family, const uint ip[], uint port);
int  netaddr_resolve(  netaddr *a, int family, const str *host, uint port);

char *netaddr_format( const netaddr *a, char *buf, uint len);
int   netaddr_compare(const netaddr *a, const netaddr *b);


#endif


