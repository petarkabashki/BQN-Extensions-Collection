# priceio.bqn — high-level BQN API for local Parquet OHLCV data via DuckDB.
#
# Default auto schema detection tries:
#   1. named: timestamp, open, high, low, close, volume
#   2. numeric: 0, 1, 2, 3, 4, 5 (timestamp_ms, open, high, low, close, volume)
#
# Returned namespace fields:
#   names    list of column names
#   ts_ms    vector
#   open     vector
#   high     vector
#   low      vector
#   close    vector
#   volume   vector
#   columns  list of vectors
#   table    n×6 matrix, column order = names

bridge ← •Import "duckdb_bridge.bqn"

Check ← {𝕊 tbl:
  s ← bridge.StatusRaw tbl
  {𝕤
    msg ← bridge.StrFromPtr bridge.ErrorRaw tbl
    bridge.FreeRaw tbl
    ! "bqn_price_read_ohlcv failed: "∾msg
  }⍟(s≠0) @
  tbl
}

ReadCol ← {𝕊 tbl‿col‿n:
  ptr ← bridge.ColPtrRaw ⟨tbl,col⟩
  ptr.Read¨ ↕n
}

BuildResult ← {𝕊 tbl:
  n ← bridge.RowsRaw tbl

  ts_ms  ← ReadCol tbl‿0‿n
  open   ← ReadCol tbl‿1‿n
  high   ← ReadCol tbl‿2‿n
  low    ← ReadCol tbl‿3‿n
  close  ← ReadCol tbl‿4‿n
  volume ← ReadCol tbl‿5‿n

  bridge.FreeRaw tbl

  cols ← ⟨ts_ms, open, high, low, close, volume⟩
  mat ← ⍉ > cols

  {
    names   ⇐ "ts_ms"‿"open"‿"high"‿"low"‿"close"‿"volume"
    ts_ms   ⇐ ts_ms
    open    ⇐ open
    high    ⇐ high
    low     ⇐ low
    close   ⇐ close
    volume  ⇐ volume
    columns ⇐ cols
    table   ⇐ mat
  }
}

# Lowest-level public reader.
# time_mode:
#   0 = timestamp is TIMESTAMP-like
#   1 = timestamp is epoch milliseconds
#   2 = timestamp is epoch seconds
#   3 = date is TIMESTAMP-like; timestamp may still be BIGINT
# from_ms/to_ms:
#   if from_ms <= to_ms, inclusive filter; otherwise full file
ReadOHLCVRangeModeSchema ⇐ {𝕊 path‿from_ms‿to_ms‿time_mode‿schema_mode:
  tbl ← Check bridge.ReadRaw ⟨path∾@, from_ms, to_ms, time_mode, schema_mode⟩
  BuildResult tbl
}

# schema_mode: 1=named timestamp/open/high/low/close/volume, 2=numeric 0..5.
# The default auto mode tries named columns first, then numeric columns.
ReadOHLCVRangeModeNamed ← {𝕊 path‿from_ms‿to_ms‿time_mode: ReadOHLCVRangeModeSchema path‿from_ms‿to_ms‿time_mode‿1}
ReadOHLCVRangeModeNumeric ← {𝕊 path‿from_ms‿to_ms‿time_mode: ReadOHLCVRangeModeSchema path‿from_ms‿to_ms‿time_mode‿2}
ReadOHLCVRangeMode ⇐ ReadOHLCVRangeModeNamed⎊ReadOHLCVRangeModeNumeric

# Whole-file readers.
#
# Whole-file readers.
#
# The local data/ BTC file has:
#   0 = epoch milliseconds, 1 = open, 2 = high, 3 = low, 4 = close, 5 = volume
# Therefore the no-suffix default tries epoch-ms first. If that is wrong for a
# different file, it falls back to date, then timestamp-as-timestamp.
ReadOHLCVTimestamp ⇐ {𝕊 path: ReadOHLCVRangeMode path‿0‿¯1‿0}
ReadOHLCVMs        ⇐ {𝕊 path: ReadOHLCVRangeMode path‿0‿¯1‿1}
ReadOHLCVSeconds   ⇐ {𝕊 path: ReadOHLCVRangeMode path‿0‿¯1‿2}
ReadOHLCVDate      ⇐ {𝕊 path: ReadOHLCVRangeMode path‿0‿¯1‿3}
ReadOHLCV          ⇐ ReadOHLCVMs⎊(ReadOHLCVDate⎊ReadOHLCVTimestamp)

# Inclusive time-range readers in epoch milliseconds.
ReadOHLCVRangeTimestamp ⇐ {𝕊 path‿from_ms‿to_ms: ReadOHLCVRangeMode path‿from_ms‿to_ms‿0}
ReadOHLCVMsRange        ⇐ {𝕊 path‿from_ms‿to_ms: ReadOHLCVRangeMode path‿from_ms‿to_ms‿1}
ReadOHLCVSecondsRange   ⇐ {𝕊 path‿from_ms‿to_ms: ReadOHLCVRangeMode path‿from_ms‿to_ms‿2}
ReadOHLCVDateRange      ⇐ {𝕊 path‿from_ms‿to_ms: ReadOHLCVRangeMode path‿from_ms‿to_ms‿3}
ReadOHLCVRange          ⇐ ReadOHLCVMsRange⎊(ReadOHLCVDateRange⎊ReadOHLCVRangeTimestamp)

ReadOHLCVNamedMs      ⇐ {𝕊 path: ReadOHLCVRangeModeSchema path‿0‿¯1‿1‿1}
ReadOHLCVNumericMs    ⇐ {𝕊 path: ReadOHLCVRangeModeSchema path‿0‿¯1‿1‿2}
ReadOHLCVNamedMsRange ⇐ {𝕊 path‿from_ms‿to_ms: ReadOHLCVRangeModeSchema path‿from_ms‿to_ms‿1‿1}
ReadOHLCVNumericMsRange ⇐ {𝕊 path‿from_ms‿to_ms: ReadOHLCVRangeModeSchema path‿from_ms‿to_ms‿1‿2}

DuckDBVersion ⇐ bridge.DuckDBVersion
