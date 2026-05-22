#ifndef BQN_DUCKDB_BRIDGE_H
#define BQN_DUCKDB_BRIDGE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Opaque BQN-facing price table returned by bqn_price_read_ohlcv().
 *
 * Ownership:
 *   - bqn_price_read_ohlcv returns a heap-allocated BqnPriceTable*.
 *   - Always release it with bqn_price_table_free, including error results.
 *   - Column pointers returned by bqn_price_table_col_ptr are borrowed and become
 *     invalid after bqn_price_table_free.
 *
 * Status codes:
 *   0 = OK
 *   1 = bad argument / allocation failure
 *   2 = DuckDB query / schema / type error
 *   3 = NULL found in required OHLCV output
 *   4 = invalid column or row access
 *
 * Time modes:
 *   0 = timestamp column is DuckDB TIMESTAMP-like; use epoch_ms(CAST("timestamp" AS TIMESTAMP))
 *   1 = timestamp column is epoch milliseconds; use CAST("timestamp" AS DOUBLE)
 *   2 = timestamp column is epoch seconds; use CAST("timestamp" AS DOUBLE) * 1000.0
 *   3 = date column is DuckDB TIMESTAMP-like; use epoch_ms(CAST("date" AS TIMESTAMP))
 *
 * Range:
 *   If from_ms <= to_ms, rows are filtered inclusively by ts_ms.
 *   If from_ms > to_ms, no time filter is applied.
 *
 * Expected Parquet columns:
 *   schema_mode 1: timestamp, open, high, low, close, volume
 *   schema_mode 2: 0, 1, 2, 3, 4, 5
 *   schema_mode 3: Date, Open, High, Low, Close, Volume
 *   For time mode 3, date is also required for schema modes 1-2; schema mode 3 uses Date.
 */

typedef struct BqnPriceTable BqnPriceTable;

BqnPriceTable *bqn_price_read_ohlcv(
    const char *parquet_path,
    int64_t from_ms,
    int64_t to_ms,
    int32_t time_mode,
    int32_t schema_mode
);

void bqn_price_table_free(BqnPriceTable *table);

int64_t bqn_price_table_status(const BqnPriceTable *table);
int64_t bqn_price_table_rows(const BqnPriceTable *table);
int64_t bqn_price_table_cols(const BqnPriceTable *table);
const char *bqn_price_table_error(const BqnPriceTable *table);

/* Column order: 0=ts_ms, 1=open, 2=high, 3=low, 4=close, 5=volume. */
const double *bqn_price_table_col_ptr(const BqnPriceTable *table, int32_t col);
double bqn_price_table_get(const BqnPriceTable *table, int32_t col, int64_t row);

const char *bqn_price_duckdb_version(void);

#ifdef __cplusplus
}
#endif

#endif /* BQN_DUCKDB_BRIDGE_H */
