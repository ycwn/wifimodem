
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "core/types.h"
#include "core/debug.h"
#include "core/str.h"
#include "core/util.h"

#include "util/net.h"

#include "net/netaddr.h"


static const struct {

	str  name;
	uint port;
	uint type;

} port_list[] = {

	{ CSTR("tcpmux"),            1,      SOCK_STREAM  },
	{ CSTR("echo"),              7,      SOCK_STREAM  },
	{ CSTR("echo"),              7,      SOCK_DGRAM   },
	{ CSTR("discard"),           9,      SOCK_STREAM  },
	{ CSTR("discard"),           9,      SOCK_DGRAM   },
	{ CSTR("systat"),            11,     SOCK_STREAM  },
	{ CSTR("daytime"),           13,     SOCK_STREAM  },
	{ CSTR("daytime"),           13,     SOCK_DGRAM   },
	{ CSTR("netstat"),           15,     SOCK_STREAM  },
	{ CSTR("qotd"),              17,     SOCK_STREAM  },
	{ CSTR("chargen"),           19,     SOCK_STREAM  },
	{ CSTR("chargen"),           19,     SOCK_DGRAM   },
	{ CSTR("ftp-data"),          20,     SOCK_STREAM  },
	{ CSTR("ftp"),               21,     SOCK_STREAM  },
	{ CSTR("fsp"),               21,     SOCK_DGRAM   },
	{ CSTR("ssh"),               22,     SOCK_STREAM  },
	{ CSTR("telnet"),            23,     SOCK_STREAM  },
	{ CSTR("smtp"),              25,     SOCK_STREAM  },
	{ CSTR("time"),              37,     SOCK_STREAM  },
	{ CSTR("time"),              37,     SOCK_DGRAM   },
	{ CSTR("whois"),             43,     SOCK_STREAM  },
	{ CSTR("tacacs"),            49,     SOCK_STREAM  },
	{ CSTR("tacacs"),            49,     SOCK_DGRAM   },
	{ CSTR("domain"),            53,     SOCK_STREAM  },
	{ CSTR("domain"),            53,     SOCK_DGRAM   },
	{ CSTR("bootps"),            67,     SOCK_DGRAM   },
	{ CSTR("bootpc"),            68,     SOCK_DGRAM   },
	{ CSTR("tftp"),              69,     SOCK_DGRAM   },
	{ CSTR("gopher"),            70,     SOCK_STREAM  },
	{ CSTR("finger"),            79,     SOCK_STREAM  },
	{ CSTR("http"),              80,     SOCK_STREAM  },
	{ CSTR("kerberos"),          88,     SOCK_STREAM  },
	{ CSTR("kerberos"),          88,     SOCK_DGRAM   },
	{ CSTR("iso-tsap"),          102,    SOCK_STREAM  },
	{ CSTR("acr-nema"),          104,    SOCK_STREAM  },
	{ CSTR("pop3"),              110,    SOCK_STREAM  },
	{ CSTR("sunrpc"),            111,    SOCK_STREAM  },
	{ CSTR("sunrpc"),            111,    SOCK_DGRAM   },
	{ CSTR("auth"),              113,    SOCK_STREAM  },
	{ CSTR("nntp"),              119,    SOCK_STREAM  },
	{ CSTR("ntp"),               123,    SOCK_DGRAM   },
	{ CSTR("epmap"),             135,    SOCK_STREAM  },
	{ CSTR("netbios-ns"),        137,    SOCK_DGRAM   },
	{ CSTR("netbios-dgm"),       138,    SOCK_DGRAM   },
	{ CSTR("netbios-ssn"),       139,    SOCK_STREAM  },
	{ CSTR("imap2"),             143,    SOCK_STREAM  },
	{ CSTR("snmp"),              161,    SOCK_STREAM  },
	{ CSTR("snmp"),              161,    SOCK_DGRAM   },
	{ CSTR("snmp-trap"),         162,    SOCK_STREAM  },
	{ CSTR("snmp-trap"),         162,    SOCK_DGRAM   },
	{ CSTR("cmip-man"),          163,    SOCK_STREAM  },
	{ CSTR("cmip-man"),          163,    SOCK_DGRAM   },
	{ CSTR("cmip-agent"),        164,    SOCK_STREAM  },
	{ CSTR("cmip-agent"),        164,    SOCK_DGRAM   },
	{ CSTR("mailq"),             174,    SOCK_STREAM  },
	{ CSTR("xdmcp"),             177,    SOCK_DGRAM   },
	{ CSTR("bgp"),               179,    SOCK_STREAM  },
	{ CSTR("smux"),              199,    SOCK_STREAM  },
	{ CSTR("qmtp"),              209,    SOCK_STREAM  },
	{ CSTR("z3950"),             210,    SOCK_STREAM  },
	{ CSTR("ipx"),               213,    SOCK_DGRAM   },
	{ CSTR("ptp-event"),         319,    SOCK_DGRAM   },
	{ CSTR("ptp-general"),       320,    SOCK_DGRAM   },
	{ CSTR("pawserv"),           345,    SOCK_STREAM  },
	{ CSTR("zserv"),             346,    SOCK_STREAM  },
	{ CSTR("rpc2portmap"),       369,    SOCK_STREAM  },
	{ CSTR("rpc2portmap"),       369,    SOCK_DGRAM   },
	{ CSTR("codaauth2"),         370,    SOCK_STREAM  },
	{ CSTR("codaauth2"),         370,    SOCK_DGRAM   },
	{ CSTR("clearcase"),         371,    SOCK_DGRAM   },
	{ CSTR("ldap"),              389,    SOCK_STREAM  },
	{ CSTR("ldap"),              389,    SOCK_DGRAM   },
	{ CSTR("svrloc"),            427,    SOCK_STREAM  },
	{ CSTR("svrloc"),            427,    SOCK_DGRAM   },
	{ CSTR("https"),             443,    SOCK_STREAM  },
	{ CSTR("https"),             443,    SOCK_DGRAM   },
	{ CSTR("snpp"),              444,    SOCK_STREAM  },
	{ CSTR("microsoft-ds"),      445,    SOCK_STREAM  },
	{ CSTR("kpasswd"),           464,    SOCK_STREAM  },
	{ CSTR("kpasswd"),           464,    SOCK_DGRAM   },
	{ CSTR("submissions"),       465,    SOCK_STREAM  },
	{ CSTR("saft"),              487,    SOCK_STREAM  },
	{ CSTR("isakmp"),            500,    SOCK_DGRAM   },
	{ CSTR("rtsp"),              554,    SOCK_STREAM  },
	{ CSTR("rtsp"),              554,    SOCK_DGRAM   },
	{ CSTR("nqs"),               607,    SOCK_STREAM  },
	{ CSTR("asf-rmcp"),          623,    SOCK_DGRAM   },
	{ CSTR("qmqp"),              628,    SOCK_STREAM  },
	{ CSTR("ipp"),               631,    SOCK_STREAM  },
	{ CSTR("ldp"),               646,    SOCK_STREAM  },
	{ CSTR("ldp"),               646,    SOCK_DGRAM   },
	{ CSTR("exec"),              512,    SOCK_STREAM  },
	{ CSTR("biff"),              512,    SOCK_DGRAM   },
	{ CSTR("login"),             513,    SOCK_STREAM  },
	{ CSTR("who"),               513,    SOCK_DGRAM   },
	{ CSTR("shell"),             514,    SOCK_STREAM  },
	{ CSTR("syslog"),            514,    SOCK_DGRAM   },
	{ CSTR("printer"),           515,    SOCK_STREAM  },
	{ CSTR("talk"),              517,    SOCK_DGRAM   },
	{ CSTR("ntalk"),             518,    SOCK_DGRAM   },
	{ CSTR("route"),             520,    SOCK_DGRAM   },
	{ CSTR("gdomap"),            538,    SOCK_STREAM  },
	{ CSTR("gdomap"),            538,    SOCK_DGRAM   },
	{ CSTR("uucp"),              540,    SOCK_STREAM  },
	{ CSTR("klogin"),            543,    SOCK_STREAM  },
	{ CSTR("kshell"),            544,    SOCK_STREAM  },
	{ CSTR("dhcpv6-client"),     546,    SOCK_DGRAM   },
	{ CSTR("dhcpv6-server"),     547,    SOCK_DGRAM   },
	{ CSTR("afpovertcp"),        548,    SOCK_STREAM  },
	{ CSTR("nntps"),             563,    SOCK_STREAM  },
	{ CSTR("submission"),        587,    SOCK_STREAM  },
	{ CSTR("ldaps"),             636,    SOCK_STREAM  },
	{ CSTR("ldaps"),             636,    SOCK_DGRAM   },
	{ CSTR("tinc"),              655,    SOCK_STREAM  },
	{ CSTR("tinc"),              655,    SOCK_DGRAM   },
	{ CSTR("silc"),              706,    SOCK_STREAM  },
	{ CSTR("kerberos-adm"),      749,    SOCK_STREAM  },
	{ CSTR("domain-s"),          853,    SOCK_STREAM  },
	{ CSTR("domain-s"),          853,    SOCK_DGRAM   },
	{ CSTR("rsync"),             873,    SOCK_STREAM  },
	{ CSTR("ftps-data"),         989,    SOCK_STREAM  },
	{ CSTR("ftps"),              990,    SOCK_STREAM  },
	{ CSTR("telnets"),           992,    SOCK_STREAM  },
	{ CSTR("imaps"),             993,    SOCK_STREAM  },
	{ CSTR("pop3s"),             995,    SOCK_STREAM  },
	{ CSTR("socks"),             1080,   SOCK_STREAM  },
	{ CSTR("proofd"),            1093,   SOCK_STREAM  },
	{ CSTR("rootd"),             1094,   SOCK_STREAM  },
	{ CSTR("openvpn"),           1194,   SOCK_STREAM  },
	{ CSTR("openvpn"),           1194,   SOCK_DGRAM   },
	{ CSTR("rmiregistry"),       1099,   SOCK_STREAM  },
	{ CSTR("lotusnote"),         1352,   SOCK_STREAM  },
	{ CSTR("ms-sql-s"),          1433,   SOCK_STREAM  },
	{ CSTR("ms-sql-m"),          1434,   SOCK_DGRAM   },
	{ CSTR("ingreslock"),        1524,   SOCK_STREAM  },
	{ CSTR("datametrics"),       1645,   SOCK_STREAM  },
	{ CSTR("datametrics"),       1645,   SOCK_DGRAM   },
	{ CSTR("sa-msg-port"),       1646,   SOCK_STREAM  },
	{ CSTR("sa-msg-port"),       1646,   SOCK_DGRAM   },
	{ CSTR("kermit"),            1649,   SOCK_STREAM  },
	{ CSTR("groupwise"),         1677,   SOCK_STREAM  },
	{ CSTR("l2f"),               1701,   SOCK_DGRAM   },
	{ CSTR("radius"),            1812,   SOCK_STREAM  },
	{ CSTR("radius"),            1812,   SOCK_DGRAM   },
	{ CSTR("radius-acct"),       1813,   SOCK_STREAM  },
	{ CSTR("radius-acct"),       1813,   SOCK_DGRAM   },
	{ CSTR("cisco-sccp"),        2000,   SOCK_STREAM  },
	{ CSTR("nfs"),               2049,   SOCK_STREAM  },
	{ CSTR("nfs"),               2049,   SOCK_DGRAM   },
	{ CSTR("gnunet"),            2086,   SOCK_STREAM  },
	{ CSTR("gnunet"),            2086,   SOCK_DGRAM   },
	{ CSTR("rtcm-sc104"),        2101,   SOCK_STREAM  },
	{ CSTR("rtcm-sc104"),        2101,   SOCK_DGRAM   },
	{ CSTR("gsigatekeeper"),     2119,   SOCK_STREAM  },
	{ CSTR("gris"),              2135,   SOCK_STREAM  },
	{ CSTR("cvspserver"),        2401,   SOCK_STREAM  },
	{ CSTR("venus"),             2430,   SOCK_STREAM  },
	{ CSTR("venus"),             2430,   SOCK_DGRAM   },
	{ CSTR("venus-se"),          2431,   SOCK_STREAM  },
	{ CSTR("venus-se"),          2431,   SOCK_DGRAM   },
	{ CSTR("codasrv"),           2432,   SOCK_STREAM  },
	{ CSTR("codasrv"),           2432,   SOCK_DGRAM   },
	{ CSTR("codasrv-se"),        2433,   SOCK_STREAM  },
	{ CSTR("codasrv-se"),        2433,   SOCK_DGRAM   },
	{ CSTR("mon"),               2583,   SOCK_STREAM  },
	{ CSTR("mon"),               2583,   SOCK_DGRAM   },
	{ CSTR("dict"),              2628,   SOCK_STREAM  },
	{ CSTR("f5-globalsite"),     2792,   SOCK_STREAM  },
	{ CSTR("gsiftp"),            2811,   SOCK_STREAM  },
	{ CSTR("gpsd"),              2947,   SOCK_STREAM  },
	{ CSTR("gds-db"),            3050,   SOCK_STREAM  },
	{ CSTR("icpv2"),             3130,   SOCK_DGRAM   },
	{ CSTR("isns"),              3205,   SOCK_STREAM  },
	{ CSTR("isns"),              3205,   SOCK_DGRAM   },
	{ CSTR("iscsi-target"),      3260,   SOCK_STREAM  },
	{ CSTR("mysql"),             3306,   SOCK_STREAM  },
	{ CSTR("ms-wbt-server"),     3389,   SOCK_STREAM  },
	{ CSTR("nut"),               3493,   SOCK_STREAM  },
	{ CSTR("nut"),               3493,   SOCK_DGRAM   },
	{ CSTR("distcc"),            3632,   SOCK_STREAM  },
	{ CSTR("daap"),              3689,   SOCK_STREAM  },
	{ CSTR("svn"),               3690,   SOCK_STREAM  },
	{ CSTR("suucp"),             4031,   SOCK_STREAM  },
	{ CSTR("sysrqd"),            4094,   SOCK_STREAM  },
	{ CSTR("sieve"),             4190,   SOCK_STREAM  },
	{ CSTR("epmd"),              4369,   SOCK_STREAM  },
	{ CSTR("remctl"),            4373,   SOCK_STREAM  },
	{ CSTR("f5-iquery"),         4353,   SOCK_STREAM  },
	{ CSTR("ntske"),             4460,   SOCK_STREAM  },
	{ CSTR("ipsec-nat-t"),       4500,   SOCK_DGRAM   },
	{ CSTR("iax"),               4569,   SOCK_DGRAM   },
	{ CSTR("mtn"),               4691,   SOCK_STREAM  },
	{ CSTR("radmin-port"),       4899,   SOCK_STREAM  },
	{ CSTR("sip"),               5060,   SOCK_STREAM  },
	{ CSTR("sip"),               5060,   SOCK_DGRAM   },
	{ CSTR("sip-tls"),           5061,   SOCK_STREAM  },
	{ CSTR("sip-tls"),           5061,   SOCK_DGRAM   },
	{ CSTR("xmpp-client"),       5222,   SOCK_STREAM  },
	{ CSTR("xmpp-server"),       5269,   SOCK_STREAM  },
	{ CSTR("cfengine"),          5308,   SOCK_STREAM  },
	{ CSTR("mdns"),              5353,   SOCK_DGRAM   },
	{ CSTR("postgresql"),        5432,   SOCK_STREAM  },
	{ CSTR("freeciv"),           5556,   SOCK_STREAM  },
	{ CSTR("amqps"),             5671,   SOCK_STREAM  },
	{ CSTR("amqp"),              5672,   SOCK_STREAM  },
	{ CSTR("x11"),               6000,   SOCK_STREAM  },
	{ CSTR("x11-1"),             6001,   SOCK_STREAM  },
	{ CSTR("x11-2"),             6002,   SOCK_STREAM  },
	{ CSTR("x11-3"),             6003,   SOCK_STREAM  },
	{ CSTR("x11-4"),             6004,   SOCK_STREAM  },
	{ CSTR("x11-5"),             6005,   SOCK_STREAM  },
	{ CSTR("x11-6"),             6006,   SOCK_STREAM  },
	{ CSTR("x11-7"),             6007,   SOCK_STREAM  },
	{ CSTR("gnutella-svc"),      6346,   SOCK_STREAM  },
	{ CSTR("gnutella-svc"),      6346,   SOCK_DGRAM   },
	{ CSTR("gnutella-rtr"),      6347,   SOCK_STREAM  },
	{ CSTR("gnutella-rtr"),      6347,   SOCK_DGRAM   },
	{ CSTR("redis"),             6379,   SOCK_STREAM  },
	{ CSTR("sge-qmaster"),       6444,   SOCK_STREAM  },
	{ CSTR("sge-execd"),         6445,   SOCK_STREAM  },
	{ CSTR("mysql-proxy"),       6446,   SOCK_STREAM  },
	{ CSTR("babel"),             6696,   SOCK_DGRAM   },
	{ CSTR("ircs-u"),            6697,   SOCK_STREAM  },
	{ CSTR("bbs"),               7000,   SOCK_STREAM  },
	{ CSTR("afs3-fileserver"),   7000,   SOCK_DGRAM   },
	{ CSTR("afs3-callback"),     7001,   SOCK_DGRAM   },
	{ CSTR("afs3-prserver"),     7002,   SOCK_DGRAM   },
	{ CSTR("afs3-vlserver"),     7003,   SOCK_DGRAM   },
	{ CSTR("afs3-kaserver"),     7004,   SOCK_DGRAM   },
	{ CSTR("afs3-volser"),       7005,   SOCK_DGRAM   },
	{ CSTR("afs3-bos"),          7007,   SOCK_DGRAM   },
	{ CSTR("afs3-update"),       7008,   SOCK_DGRAM   },
	{ CSTR("afs3-rmtsys"),       7009,   SOCK_DGRAM   },
	{ CSTR("font-service"),      7100,   SOCK_STREAM  },
	{ CSTR("http-alt"),          8080,   SOCK_STREAM  },
	{ CSTR("puppet"),            8140,   SOCK_STREAM  },
	{ CSTR("bacula-dir"),        9101,   SOCK_STREAM  },
	{ CSTR("bacula-fd"),         9102,   SOCK_STREAM  },
	{ CSTR("bacula-sd"),         9103,   SOCK_STREAM  },
	{ CSTR("xmms2"),             9667,   SOCK_STREAM  },
	{ CSTR("nbd"),               10809,  SOCK_STREAM  },
	{ CSTR("zabbix-agent"),      10050,  SOCK_STREAM  },
	{ CSTR("zabbix-trapper"),    10051,  SOCK_STREAM  },
	{ CSTR("amanda"),            10080,  SOCK_STREAM  },
	{ CSTR("dicom"),             11112,  SOCK_STREAM  },
	{ CSTR("hkp"),               11371,  SOCK_STREAM  },
	{ CSTR("db-lsp"),            17500,  SOCK_STREAM  },
	{ CSTR("dcap"),              22125,  SOCK_STREAM  },
	{ CSTR("gsidcap"),           22128,  SOCK_STREAM  },
	{ CSTR("wnn6"),              22273,  SOCK_STREAM  },
	{ CSTR("kerberos4"),         750,    SOCK_DGRAM   },
	{ CSTR("kerberos4"),         750,    SOCK_STREAM  },
	{ CSTR("kerberos-master"),   751,    SOCK_DGRAM   },
	{ CSTR("kerberos-master"),   751,    SOCK_STREAM  },
	{ CSTR("passwd-server"),     752,    SOCK_DGRAM   },
	{ CSTR("krb-prop"),          754,    SOCK_STREAM  },
	{ CSTR("zephyr-srv"),        2102,   SOCK_DGRAM   },
	{ CSTR("zephyr-clt"),        2103,   SOCK_DGRAM   },
	{ CSTR("zephyr-hm"),         2104,   SOCK_DGRAM   },
	{ CSTR("iprop"),             2121,   SOCK_STREAM  },
	{ CSTR("supfilesrv"),        871,    SOCK_STREAM  },
	{ CSTR("supfiledbg"),        1127,   SOCK_STREAM  },
	{ CSTR("poppassd"),          106,    SOCK_STREAM  },
	{ CSTR("moira-db"),          775,    SOCK_STREAM  },
	{ CSTR("moira-update"),      777,    SOCK_STREAM  },
	{ CSTR("moira-ureg"),        779,    SOCK_DGRAM   },
	{ CSTR("spamd"),             783,    SOCK_STREAM  },
	{ CSTR("skkserv"),           1178,   SOCK_STREAM  },
	{ CSTR("predict"),           1210,   SOCK_DGRAM   },
	{ CSTR("rmtcfg"),            1236,   SOCK_STREAM  },
	{ CSTR("xtel"),              1313,   SOCK_STREAM  },
	{ CSTR("xtelw"),             1314,   SOCK_STREAM  },
	{ CSTR("zebrasrv"),          2600,   SOCK_STREAM  },
	{ CSTR("zebra"),             2601,   SOCK_STREAM  },
	{ CSTR("ripd"),              2602,   SOCK_STREAM  },
	{ CSTR("ripngd"),            2603,   SOCK_STREAM  },
	{ CSTR("ospfd"),             2604,   SOCK_STREAM  },
	{ CSTR("bgpd"),              2605,   SOCK_STREAM  },
	{ CSTR("ospf6d"),            2606,   SOCK_STREAM  },
	{ CSTR("ospfapi"),           2607,   SOCK_STREAM  },
	{ CSTR("isisd"),             2608,   SOCK_STREAM  },
	{ CSTR("fax"),               4557,   SOCK_STREAM  },
	{ CSTR("hylafax"),           4559,   SOCK_STREAM  },
	{ CSTR("munin"),             4949,   SOCK_STREAM  },
	{ CSTR("rplay"),             5555,   SOCK_DGRAM   },
	{ CSTR("nrpe"),              5666,   SOCK_STREAM  },
	{ CSTR("nsca"),              5667,   SOCK_STREAM  },
	{ CSTR("canna"),             5680,   SOCK_STREAM  },
	{ CSTR("syslog-tls"),        6514,   SOCK_STREAM  },
	{ CSTR("sane-port"),         6566,   SOCK_STREAM  },
	{ CSTR("ircd"),              6667,   SOCK_STREAM  },
	{ CSTR("zope-ftp"),          8021,   SOCK_STREAM  },
	{ CSTR("tproxy"),            8081,   SOCK_STREAM  },
	{ CSTR("omniorb"),           8088,   SOCK_STREAM  },
	{ CSTR("clc-build-daemon"),  8990,   SOCK_STREAM  },
	{ CSTR("xinetd"),            9098,   SOCK_STREAM  },
	{ CSTR("git"),               9418,   SOCK_STREAM  },
	{ CSTR("zope"),              9673,   SOCK_STREAM  },
	{ CSTR("webmin"),            10000,  SOCK_STREAM  },
	{ CSTR("kamanda"),           10081,  SOCK_STREAM  },
	{ CSTR("amandaidx"),         10082,  SOCK_STREAM  },
	{ CSTR("amidxtape"),         10083,  SOCK_STREAM  },
	{ CSTR("sgi-cmsd"),          17001,  SOCK_DGRAM   },
	{ CSTR("sgi-crsd"),          17002,  SOCK_DGRAM   },
	{ CSTR("sgi-gcd"),           17003,  SOCK_DGRAM   },
	{ CSTR("sgi-cad"),           17004,  SOCK_STREAM  },
	{ CSTR("binkp"),             24554,  SOCK_STREAM  },
	{ CSTR("asp"),               27374,  SOCK_STREAM  },
	{ CSTR("asp"),               27374,  SOCK_DGRAM   },
	{ CSTR("csync2"),            30865,  SOCK_STREAM  },
	{ CSTR("dircproxy"),         57000,  SOCK_STREAM  },
	{ CSTR("tfido"),             60177,  SOCK_STREAM  },
	{ CSTR("fido"),              60179,  SOCK_STREAM  },

	{}

};



