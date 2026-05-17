
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include <driver/uart.h>
#include <driver/rmt_rx.h>

#include "core/types.h"
#include "core/debug.h"
#include "core/str.h"
#include "core/logger.h"

#include "phy/serialport.h"


static void autobaud_calculate(serialport *p);
static int  autobaud_activate( serialport *p);
static bool autobaud_callback(rmt_channel_handle_t chan, const rmt_rx_done_event_data_t *rx, serialport *p);


static const uint g_baudrates[]={
	110, 300, 600, 1200, 2400, 4800,9600, 19200, 38400, 57600, 115200, 0
};



bool serialport_init(serialport *p, int port, int rx, int tx, int cts, int rts, int abd)
{

	p->p.port = port;

	p->p.queue = NULL;
	p->p.rmt   = NULL;

	p->autobaud.timing   = 0.0f;
	p->autobaud.error    = 0.0f;
	p->autobaud.realrate = 0;
	p->autobaud.baudrate = 0;
	p->autobaud.running  = false;
	p->autobaud.ready    = false;

	// Default configuration is 115200-8n1-none
	p->baudrate = 115200;
	p->databits = 8;
	p->parity   = 0;
	p->stopbits = 1;

	uart_set_pin(p->p.port, tx, rx, rts, cts);

	uart_driver_install(p->p.port,
		SERIAL_RX_BUFFER, SERIAL_TX_BUFFER,
		SERIAL_EVENT_QUEUE, (QueueHandle_t*)&p->p.queue, 0);

	if (abd >= 0) {

		rmt_rx_channel_config_t rx_cfg = {
			.gpio_num          = abd,
			.clk_src           = RMT_CLK_SRC_DEFAULT,
			.resolution_hz     = SERIAL_AUTOBAUD_RESOLUTION,
			.mem_block_symbols = sizeof(p->p.buffer) / sizeof(rmt_symbol_word_t),
			.intr_priority     = 0,
			.flags             = {
				.invert_in    = false,
				.with_dma     = false,
				.io_loop_back = false,
				.allow_pd     = false
			}
		};

		int rv = rmt_new_rx_channel(&rx_cfg, (struct rmt_channel_t**)&p->p.rmt);

		if (rv != ESP_OK)
			LOGE("autobaud: Failed to initialize RMT: %d (%s)", rv, esp_err_to_name(rv));

		rmt_rx_event_callbacks_t cb = {
			.on_recv_done = (rmt_rx_done_callback_t)&autobaud_callback
		};

		rmt_rx_register_event_callbacks(p->p.rmt, &cb, p);

	}

	return serialport_configure(p);

}



void serialport_del(serialport *p)
{

	serialport_autobaud(p, false);

	uart_driver_delete(p->p.port);

	if (p->p.rmt != NULL)
		rmt_del_channel(p->p.rmt);

}



int serialport_read(serialport *p, u8 *buf, uint len)
{

	if (p->autobaud.ready)
		autobaud_calculate(p);

	return uart_read_bytes(p->p.port, buf, len, 0);

}



int serialport_write(serialport *p, const u8 *buf, uint len)
{

	return uart_write_bytes(p->p.port, buf, len);

}



str serialport_get_config(serialport *p, char *buf, uint len)
{

	const char *parity =
		(p->parity == 1)? "o":
		(p->parity == 2)? "e": "n";

	int num = snprintf(buf, len, 
		"%d,%d%s%d",
		p->baudrate,
		p->databits,
		parity,
		p->stopbits);

	if (num < 0)
		num = 0;

	return CSTRN(buf, num);

}



bool serialport_set_config(serialport *p, const str *config)
{

	static const str parity_table[]={
		CSTR("n"), CSTR("o"), CSTR("e"), CSTR("")
	};

	str tok = *config;

	uint baudrate  = 0;
	uint databits  = p->databits;
	int  parity    = p->parity;
	uint stopbits  = p->stopbits;

	str_ltrim(&tok);

	if (!str_readuint(&tok, &baudrate))
		return false;

	str_ltrim(&tok);

	if (str_accept(&tok, &CSTR(","))) {

		str_ltrim(&tok);

		if (!str_readuint(&tok, &databits))
			return false;

		parity = str_matchiv(&tok, parity_table);

		if (parity < 0)
			return false;

		if (!str_readuint(&tok, &stopbits))
			return false;

	}

	return serialport_configure(p);

}



