
#ifndef PHY_SERIALPORT_H
#define PHY_SERIALPORT_H


enum {

	SERIAL_RX_BUFFER = 1024,
	SERIAL_TX_BUFFER = 1024,

	SERIAL_EVENT_QUEUE = 16,

	SERIAL_BAUDRATE_MIN = 100,
	SERIAL_BAUDRATE_MAX = 125000,

	SERIAL_AUTOBAUD_BUFFER     = 64,
	SERIAL_AUTOBAUD_SAMPLES    = 8,
	SERIAL_AUTOBAUD_RESOLUTION = 1000000

};


typedef struct serialport {

	struct {

		int port;

		void *queue;
		void *rmt;

		u32 buffer[ SERIAL_AUTOBAUD_BUFFER];
		u32 samples[SERIAL_AUTOBAUD_SAMPLES];

	} p;

	struct {

		float timing;
		float error;
		uint  realrate;
		uint  baudrate;

		bool running;
		bool ready;

	} autobaud;

	int baudrate;
	int databits;
	int stopbits;
	int parity;

} serialport;


bool serialport_init(serialport *p, int port, int rx, int tx, int cts, int rts, int abd);
void serialport_del( serialport *p);

int serialport_read( serialport *p, u8       *buf, uint len);
int serialport_write(serialport *p, const u8 *buf, uint len);

str  serialport_get_config(serialport *p, char *buf, uint len);
bool serialport_set_config(serialport *p, const str *cfg);
bool serialport_configure( serialport *p);
bool serialport_autobaud(  serialport *p, bool enabled);


#define SERIALPORT_IO_INIT(p)  IO_INIT((p),  (io_rdfn*)&serialport_read, (io_wrfn*)&serialport_write)
#define SERIALPORT_IO(io, p)   io_mkrw((io), (io_rdfn*)&serialport_read, (io_wrfn*)&serialport_write, (p))


#endif


