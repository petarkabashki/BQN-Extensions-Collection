# Run from repository root after `make`:
#   BQN examples/smoke.bqn data/BTCUSDT-1h.parquet

p ← •Import "../bqn/priceio.bqn"
args ← •args
"Usage: BQN examples/smoke.bqn path/to/data.parquet" ! 0<≠args

•Show "DuckDB "∾p.DuckDBVersion @

d ← p.ReadOHLCV ⊑args
•Show d.names
•Show ≢ d.table
•Show 5 ↑ d.table
