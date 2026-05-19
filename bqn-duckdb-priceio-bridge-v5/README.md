# bqn-duckdb-priceio-bridge

CBQN `•FFI` bridge for reading local OHLCV Parquet files through DuckDB.

The bridge is split into two layers:

```text
BQN price API:      bqn/priceio.bqn
BQN FFI bindings:   bqn/duckdb_bridge.bqn
C ABI bridge:       src/bqn_duckdb_bridge.c
DuckDB C API:       libduckdb.so
Parquet files:      read_parquet(...)
```

## Expected Parquet schema

The default reader expects these column names:

```text
timestamp, open, high, low, close, volume
```

The C layer projects and casts them to:

```text
ts_ms, open, high, low, close, volume
```

`ts_ms` is returned as a `double`, because CBQN numbers are floating-point. Epoch milliseconds are exactly representable while they remain under `2^53`, which is fine for normal market timestamps.

## Build on Ubuntu 24

```bash
sudo apt install -y build-essential curl unzip
make fetch-duckdb
make
```

This downloads `libduckdb-linux-amd64.zip` into `.deps/duckdb` and builds:

```text
build/libbqn_duckdb_bridge.so
```

## Usage from CBQN

```bqn
p ← •Import "bqn/priceio.bqn"

# Auto for common OHLCV files. The attached Binance file stores
# timestamp as BIGINT epoch-ms, so this path succeeds directly.
d ← p.ReadOHLCV "data/BTCUSDT-1h.parquet"

# Explicit: timestamp is already epoch milliseconds.
d ← p.ReadOHLCVMs "data/BTCUSDT-1Mo.parquet"

# Explicit: use the timestamp_ns/date column instead.
d ← p.ReadOHLCVDate "data/BTCUSDT-1Mo.parquet"

# Explicit: timestamp is DuckDB TIMESTAMP-like.
d ← p.ReadOHLCVTimestamp "data/BTCUSDT-1h.parquet"

# Explicit: timestamp is epoch seconds.
d ← p.ReadOHLCVSeconds "data/BTCUSDT-1h.parquet"

# inclusive range filter, using epoch milliseconds
r ← p.ReadOHLCVMsRange "data/BTCUSDT-1h.parquet"‿1704067200000‿1706745600000

5 ↑ d.table
```

The returned namespace has:

```bqn
d.names
d.ts_ms
 d.open
 d.high
 d.low
 d.close
 d.volume
 d.columns  # list of six vectors
 d.table    # n×6 matrix
```

## Smoke test

```bash
BQN examples/smoke.bqn data/BTCUSDT-1h.parquet
```

`•args` in CBQN already contains only user-supplied command-line arguments, so the smoke script uses `args ← •args` rather than dropping the first element.

## Notes

- The C table pointer is opaque to BQN. BQN does not inspect the C struct layout.
- BQN reads borrowed C column pointers before freeing the table.
- The C bridge rejects NULLs in required OHLCV fields.
- `from_ms > to_ms` means “no range filter”.
- `ReadOHLCV` tries epoch-ms first, then `date` as TIMESTAMP-like, then `timestamp` as TIMESTAMP-like. Use `ReadOHLCVMs` directly for files whose `timestamp` is pandas/Arrow `int64`.
- The attached Binance schema is `timestamp BIGINT`, OHLCV `DOUBLE`, and `date TIMESTAMP_NS`, so both `ReadOHLCVMs` and `ReadOHLCVDate` are valid.
- File paths are SQL-quoted defensively.
- `•FFI` bindings are assigned to lowercase exported names because `•FFI` returns a function value in subject role. Use uppercase spelling, such as `bridge.ReadRaw`, when calling them.
