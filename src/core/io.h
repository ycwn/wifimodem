

#ifndef CORE_IO_H
#define CORE_IO_H


typedef int (io_rdfn)(void *self, u8       *buf, uint len);
typedef int (io_wrfn)(void *self, const u8 *buf, uint len);


typedef struct {

	io_rdfn *rd;
	io_wrfn *wr;

	void *self;

} io_rdwr;


#define IO_INIT(s, r, w) { .rd=(io_rdfn)(r), .wr=(io_wrfn)(w), .self=(s) }
#define IO_RD(s, r)      IO_INIT(s, (r), &io_guard)
#define IO_WR(s, r)      IO_INIT(s, &io_guard, (w))


static inline int io_guard(void*, const u8*, uint)   { return -1; }
static inline int io_rdnop(void*, u8*,       uint)   { return  0; }
static inline int io_wrnop(void*, const u8*, uint l) { return  l; }

static inline void io_zero(io_rdwr *io) {
	io->rd   = (io_rdfn*)&io_guard;
	io->wr   = (io_wrfn*)&io_guard;
	io->self = NULL;
}

static inline void io_mkrw(io_rdwr *io, io_rdfn *rd, io_wrfn *wr, void *self) {
	io->rd = rd; io->wr = wr; io->self = self;
}

static inline void io_mkrd(io_rdwr *io, io_rdfn *rd, io_wrfn *wr, void *self) {
	io->rd = rd; io->wr = (io_wrfn*)&io_guard; io->self = self;
}

static inline void io_mkwr(io_rdwr *io, io_rdfn *rd, io_wrfn *wr, void *self) {
	io->rd = (io_rdfn*)&io_guard; io->wr = wr; io->self = self;
}


static inline int io_read( const io_rdwr *io, u8       *buf, uint len) { return (*io->rd)(io->self, buf, len); }
static inline int io_write(const io_rdwr *io, const u8 *buf, uint len) { return (*io->wr)(io->self, buf, len); }

static inline int io_getc(const io_rdwr *io)         { u8 b; return (io_read(io, &b, sizeof(u8)) > 0)? b: -1; }
static inline int io_putc(const io_rdwr *io, int ch) { u8 b=ch; return io_write(io, &b, sizeof(u8)); }


#endif


