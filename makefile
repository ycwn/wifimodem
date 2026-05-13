

.PHONY: all clean purge verbose build config erase upload monitor docs codecs


PFLAGS :=
STYLE  := $(shell cut -d "#" -f 1 < lazerwave.kcstyle | tr -d ' ')
BAUD   := 115200
PORT   := /dev/ttyUSB*



all: docs codecs build upload 

clean: purge

purge:
	rm -rf build/

verbose:
	$(eval PFLAGS=$(PFLAGS) -v)

build:
	pio run $(PFLAGS)

config:
	MENUCONFIG_STYLE="$(STYLE)" pio run -t menuconfig $(PFLAGS)

erase:
	pio run -t erase $(PFLAGS)

upload:
	pio run -t nobuild -t upload $(PFLAGS)

monitor:
	picocom -b $(BAUD) --omap crcrlf,delbs $(PORT)

docs:
	pio run -t gendocs $(PFLAGS)

codecs:
	pio run -t gencodecs $(PFLAGS)


