
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

#include <sys/socket.h>

#include <esp_wifi.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include "core/types.h"
#include "core/debug.h"
#include "core/str.h"
#include "core/io.h"
#include "core/list.h"
#include "core/util.h"
#include "core/logger.h"

#include "util/net.h"

#include "phy/wireless.h"



enum {
	WIFI_CONNECTED_BIT = 1 << 0,
	WIFI_FAIL_BIT      = 1 << 1
};

enum {
	SSID_LEN = 32,
	PASS_LEN = 64,
	NAME_LEN = 32
};

enum {
	MAX_RETRIES  = 10,
	MAX_STATIONS = 32
};


typedef struct {

	bool up;

	i8 chan;
	u8 auth;

	char ssid[SSID_LEN + 1];
	char pass[PASS_LEN + 1];
	char name[NAME_LEN + 1];

} wifi_config;


static void raise_event(int ev);
static void event_netif(void *arg, esp_event_base_t evbase, long evid, void *evdata);
static void event_wifi( void *arg, esp_event_base_t evbase, long evid, void *evdata);
static bool phy_disable();
static bool phy_enable();
static bool phy_connect();


static const str wifi_auth_list[] = {
	CSTR("OPEN"),
	CSTR("WEP"),
	CSTR("WPA"),  CSTR("WPA12"),
	CSTR("WPA2"), CSTR("WPA23"),
	CSTR("WPA3"),
	CSTR("")
};

static void        *g_evgroup;
static int          g_retries;
static wifi_config  g_config;
static esp_netif_t *g_netif;
static list         g_observer_list;



int wifi_create()
{

	g_evgroup = xEventGroupCreate();
	g_retries = 0;
	g_netif   = NULL;

	g_config = (wifi_config) {
		.up   = false,
		.chan = -1,
		.auth = 0,
		.ssid = "",
		.pass = "",
		.name = "wifimodem"
	};

	list_init(&g_observer_list, NULL);


	esp_netif_init();

	esp_event_loop_create_default();
	esp_event_handler_instance_register(IP_EVENT,   ESP_EVENT_ANY_ID, &event_netif, NULL, NULL);
	esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_wifi,  NULL, NULL);

	esp_wifi_init(&(wifi_init_config_t)WIFI_INIT_CONFIG_DEFAULT());
	esp_wifi_set_ps(WIFI_PS_NONE);
	esp_wifi_set_mode(WIFI_MODE_NULL);
	esp_wifi_set_storage(WIFI_STORAGE_RAM);

	//FIXME: Check for errors

	g_netif = esp_netif_create_default_wifi_sta();

	return 0;

}



void wifi_destroy()
{

	phy_disable();

	esp_netif_destroy_default_wifi(g_netif);

	g_netif = NULL;

	esp_wifi_deinit();

	esp_event_loop_delete_default();
	esp_netif_deinit();

	vEventGroupDelete(g_evgroup);

}



int wifi_get(uint key, str *val, char *buf, uint len)
{

	if (g_netif == NULL || buf == NULL)
		return -1;

	str  value  = CSTR("");
	bool cached = !(key & WIFI_LIVE);
	int  rc     = ESP_OK;

	switch (key & ~WIFI_LIVE) {

		case WIFI_UP:
		case WIFI_DOWN:
			value = g_config.up? CSTR("up"): CSTR("down");
			break;

		case WIFI_SSID: {
			wifi_ap_record_t ap;
			if (cached)
				value = CSTRZ(g_config.ssid);
			else if ((rc = esp_wifi_sta_get_ap_info(&ap)) == ESP_OK) {
				if (len < sizeof(ap.ssid))
					return -1;
				int l = strnlen((char*)ap.ssid, sizeof(ap.ssid));
				value = CSTRN(memcpy(buf, ap.ssid, l), l);
			}
			break;
		}

		case WIFI_PASS:
			value = CSTRZ(g_config.pass);
			break;

		case WIFI_HOSTNAME: {
			const char *host;
			if (cached)
				value = CSTRZ(g_config.name);
			else if ((rc = esp_netif_get_hostname(g_netif, &host)) == ESP_OK) {
				if (host == NULL)
					return -1;
				value = CSTRZ(host);
			}
			break;
		}

		case WIFI_CHANNEL: {
			u8                 pri;
			wifi_second_chan_t sec;
			if (cached)
				value = CSTRN(buf, snprintf(buf, len, "%d", g_config.chan));
			else if ((rc = esp_wifi_get_channel(&pri, &sec)) == ESP_OK)
				value = CSTRN(buf, snprintf(buf, len, "%d", pri));
			break;
		}

		case WIFI_MAC: {
			u8 mac[6];
			if ((rc = esp_netif_get_mac(g_netif, mac)) != ESP_OK)
				value = mac_format(mac, buf, len);
			break;
		}

		case WIFI_COUNTRY: {
			if (len < 2)
				return -1;
			if ((rc = esp_wifi_get_country_code(buf)) != ESP_OK)
				value = CSTRN(buf, 2);
			break;
		}

		case WIFI_IPV4_ADDR: {
			esp_netif_ip_info_t ip;
			if ((rc = esp_netif_get_ip_info(g_netif, &ip)) == ESP_OK)
				value = ipv4_format(ip.ip.addr, buf, len);
			break;
		}

		case WIFI_IPV4_MASK: {
			esp_netif_ip_info_t ip;
			if ((rc = esp_netif_get_ip_info(g_netif, &ip)) == ESP_OK)
				value = ipv4_format(ip.netmask.addr, buf, len);
			break;
		}

		case WIFI_IPV4_GWAY: {
			esp_netif_ip_info_t ip;
			if ((rc = esp_netif_get_ip_info(g_netif, &ip)) == ESP_OK)
				value = ipv4_format(ip.gw.addr, buf, len);
			break;
		}

		case WIFI_IPV4_DNS0: {
			esp_netif_dns_info_t  dns;
			if ((rc = esp_netif_get_dns_info(g_netif, ESP_NETIF_DNS_MAIN, &dns)) == ESP_OK)
				value = ipv4_format(dns.ip.u_addr.ip4.addr, buf, len);
			break;
		}

		case WIFI_IPV4_DNS1: {
			esp_netif_dns_info_t  dns;
			if ((rc = esp_netif_get_dns_info(g_netif, ESP_NETIF_DNS_BACKUP, &dns)) == ESP_OK)
				value = ipv4_format(dns.ip.u_addr.ip4.addr, buf, len);
			break;
		}

	}

	if (val != NULL)
		*val = value;

	return rc;

}



