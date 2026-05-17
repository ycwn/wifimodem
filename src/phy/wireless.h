
#ifndef PHY_WIRELESS_H
#define PHY_WIRELESS_H


enum {
	WIFI_KEY_NONE,
	WIFI_KEY_WEP,
	WIFI_KEY_WPA,  WIFI_KEY_WPA12,
	WIFI_KEY_WPA2, WIFI_KEY_WPA23,
	WIFI_KEY_WPA3,
	WIFI_KEY_MAX
};

enum {

	WIFI_UP,
	WIFI_DOWN,

	WIFI_SSID,    WIFI_PASS,     WIFI_AUTHMODE,
	WIFI_CHANNEL, WIFI_HOSTNAME,

	WIFI_MAC,
	WIFI_COUNTRY,

	WIFI_IPV4_ADDR, WIFI_IPV4_MASK, WIFI_IPV4_GWAY,
	WIFI_IPV4_DNS0, WIFI_IPV4_DNS1,

	WIFI_CONF = 0,
	WIFI_LIVE = 1024,

};

enum {
	WIFI_EV_BEGIN,   WIFI_EV_DONE, WIFI_EV_ERROR,
	WIFI_EV_UP,      WIFI_EV_DOWN,
	WIFI_EV_CONNECT
};


typedef void (wifi_observer_proc)(int event, void *extra, void *data);


typedef struct wifi_hotspot {

	int  rssi;

	uint chan;
	uint auth;
	uint band;

	str ssid;
	str mac;

	struct wifi_hotspot *next;

} wifi_hotspot;


typedef struct {

	list node;

	wifi_observer_proc *proc;
	void               *data;

} wifi_observer;



int  wifi_create();
void wifi_destroy();

int wifi_get(uint key, str       *val, char *buf, uint len);
int wifi_set(uint key, const str *val);
int wifi_scan(wifi_hotspot **hotspots, u8 *buf, uint len);

void wifi_observer_init(wifi_observer *wo, wifi_observer_proc *p, void *d);
void wifi_observer_add(wifi_observer *wo);
void wifi_observer_del(wifi_observer *wo);


#endif