int socket_open_host(int family, int type, int proto, const str *host, uint port, const str *local, uint lport, int backlog, int flags)
{

	netaddr haddr, laddr;

	if (host != NULL && netaddr_resolve(&haddr, family, host, port) < 0)
		return -1;

	if (local != NULL && netaddr_resolve(&laddr, family, local, lport) < 0)
		return -1;

	return socket_open_addr(family, type, proto,
		(host  != NULL)? &haddr.addr: NULL,
		(local != NULL)? &laddr.addr: NULL,
		backlog, flags);

}



int socket_open_addr(int family, int type, int proto, const struct sockaddr *haddr, const struct sockaddr *laddr, int backlog, int flags)
{

	const socklen_t socklen =
		(family == AF_INET)?  sizeof(struct sockaddr_in):
		(family == AF_INET6)? sizeof(struct sockaddr_in6): 0;

	int sock = socket(family, type, proto);

	if (sock < 0)
		return -1;

	if (laddr != NULL && (socklen == 0 || bind(sock, laddr, socklen) < 0))
		goto fail;

	if (backlog >= 0 && listen(sock, backlog) < 0)
		goto fail;

	if (haddr != NULL && (socklen == 0 || connect(sock, haddr, socklen) < 0))
		goto fail;

	if (flags != 0) {

		int sockflags = fcntl(sock, F_GETFL, 0);

		if (sockflags >= 0)
			fcntl(sock, F_SETFL, sockflags | flags);

	}

	return sock;

fail:
	close(sock);
	return -1;

}



