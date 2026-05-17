
#ifndef PHY_LEDCONTROL_H
#define PHY_LEDCONTROL_H


enum {

	LEDCONTROL_NUM = 4

};


typedef struct {

	struct {

		void *timer;
		uint  count;

		int gpio[LEDCONTROL_NUM];
		int link[LEDCONTROL_NUM];

	} p;

	u32 pattern[LEDCONTROL_NUM];

} ledcontrol;


int  ledcontrol_init(ledcontrol *l, uint res, uint hz);
void ledcontrol_del( ledcontrol *l);

int  ledcontrol_start(ledcontrol *l);
void ledcontrol_stop( ledcontrol *l);

void ledcontrol_attach(ledcontrol *l, int ch, int gpio, int link);
void ledcontrol_detach(ledcontrol *l, int ch);
void ledcontrol_shadow(ledcontrol *l, int ch, bool shadow);


#endif


