DUCKDB_VERSION ?= 1.5.2
DUCKDB_PREFIX  ?= $(CURDIR)/.deps/duckdb
CC             ?= gcc
CFLAGS         ?= -O3 -fPIC -Wall -Wextra -Wpedantic -std=c11
CPPFLAGS       += -Iinclude -I$(DUCKDB_PREFIX)
LDFLAGS        += -shared
LDLIBS         += -L$(DUCKDB_PREFIX) -lduckdb -Wl,-rpath,$(DUCKDB_PREFIX)

TARGET := build/libbqn_duckdb_bridge.so
SRC    := src/bqn_duckdb_bridge.c
HDR    := include/bqn_duckdb_bridge.h

.PHONY: all clean fetch-duckdb print-duckdb-url

all: $(TARGET)

$(TARGET): $(SRC) $(HDR)
	mkdir -p build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $(SRC) $(LDLIBS)

fetch-duckdb:
	mkdir -p $(DUCKDB_PREFIX)
	curl -L -o /tmp/libduckdb-$(DUCKDB_VERSION)-linux-amd64.zip \
	  https://github.com/duckdb/duckdb/releases/download/v$(DUCKDB_VERSION)/libduckdb-linux-amd64.zip
	unzip -o /tmp/libduckdb-$(DUCKDB_VERSION)-linux-amd64.zip -d $(DUCKDB_PREFIX)

print-duckdb-url:
	@echo https://github.com/duckdb/duckdb/releases/download/v$(DUCKDB_VERSION)/libduckdb-linux-amd64.zip

clean:
	rm -rf build