int ipv4_parse(str *s, int addr[4])
{

	int n = 0;

	while (n < 4) {

		if (n != 0 && !str_accept(s, &CSTR(".")))
			break;

		u8 part;

		if (str_readu8(s, &part, 0) <= 0)
			return -1;

		addr[n++] = part;

	}

	return n;

}



str ipv4_format(u32 addr, char *buf, uint len)
{

	int num = snprintf(buf, len, "%d.%d.%d.%d",
		byte(addr, 3), byte(addr, 2), byte(addr, 1), byte(addr, 0));

	if (num <= 0)
		return CSTR("");

	return CSTRN(buf, num);

}



int mac_parse(str *s, int addr[6])
{

	int n = 0;

	while (n < 6) {

		if (n != 0 && !str_accept(s, &CSTR(".")))
			break;

		u8 part;

		if (str_readu8(s, &part, 16) <= 0)
			return -1;

		addr[n++] = part;

	}

	return n;

}



str mac_format(u8 addr[6], char *buf, uint len)
{

	int num = snprintf(buf, len,
		"%02x:%02x:%02x:%02x:%02x:%02x",
		addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

	if (num <= 0)
		return CSTR("");

	return CSTRN(buf, num);

}



const str *port_str(uint type, int num)
{

	for (auto p = &port_list[0]; p->name.len > 0; p++)
		if ((type == 0 || p->type == type) && p->port == num)
			return &p->name;

	return NULL;

}



int port_num(uint type, const str *srv)
{

	for (auto p = &port_list[0]; p->name.len > 0; p++)
		if ((type == 0 || p->type == type) && !str_cmpi(&p->name, srv))
			return p->port;

	return -1;

}



const str *http_code_str(uint code)
{

	switch (code) {

		// 100: Informational
		case HTTP_CONTINUE:       return &CSTR("Continue");
		case HTTP_SWITCHINGPROTO: return &CSTR("Switching protocols");
		case HTTP_PROCESSING:     return &CSTR("Processing");
		case HTTP_EARLYHINTS:     return &CSTR("Early hints");

		// 200: Success
		case HTTP_OK:              return &CSTR("Ok");
		case HTTP_CREATED:         return &CSTR("Created");
		case HTTP_ACCEPTED:        return &CSTR("Accepted");
		case HTTP_NONAUTHINFO:     return &CSTR("Non-Authoritative Information");
		case HTTP_NOCONTENT:       return &CSTR("No Content");
		case HTTP_RESETCONTENT:    return &CSTR("Reset Content");
		case HTTP_PARTIALCONTENT:  return &CSTR("Partial Content");
		case HTTP_MULTISTATUS:     return &CSTR("Multi-Status");
		case HTTP_ALREADYREPORTED: return &CSTR("Already Reported");
		case HTTP_IMUSED:          return &CSTR("IM Used");

		// 300: Redirection
		case HTTP_MULTICHOICE: return &CSTR("Multiple Choices");
		case HTTP_MOVED:       return &CSTR("Moved Permanently");
		case HTTP_FOUND:       return &CSTR("Found");
		case HTTP_SEEOTHER:    return &CSTR("See Other");
		case HTTP_NOTMODIFIED: return &CSTR("Not Modified");
		case HTTP_USEPROXY:    return &CSTR("Use Proxy");
		case HTTP_SWITCHPROXY: return &CSTR("Switch Proxy");
		case HTTP_TEMPREDIR:   return &CSTR("Temporary Redirect");
		case HTTP_PERMREDIR:   return &CSTR("Permanent Redirect");

		// 400: Client error
		case HTTP_BADRQST:     return &CSTR("Bad Request");
		case HTTP_UNAUTH:      return &CSTR("Unauthorized");
		case HTTP_PAYREQUIRED: return &CSTR("Payment Required");
		case HTTP_FORBIDDEN:   return &CSTR("Forbidden");
		case HTTP_NOTFOUND:    return &CSTR("Not Found");
		case HTTP_NOTALLOWED:  return &CSTR("Method Not Allowed");
		case HTTP_NOACCEPT:    return &CSTR("Not Acceptable");
		case HTTP_PROXYAUTH:   return &CSTR("Proxy Authentication Required");
		case HTTP_OUTATIME:    return &CSTR("Request Timeout");
		case HTTP_CONFLICT:    return &CSTR("Conflict");
		case HTTP_GONE:        return &CSTR("Gone");
		case HTTP_LENGTH:      return &CSTR("Length Required");
		case HTTP_PRECONDFAIL: return &CSTR("Precondition Failed");
		case HTTP_TOOLARGE:    return &CSTR("Payload Too Large");
		case HTTP_TOOLONG:     return &CSTR("URI Too Long");
		case HTTP_UNSUPPORT:   return &CSTR("Unsupported Media Type");
		case HTTP_OUTARANGE:   return &CSTR("Range Not Satisfiable");
		case HTTP_EXPECTFAIL:  return &CSTR("Expectation Failed");
		case HTTP_TEAPOT:      return &CSTR("I'm a Teapot");
		case HTTP_MISDIRECT:   return &CSTR("Misdirected Request");
		case HTTP_UNPROCESS:   return &CSTR("Unprocessable Content");
		case HTTP_LOCKED:      return &CSTR("Locked");
		case HTTP_FAILDEPEND:  return &CSTR("Failed Dependency");
		case HTTP_TOOEARLY:    return &CSTR("Too Early");
		case HTTP_UPGRADE:     return &CSTR("Upgrade Required");
		case HTTP_PRECOND:     return &CSTR("Precondition Required");
		case HTTP_MANYRQSTS:   return &CSTR("Too Many Requests");
		case HTTP_HEADLARGE:   return &CSTR("Request Header Fields Too Large");
		case HTTP_LEGAL:       return &CSTR("Unavailable For Legal Reasons");

		// 500: Server failed
		case HTTP_INTERNAL: return &CSTR("Internal Server Error");
		case HTTP_NOIMPL:   return &CSTR("Not Implemented");
		case HTTP_GWBAD:    return &CSTR("Bad Gateway");
		case HTTP_UNAVAIL:  return &CSTR("Service Unavailable");
		case HTTP_GWTIME:   return &CSTR("Gateway Timeout");
		case HTTP_VERSION:  return &CSTR("HTTP Version Not Supported");
		case HTTP_VARNEG:   return &CSTR("Variant Also Negotiates");
		case HTTP_NOSPACE:  return &CSTR("Insufficient Storage");
		case HTTP_LOOP:     return &CSTR("Loop Detected");
		case HTTP_NOEXTEND: return &CSTR("Not Extended");
		case HTTP_NETAUTH:  return &CSTR("Network Authentication Required");

	}

	if (http_is_info(code))  return &CSTR("Unknown informational response");
	if (http_is_ok(code))    return &CSTR("Unknown success reponse");
	if (http_is_redir(code)) return &CSTR("Unknown redirection response");
	if (http_is_error(code)) return &CSTR("Unknown client error");
	if (http_is_fail(code))  return &CSTR("Unknown server error");

	return &CSTR("Unknown status code");

}


