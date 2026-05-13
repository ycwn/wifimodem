

#ifndef UTIL_NET_H
#define UTIL_NET_H


#define MACADDR_STR_F     "%02x:%02x:%02x:%02x:%02x:%02x"
#define MACADDR_ARG_F(m)  (m)[0], (m)[1], (m)[2], (m)[3], (m)[4], (m)[5]

#define IPV4ADDR_STR_F      "%d.%d.%d.%d"
#define IPV4ADDR_ARG_F(ip)  (((ip) >> 0) & 255), (((ip) >> 8) & 255), (((ip) >>  16) & 255), (((ip) >> 24) & 255)


enum {

	// 100: Informational
	HTTP_CONTINUE       = 100,
	HTTP_SWITCHINGPROTO = 101,
	HTTP_PROCESSING     = 102,
	HTTP_EARLYHINTS     = 103,

	// 200: Success
	HTTP_OK              = 200,
	HTTP_CREATED         = 201,
	HTTP_ACCEPTED        = 202,
	HTTP_NONAUTHINFO     = 203,
	HTTP_NOCONTENT       = 204,
	HTTP_RESETCONTENT    = 205,
	HTTP_PARTIALCONTENT  = 206,
	HTTP_MULTISTATUS     = 207,
	HTTP_ALREADYREPORTED = 208,
	HTTP_IMUSED          = 211,

	// 300: Redirection
	HTTP_MULTICHOICE   = 300,
	HTTP_MOVED         = 301,
	HTTP_FOUND         = 302,
	HTTP_SEEOTHER      = 303,
	HTTP_NOTMODIFIED   = 304,
	HTTP_USEPROXY      = 305,
	HTTP_SWITCHPROXY   = 306,
	HTTP_TEMPREDIR     = 307,
	HTTP_PERMREDIR     = 308,

	// 400: Client error
	HTTP_BADRQST     = 400,
	HTTP_UNAUTH      = 401,
	HTTP_PAYREQUIRED = 402,
	HTTP_FORBIDDEN   = 403,
	HTTP_NOTFOUND    = 404,
	HTTP_NOTALLOWED  = 405,
	HTTP_NOACCEPT    = 406,
	HTTP_PROXYAUTH   = 407,
	HTTP_OUTATIME    = 408,
	HTTP_CONFLICT    = 409,
	HTTP_GONE        = 410,
	HTTP_LENGTH      = 411,
	HTTP_PRECONDFAIL = 412,
	HTTP_TOOLARGE    = 413,
	HTTP_TOOLONG     = 414,
	HTTP_UNSUPPORT   = 415,
	HTTP_OUTARANGE   = 416,
	HTTP_EXPECTFAIL  = 417,
	HTTP_TEAPOT      = 418,
	HTTP_MISDIRECT   = 421,
	HTTP_UNPROCESS   = 422,
	HTTP_LOCKED      = 423,
	HTTP_FAILDEPEND  = 424,
	HTTP_TOOEARLY    = 425,
	HTTP_UPGRADE     = 426,
	HTTP_PRECOND     = 428,
	HTTP_MANYRQSTS   = 429,
	HTTP_HEADLARGE   = 431,
	HTTP_LEGAL       = 451,

	// 500: Server failed
	HTTP_INTERNAL = 500,
	HTTP_NOIMPL   = 501,
	HTTP_GWBAD    = 502,
	HTTP_UNAVAIL  = 503,
	HTTP_GWTIME   = 504,
	HTTP_VERSION  = 505,
	HTTP_VARNEG   = 506,
	HTTP_NOSPACE  = 507,
	HTTP_LOOP     = 508,
	HTTP_NOEXTEND = 510,
	HTTP_NETAUTH  = 511

};


int socket_open_host(int family, int type, int proto, const str *host, uint port, const str *local, uint lport, int backlog, int flags);
int socket_open_addr(int family, int type, int proto, const struct sockaddr *haddr, const struct sockaddr *laddr, int backlog, int flags);

int ipv4_parse(str *s, int addr[4]);
str ipv4_format(u32 addr, char *buf, uint len);

int mac_parse(str *s, int addr[6]);
str mac_format(u8 addr[6], char *buf, uint len);

const str *port_str(uint type, int no);
int        port_num(uint type, const str *srv);

const str *http_code_str(uint code);

inline bool http_is_info( uint code) { return code >= 100 && code <= 199; }
inline bool http_is_ok(   uint code) { return code >= 200 && code <= 299; }
inline bool http_is_redir(uint code) { return code >= 300 && code <= 399; }
inline bool http_is_error(uint code) { return code >= 400 && code <= 499; }
inline bool http_is_fail( uint code) { return code >= 500 && code <= 599; }


#endif