int wifi_set(uint key, const str *val)
{

	if (val == NULL)
		return -1;

	bool ok = false;

	switch (key) {

		case WIFI_UP: {
			if (phy_disable())
				ok = g_config.ssid[0]?
					phy_connect():
					phy_enable();
			break;
		}

		case WIFI_DOWN:     ok = phy_disable(); break;
		case WIFI_SSID:     ok = str_zcpy(val, g_config.ssid, SSID_LEN) != NULL; break;
		case WIFI_PASS:     ok = str_zcpy(val, g_config.pass, PASS_LEN) != NULL; break;
		case WIFI_HOSTNAME: ok = str_zcpy(val, g_config.name, NAME_LEN) != NULL; break;
		case WIFI_CHANNEL:  ok = str_readint(&STR(*val), &g_config.chan); break;

		case WIFI_AUTHMODE: {

			int auth = str_matchiv(&STR(*val), wifi_auth_list);

			if (ok = (auth >= 0))
				g_config.auth = auth;
			break;

		}

	}

	return ok? ESP_OK: ESP_FAIL;

}



int wifi_scan(wifi_hotspot **hotspots, u8 *buf, uint len)
{

	wifi_scan_config_t scan_config = {};
	int                rc          = 0;

	scan_config.show_hidden = true;

	if (!g_config.up && !phy_enable())
		goto failed;

	raise_event(WIFI_EV_BEGIN);

	if ((rc = esp_wifi_scan_start(&scan_config, true)) != ESP_OK)
		goto failed;

	int              num = 0;
	wifi_ap_record_t ap  = {};

	wifi_hotspot *first = NULL;
	wifi_hotspot *curr  = NULL;

	while ((rc = esp_wifi_scan_get_ap_record(&ap)) == ESP_OK) {

		// Make sure the buffer is aligned
		while ((len > 0) && ((uintptr_t)buf & (alignof(wifi_hotspot) - 1))) {

			buf++;
			len--;

		}

		// Copy basic info
		if (len < sizeof(wifi_hotspot))
			break;

		wifi_hotspot *wh = (wifi_hotspot*)buf;

		buf += sizeof(wifi_hotspot);
		len -= sizeof(wifi_hotspot);

		wh->rssi = ap.rssi;
		wh->chan = ap.primary;
		wh->auth = ap.authmode;
		wh->band = (ap.primary <= 14)? 0x24: 0x50;
		wh->next = NULL;

		// Allocate and copy SSID
		int ssid_len = strnlen((const char*)ap.ssid, sizeof(ap.ssid));

		if (len < ssid_len)
			break;

		wh->ssid.ptr = memcpy(buf, ap.ssid, ssid_len);
		wh->ssid.len = ssid_len;

		buf += ssid_len;
		len -= ssid_len;

		// Allocate and format MAC address
		wh->mac = mac_format(ap.bssid, (char*)buf, len);

		buf += wh->mac.len;
		len -= wh->mac.len;

		if (first == NULL)
			first = wh;

		if (curr != NULL)
			curr->next = wh;

		curr = wh;
		num++;

	}

	esp_wifi_clear_ap_list();

	if (rc != ESP_FAIL)
		goto failed;

	if (hotspots != NULL)
		*hotspots = first;

	raise_event(WIFI_EV_DONE);
	return num;

failed:
	raise_event(WIFI_EV_ERROR);
	return -rc;

}



void wifi_observer_init(wifi_observer *wo, wifi_observer_proc *p, void *d)
{

	list_init(&wo->node, wo);

	wo->proc = p;
	wo->data = d;

}