bool serialport_configure(serialport *p)
{

	if (p->baudrate < SERIAL_BAUDRATE_MIN || p->baudrate > SERIAL_BAUDRATE_MAX) {

		LOGE("serialport: Baud rate %d out of range (%d, %d)\n",
			p->baudrate, SERIAL_BAUDRATE_MIN, SERIAL_BAUDRATE_MAX);

		return false;

	}

	uart_config_t config = {
		.baud_rate  = p->baudrate,
		.data_bits  = UART_DATA_8_BITS,
		.parity     = UART_PARITY_DISABLE,
		.stop_bits  = UART_STOP_BITS_1,
		.flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
		.source_clk = UART_SCLK_DEFAULT
	};

	switch (p->databits) {

		case 5: config.data_bits = UART_DATA_5_BITS; break;
		case 6: config.data_bits = UART_DATA_6_BITS; break;
		case 7: config.data_bits = UART_DATA_7_BITS; break;
		case 8: config.data_bits = UART_DATA_8_BITS; break;

		default:
			LOGE("serialport: Invalid data bits %d\n", p->databits);
			return false;

	}

	switch (p->parity) {

		case 0: config.parity = UART_PARITY_DISABLE; break;
		case 1: config.parity = UART_PARITY_ODD;     break;
		case 2: config.parity = UART_PARITY_EVEN;    break;

		default:
			LOGE("serialport: Invalid parity %d\n", p->parity);
			return false;

	}

	switch (p->stopbits) {

		case 1: config.stop_bits = UART_STOP_BITS_1; break;
		case 2: config.stop_bits = UART_STOP_BITS_2; break;

		default:
			LOGE("serialport: Invalid stop bits %d\n", p->stopbits);
			return false;

	}

	int rv = uart_param_config(p->p.port, &config);

	if (rv != ESP_OK) {

		LOGE("serialport: Failed to configure hardware (%d)\n", rv);
		return false;

	}

	return true;

}



bool serialport_autobaud(serialport *p, bool enabled)
{

	if (p->p.rmt == NULL)
		return false;

	if (p->autobaud.running == enabled)
		return true;

	p->autobaud.ready   = false;
	p->autobaud.running = false;

	if (enabled) {

		rmt_enable(p->p.rmt);

		int rc = autobaud_activate(p);

		if (rc != ESP_OK) {

			LOGE("serialport: Failed to start the RMT RX channel: %d\n", rc);
			return false;

		}

	} else
		rmt_disable(p->p.rmt);

	return true;

}



void autobaud_calculate(serialport *p)
{

	p->autobaud.ready = false;

	p->autobaud.timing   = 0.0f;
	p->autobaud.error    = 0.0f;
	p->autobaud.realrate = 0;
	p->autobaud.baudrate = 0;

	int C = 0, M = 65536;
	int S = 0, N = 0;

	rmt_symbol_word_t *symbols = (void*)p->p.samples;

	// Find the shortest 0 or 1 duration
	for (int n=0; n < SERIAL_AUTOBAUD_SAMPLES; n++) {

		int i = symbols[n].duration0;
		int j = symbols[n].duration1;

		if ((i > 0) && (M > i))
			M = i;

		if ((j > 0) && (M > j))
			M = j;

		if (i != 0 || j != 0)
			C++;

       	}

	// If we have too few transitions, then we can't rely on this byte
	if (C <= 2)
		return;

	// Find the total duration, and count the number of bits
	for (int n=0; n < SERIAL_AUTOBAUD_SAMPLES; n++) {

		int k = symbols[n].duration0 + symbols[n].duration1;

		S += k;
		N += floorf((float)k / (float)M + 0.5f);

	}

	if (N <= 0)
		return;

	// Find the closest standard baudrate to the measured one
	float D = (float)S / (float)N;
	int   R = (float)SERIAL_AUTOBAUD_RESOLUTION / D;
	int   B = 0;
	float E = 1073741824.0;

	for (int n=0; g_baudrates[n] != 0; n++) {

		float ee = 100.0f * fabs((float)R / (float)g_baudrates[n] - 1.0f);

		if (ee < E) {

			B = g_baudrates[n];
			E = ee;

		}

	}

	p->autobaud.timing   = D;
	p->autobaud.error    = E;
	p->autobaud.realrate = R;
	p->autobaud.baudrate = B;

	if ((B != p->baudrate) && (uart_set_baudrate(p->p.port, B) == ESP_OK))
		p->baudrate = B;

}



int autobaud_activate(serialport *p)
{

	memset(p->p.buffer, 0, sizeof(p->p.buffer));

	rmt_receive_config_t rxcfg = {
		.signal_range_min_ns = 500,
		.signal_range_max_ns = 10000000
	};

	int rc = rmt_receive(p->p.rmt, p->p.buffer, sizeof(p->p.buffer), &rxcfg);
	
	if (rc == ESP_OK)
		p->autobaud.running = true;

	return rc;

}



bool IRAM_ATTR autobaud_callback(rmt_channel_handle_t chan, const rmt_rx_done_event_data_t *rx, serialport *p)
{

	// TODO: Get actual buffer (received_symbols) and count (num->symbols) from event data
	// This only works because the buffer is cleared
	memcpy(p->p.samples, p->p.buffer, sizeof(p->p.samples));

	p->autobaud.ready   = true;
	p->autobaud.running = false;

	autobaud_activate(p);

	return false; // No task switch

}


