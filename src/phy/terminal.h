
#ifndef PHY_TERMINAL_H
#define PHY_TERMINAL_H


typedef struct {

	io_rdwr io;

	bool add_cr;    ///< Send CRLF on LF (LF -> CRLF on send)
	bool ign_cr;    ///< Ignore CR on read (CRLF to LF on recv)
	bool crlf;      ///< Convert LF to CR on send; CR to LF on recv
	bool echo;      ///< Echo incomming characters back

	u8 chr_bs;      ///< Backspace character
	u8 chr_cr;      ///< Carriage return
	u8 chr_nl;      ///< Newline character

} terminal;



void terminal_init( terminal *t);
int  terminal_read( terminal *t, u8 *buf, uint len);
int  terminal_write(terminal *t, const u8 *buf, uint len);

int terminal_getc( terminal *t);
int terminal_gets( terminal *t, char *buf, uint len);
str terminal_input(terminal *t, char *buf, uint len, const str *prompt, int hide);

void terminal_putc(   terminal *t, int c);
void terminal_puts(   terminal *t, const char *s);
void terminal_printfv(terminal *t, const char *fmt, va_list argv);
void terminal_printf( terminal *t, const char *fmt, ...);

static inline void terminal_set_msdos(terminal *t) { t->add_cr = true;  t->ign_cr = true;  t->crlf = false; }
static inline void terminal_set_unix( terminal *t) { t->add_cr = false; t->ign_cr = true;  t->crlf = false; }
static inline void terminal_set_mac(  terminal *t) { t->add_cr = false; t->ign_cr = false; t->crlf = true;  }

static inline bool terminal_is_msdos(const terminal *t) { return  t->add_cr &&  t->ign_cr && !t->crlf; }
static inline bool terminal_is_unix( const terminal *t) { return !t->add_cr &&  t->ign_cr && !t->crlf; }
static inline bool terminal_is_mac(  const terminal *t) { return !t->add_cr && !t->ign_cr &&  t->crlf; }


#define TERMINAL_IO_INIT(p)  IO_INIT((p),  (io_rdfn*)&terminal_read, (io_wrfn*)&terminal_write)
#define TERMINAL_IO(io, p)   io_mkrw((io), (io_rdfn*)&terminal_read, (io_wrfn*)&terminal_write, (p))


#endif