void wifi_observer_add(wifi_observer *wo)
{

	list_append(&g_observer_list, &wo->node);

}



void wifi_observer_del(wifi_observer *wo)
{

	list_remove(&wo->node);

}



bool phy_disable()
{

	if (g_config.up) {

		esp_wifi_disconnect();

		esp_wifi_stop();
		esp_wifi_set_mode(WIFI_MODE_NULL);

	}

	g_config.up = false;

	raise_event(WIFI_EV_DOWN);

	return true;

}



bool phy_enable()
{

	wifi_config_t cfg = {};
	int           rc  = ESP_OK;

	esp_wifi_set_mode(WIFI_MODE_STA);
	esp_wifi_set_config(WIFI_IF_STA, &cfg);

	if (*g_config.name != 0)
		esp_netif_set_hostname(g_netif, g_config.name);

	raise_event(WIFI_EV_BEGIN);

	if ((rc = esp_wifi_start()) != ESP_OK) {

		//TODO: Log error
		raise_event(WIFI_EV_ERROR);
		return false;

	}

	g_config.up = true;

	raise_event(WIFI_EV_DONE);
	raise_event(WIFI_EV_UP);

	return true;

}



bool phy_connect()
{

	if (!g_config.ssid[0])
		return ESP_ERR_INVALID_ARG;

	wifi_config_t cfg = {};
	int           rc  = ESP_OK;

	cfg.sta.channel            = g_config.chan;
	cfg.sta.threshold.authmode = WIFI_AUTH_OPEN;

	strcpy((char*)cfg.sta.ssid, g_config.ssid);

	if (*g_config.pass != 0) {

		//FIXME: Actually use the authentication method that was requested
		cfg.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
		strncpy((char*)cfg.sta.password, g_config.pass, sizeof(cfg.sta.password));

	}

	raise_event(WIFI_EV_BEGIN);

	esp_wifi_set_mode(WIFI_MODE_STA);
	esp_wifi_set_config(WIFI_IF_STA, &cfg);

	if (*g_config.name != 0)
		esp_netif_set_hostname(g_netif, g_config.name);

	g_config.up = true;

	xEventGroupClearBits(g_evgroup, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);

	if ((rc = esp_wifi_start()) != ESP_OK) {

		//TODO: Log error
		raise_event(WIFI_EV_ERROR);
		return false;

	}

	if ((xEventGroupWaitBits(g_evgroup, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, false, false, portMAX_DELAY) & WIFI_CONNECTED_BIT) == 0) {

		//TODO: Log error
		raise_event(WIFI_EV_ERROR);
		return false;

	}

	raise_event(WIFI_EV_DONE);
	raise_event(WIFI_EV_CONNECT);

	return true;

}



void raise_event(int ev)
{

	for (auto it=list_begin(&g_observer_list); it != list_end(&g_observer_list); it=it->next) {

		auto wo = LIST_PTR(wifi_observer, it);

		wo->proc(ev, NULL, wo->data);

	}

}



void event_netif(void *arg, esp_event_base_t evbase, long evid, void *evdata)
{

	switch (evid) {

		case IP_EVENT_STA_GOT_IP: {

			const ip_event_got_ip_t *event = evdata;

			LOGI("wireless: IPv4 update\n");
			LOGI("wireless: address: " IPSTR "\n", IP2STR(&event->ip_info.ip));
			LOGI("wireless: netmask: " IPSTR "\n", IP2STR(&event->ip_info.netmask));
			LOGI("wireless: gateway: " IPSTR "\n", IP2STR(&event->ip_info.gw));

			xEventGroupSetBits(g_evgroup, WIFI_CONNECTED_BIT);
			break;

		}

	}

}



void event_wifi(void *arg, esp_event_base_t evbase, long evid, void *evdata)
{

	switch (evid) {

		case WIFI_EVENT_STA_START: {

			LOGI("wireless: Starting...\n");

			g_retries = MAX_RETRIES;
			esp_wifi_connect();
			break;

		}


		case WIFI_EVENT_STA_DISCONNECTED: {

			if (g_retries-- < 0) {

				LOGI("wireless: Join failed: Maximum retries reached\n");
				xEventGroupSetBits(g_evgroup, WIFI_FAIL_BIT);

			} else {

				LOGI("wireless: Join failed, retrying (%d)\n", g_retries);
				esp_wifi_connect();

			}

			break;

		}


		case WIFI_EVENT_AP_STACONNECTED: { // Somebody connected to our AP

			const wifi_event_ap_staconnected_t *event = evdata;

			LOGI("wireless: Client connected: " MACADDR_STR_F "\n", MACADDR_ARG_F(event->mac));
			break;

		}


		case WIFI_EVENT_AP_STADISCONNECTED: { // Somebody disconnwcted

			const wifi_event_ap_stadisconnected_t *event = evdata;

			LOGI("wireless: Client disconnected: " MACADDR_STR_F "\n", MACADDR_ARG_F(event->mac));
			break;

		}

	}

}


