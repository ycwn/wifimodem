
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <esp_attr.h>
#include <driver/gpio.h>
#include <driver/gptimer.h>
#include <rom/gpio.h>
#include <soc/gpio_sig_map.h>

#include "core/types.h"
#include "core/debug.h"
#include "core/str.h"
#include "core/logger.h"

#include "phy/ledcontrol.h"


static bool callback(struct gptimer_t *timer, const gptimer_alarm_event_data_t *event, ledcontrol *l);



int ledcontrol_init(ledcontrol *l, uint res, uint hz)
{

	l->p.timer = NULL;
	l->p.count = 0;

	for (int i=0; i < LEDCONTROL_NUM; i++) {

		l->p.gpio[i] = -1;
		l->p.link[i] = -1;

		l->pattern[i] = 0;

	}

	gptimer_config_t tmrcfg = {
		.clk_src       = GPTIMER_CLK_SRC_DEFAULT,
		.direction     = GPTIMER_COUNT_UP,
		.resolution_hz = res,
		.intr_priority = 0,
		.flags = {
			.intr_shared         = 0,
			.allow_pd            = 0,
			.backup_before_sleep = 0
		}
	};

	int rv = gptimer_new_timer(&tmrcfg, (gptimer_handle_t*)&l->p.timer);

	if (rv != ESP_OK) {

		LOGE("leds: Failed to create timer: %d\n", rv);
		return rv;

	}

	gptimer_event_callbacks_t callbacks = {
		.on_alarm = (gptimer_alarm_cb_t)&callback
	};

	gptimer_register_event_callbacks(l->p.timer, &callbacks, l);


	gptimer_alarm_config_t almcfg = {
		.alarm_count  = res / hz,
		.reload_count = 0,
		.flags = {
			.auto_reload_on_alarm = true
		}
	};

	gptimer_set_alarm_action(l->p.timer, &almcfg);

	return rv;

}



void ledcontrol_del(ledcontrol *l)
{

	for (int i=0; i < LEDCONTROL_NUM; i++)
		ledcontrol_detach(l, i);

	ledcontrol_stop(l);

	if (l->p.timer != NULL)
		gptimer_del_timer(l->p.timer);

}



int ledcontrol_start(ledcontrol *l)
{

	if (l->p.timer == NULL)
		return ESP_FAIL;

	int rc = gptimer_enable(l->p.timer);

	if (rc != ESP_OK)
		return rc;

	return gptimer_start(l->p.timer);

}



void ledcontrol_stop(ledcontrol *l)
{

	if (l->p.timer == NULL)
		return;

	gptimer_stop(   l->p.timer);
	gptimer_disable(l->p.timer);

}



void ledcontrol_attach(ledcontrol *l, int ch, int gpio, int link)
{

	ledcontrol_detach(l, ch);

	if ((ch < 0) || (ch >= LEDCONTROL_NUM) || (gpio < 0))
		return;

	gpio_reset_pin( gpio);
	gpio_matrix_out(gpio, SIG_GPIO_OUT_IDX, false, false);

	gpio_set_direction(gpio, GPIO_MODE_OUTPUT);
	gpio_set_level(    gpio, 0);

	l->p.gpio[ch] = gpio;
	l->p.link[ch] = link;

}



void ledcontrol_detach(ledcontrol *l, int ch)
{

	if ((ch < 0) || (ch >= LEDCONTROL_NUM) || (l->p.gpio[ch] < 0))
		return;

	gpio_reset_pin( l->p.gpio[ch]);
	gpio_matrix_out(l->p.gpio[ch], SIG_GPIO_OUT_IDX, false, false);

	l->p.gpio[ch] = -1;
	l->p.link[ch] = -1;

}



void ledcontrol_shadow(ledcontrol *l, int ch, bool shadow)
{

	if ((ch < 0) || (ch >= LEDCONTROL_NUM) || (l->p.gpio[ch] < 0) || (l->p.link[ch] < 0))
		return;

	int pin = shadow?
		l->p.link[ch]:
		SIG_GPIO_OUT_IDX;

	gpio_matrix_out(l->p.gpio[ch], pin, true, false);

}



bool IRAM_ATTR callback(struct gptimer_t *timer, const gptimer_alarm_event_data_t *event, ledcontrol *l)
{

	uint mask = 1 << l->p.count;

	l->p.count++;
	l->p.count &= sizeof(l->pattern[0]) * 8 - 1;

	for (int i=0; i < LEDCONTROL_NUM; i++)
		if (l->p.gpio[i] >= 0)
			gpio_set_level(l->p.gpio[i], (l->pattern[i] & mask) != 0);

	return false;

}


