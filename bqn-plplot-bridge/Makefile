CC ?= gcc
CFLAGS ?= -O2 -fPIC -Wall -Wextra
PREFIX ?= /usr/local

PLPLOT_CFLAGS := $(shell pkg-config --cflags plplot 2>/dev/null || pkg-config --cflags plplotd 2>/dev/null || echo -I/usr/include/plplot)
PLPLOT_LIBS := $(shell pkg-config --libs plplot 2>/dev/null || pkg-config --libs plplotd 2>/dev/null || echo -lplplot)

.PHONY: all clean install

all: build/libbqnplplot.so

build/libbqnplplot.so: src/bqn_plplot.c include/bqn_plplot.h | build
	$(CC) $(CFLAGS) $(PLPLOT_CFLAGS) -shared -o $@ src/bqn_plplot.c $(PLPLOT_LIBS)

build:
	mkdir -p build

install: build/libbqnplplot.so
	install -d $(PREFIX)/lib $(PREFIX)/include
	install -m 0755 build/libbqnplplot.so $(PREFIX)/lib/libbqnplplot.so
	install -m 0644 include/bqn_plplot.h $(PREFIX)/include/bqn_plplot.h

clean:
	rm -rf build
