# duckdb_bridge.bqn — low-level CBQN •FFI bridge to libbqn_duckdb_bridge.so
#
# This file is intentionally thin: it binds C functions and exports them.
# The higher-level, BQN-shaped interface is in priceio.bqn.

libPath ← "../build/libbqn_duckdb_bridge.so"

# Opaque table pointer API.
readRaw   ⇐ libPath •FFI "*"    ‿"bqn_price_read_ohlcv"     ‿"*u8:c8"‿"i64"‿"i64"‿"i32"‿"i32"
freeRaw   ⇐ libPath •FFI ""     ‿"bqn_price_table_free"     ‿">*"
statusRaw ⇐ libPath •FFI "i64"  ‿"bqn_price_table_status"   ‿">*"
rowsRaw   ⇐ libPath •FFI "i64"  ‿"bqn_price_table_rows"     ‿">*"
colsRaw   ⇐ libPath •FFI "i64"  ‿"bqn_price_table_cols"     ‿">*"
errorRaw  ⇐ libPath •FFI "*u8"  ‿"bqn_price_table_error"    ‿">*"
colPtrRaw ⇐ libPath •FFI "*f64" ‿"bqn_price_table_col_ptr"  ‿"*"‿"i32"
getRaw    ⇐ libPath •FFI "f64"  ‿"bqn_price_table_get"      ‿"*"‿"i32"‿"i64"
versionRaw← libPath •FFI "*u8"  ‿"bqn_price_duckdb_version"

# C string pointer -> BQN string.
# This follows the CBQN FFI documentation pattern:
#   strlen gives length from right argument;
#   bqn_makeC8Vec takes length from left argument and pointer from right.
strlen ← @ •FFI "u64"‿"strlen"‿">*u8"
mkStr  ← @ •FFI "a"‿"bqn_makeC8Vec"‿">𝕨u64"‿">*u8"
StrFromPtr ⇐ Strlen⊸MkStr

DuckDBVersion ⇐ {𝕊:
  StrFromPtr VersionRaw ⟨⟩
}
