#include "bqn_duckdb_bridge.h"

#include <duckdb.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BQN_PRICE_COLS 6

struct BqnPriceTable {
    int64_t status;
    int64_t rows;
    int64_t cols;
    int64_t capacity;
    double *data[BQN_PRICE_COLS];
    char *error;
};

static const char *COL_NAMES[BQN_PRICE_COLS] = {
    "ts_ms", "open", "high", "low", "close", "volume"
};

static char *dup_cstr(const char *s) {
    if (!s) {
        s = "";
    }
    const size_t n = strlen(s);
    char *out = (char *)malloc(n + 1);
    if (!out) {
        return NULL;
    }
    memcpy(out, s, n + 1);
    return out;
}

static BqnPriceTable *table_new(void) {
    BqnPriceTable *t = (BqnPriceTable *)calloc(1, sizeof(BqnPriceTable));
    if (!t) {
        return NULL;
    }
    t->status = 0;
    t->rows = 0;
    t->cols = BQN_PRICE_COLS;
    t->capacity = 0;
    return t;
}

static void table_set_error(BqnPriceTable *t, int64_t status, const char *msg) {
    if (!t) {
        return;
    }
    t->status = status;
    free(t->error);
    t->error = dup_cstr(msg ? msg : "unknown error");
}

static BqnPriceTable *table_error(int64_t status, const char *msg) {
    BqnPriceTable *t = table_new();
    if (!t) {
        return NULL;
    }
    table_set_error(t, status, msg);
    return t;
}

static void table_free_columns(BqnPriceTable *t) {
    if (!t) {
        return;
    }
    for (int i = 0; i < BQN_PRICE_COLS; ++i) {
        free(t->data[i]);
        t->data[i] = NULL;
    }
    t->rows = 0;
    t->capacity = 0;
}

void bqn_price_table_free(BqnPriceTable *t) {
    if (!t) {
        return;
    }
    table_free_columns(t);
    free(t->error);
    free(t);
}

static bool table_reserve(BqnPriceTable *t, int64_t required) {
    if (!t || required < 0) {
        return false;
    }
    if (required <= t->capacity) {
        return true;
    }

    int64_t cap = t->capacity > 0 ? t->capacity : 1024;
    while (cap < required) {
        if (cap > INT64_MAX / 2) {
            return false;
        }
        cap *= 2;
    }

    if ((uint64_t)cap > (uint64_t)SIZE_MAX / sizeof(double)) {
        return false;
    }
    const size_t bytes = (size_t)cap * sizeof(double);

    double *new_cols[BQN_PRICE_COLS] = {0};
    for (int c = 0; c < BQN_PRICE_COLS; ++c) {
        new_cols[c] = (double *)realloc(t->data[c], bytes);
        if (!new_cols[c]) {
            /* Any successful realloc calls have already updated memory. Keep them. */
            for (int j = 0; j < c; ++j) {
                t->data[j] = new_cols[j];
            }
            return false;
        }
    }

    for (int c = 0; c < BQN_PRICE_COLS; ++c) {
        t->data[c] = new_cols[c];
    }
    t->capacity = cap;
    return true;
}

static char *sql_quote_literal(const char *s) {
    if (!s) {
        return NULL;
    }
    const size_t n = strlen(s);
    size_t quotes = 0;
    for (size_t i = 0; i < n; ++i) {
        if (s[i] == '\'') {
            quotes++;
        }
    }

    char *out = (char *)malloc(n + quotes + 3);
    if (!out) {
        return NULL;
    }

    char *p = out;
    *p++ = '\'';
    for (size_t i = 0; i < n; ++i) {
        *p++ = s[i];
        if (s[i] == '\'') {
            *p++ = '\'';
        }
    }
    *p++ = '\'';
    *p = '\0';
    return out;
}

static const char *time_expr_for_mode(int32_t time_mode, int32_t schema_mode) {
    switch (time_mode) {
        case 0:
            return schema_mode == 2 ? "CAST(epoch_ms(CAST(\"0\" AS TIMESTAMP)) AS DOUBLE)" : "CAST(epoch_ms(CAST(\"timestamp\" AS TIMESTAMP)) AS DOUBLE)";
        case 1:
            return schema_mode == 2 ? "CAST(\"0\" AS DOUBLE)" : "CAST(\"timestamp\" AS DOUBLE)";
        case 2:
            return schema_mode == 2 ? "CAST(\"0\" AS DOUBLE) * 1000.0" : "CAST(\"timestamp\" AS DOUBLE) * 1000.0";
        case 3:
            return "CAST(epoch_ms(CAST(\"date\" AS TIMESTAMP)) AS DOUBLE)";
        default:
            return NULL;
    }
}

static char *build_sql(const char *parquet_path, int64_t from_ms, int64_t to_ms, int32_t time_mode, int32_t schema_mode) {
    char *path_lit = sql_quote_literal(parquet_path);
    if (!path_lit) {
        return NULL;
    }

    const char *ts_expr = time_expr_for_mode(time_mode, schema_mode);
    if (!ts_expr) {
        free(path_lit);
        return NULL;
    }

    const bool use_filter = from_ms <= to_ms;

    const char *open_col = schema_mode == 2 ? "\"1\"" : "\"open\"";
    const char *high_col = schema_mode == 2 ? "\"2\"" : "\"high\"";
    const char *low_col = schema_mode == 2 ? "\"3\"" : "\"low\"";
    const char *close_col = schema_mode == 2 ? "\"4\"" : "\"close\"";
    const char *volume_col = schema_mode == 2 ? "\"5\"" : "\"volume\"";

    const char *prefix =
        "WITH q AS ("
        " SELECT %s AS ts_ms,"
        "        CAST(%s AS DOUBLE) AS open,"
        "        CAST(%s AS DOUBLE) AS high,"
        "        CAST(%s AS DOUBLE) AS low,"
        "        CAST(%s AS DOUBLE) AS close,"
        "        CAST(%s AS DOUBLE) AS volume"
        " FROM read_parquet(%s)"
        ")"
        " SELECT ts_ms, open, high, low, close, volume FROM q";

    const char *filter = " WHERE ts_ms >= %lld AND ts_ms <= %lld";
    const char *suffix = " ORDER BY ts_ms";

    int needed = snprintf(NULL, 0, prefix, ts_expr, open_col, high_col, low_col, close_col, volume_col, path_lit);
    if (needed < 0) {
        free(path_lit);
        return NULL;
    }
    if (use_filter) {
        int f = snprintf(NULL, 0, filter, (long long)from_ms, (long long)to_ms);
        if (f < 0) {
            free(path_lit);
            return NULL;
        }
        needed += f;
    }
    needed += (int)strlen(suffix) + 1;

    char *sql = (char *)malloc((size_t)needed);
    if (!sql) {
        free(path_lit);
        return NULL;
    }

    int offset = snprintf(sql, (size_t)needed, prefix, ts_expr, open_col, high_col, low_col, close_col, volume_col, path_lit);
    if (use_filter) {
        offset += snprintf(sql + offset, (size_t)needed - (size_t)offset,
                           filter, (long long)from_ms, (long long)to_ms);
    }
    snprintf(sql + offset, (size_t)needed - (size_t)offset, "%s", suffix);

    free(path_lit);
    return sql;
}

static bool validity_row_is_valid(uint64_t *validity, idx_t row) {
    if (!validity) {
        return true;
    }
    return duckdb_validity_row_is_valid(validity, row);
}

static bool copy_chunk(BqnPriceTable *t, duckdb_data_chunk chunk) {
    const idx_t n = duckdb_data_chunk_get_size(chunk);
    if (n == 0) {
        return true;
    }
    if ((uint64_t)n > (uint64_t)INT64_MAX || t->rows > INT64_MAX - (int64_t)n) {
        table_set_error(t, 1, "too many rows for int64 row count");
        return false;
    }

    const int64_t base = t->rows;
    const int64_t new_rows = t->rows + (int64_t)n;
    if (!table_reserve(t, new_rows)) {
        table_set_error(t, 1, "failed to allocate output columns");
        return false;
    }

    for (idx_t c = 0; c < BQN_PRICE_COLS; ++c) {
        duckdb_vector vec = duckdb_data_chunk_get_vector(chunk, c);
        double *src = (double *)duckdb_vector_get_data(vec);
        uint64_t *validity = duckdb_vector_get_validity(vec);

        if (!src) {
            table_set_error(t, 2, "DuckDB did not expose expected DOUBLE vector data");
            return false;
        }

        for (idx_t r = 0; r < n; ++r) {
            if (!validity_row_is_valid(validity, r)) {
                char msg[256];
                snprintf(msg, sizeof(msg), "NULL in required column '%s'", COL_NAMES[c]);
                table_set_error(t, 3, msg);
                return false;
            }
            t->data[c][base + (int64_t)r] = src[r];
        }
    }

    t->rows = new_rows;
    return true;
}

BqnPriceTable *bqn_price_read_ohlcv(
    const char *parquet_path,
    int64_t from_ms,
    int64_t to_ms,
    int32_t time_mode,
    int32_t schema_mode
) {
    if (!parquet_path || parquet_path[0] == '\0') {
        return table_error(1, "parquet_path is empty");
    }
    if (!time_expr_for_mode(time_mode, schema_mode)) {
        return table_error(1, "invalid time_mode; expected 0=timestamp, 1=epoch_ms, 2=epoch_seconds, 3=date_column");
    }
    if (schema_mode < 1 || schema_mode > 2) {
        return table_error(1, "invalid schema_mode; expected 1=named, 2=numeric");
    }

    BqnPriceTable *t = table_new();
    if (!t) {
        return NULL;
    }

    char *sql = build_sql(parquet_path, from_ms, to_ms, time_mode, schema_mode);
    if (!sql) {
        table_set_error(t, 1, "failed to allocate SQL string");
        return t;
    }

    duckdb_database db = NULL;
    duckdb_connection con = NULL;
    duckdb_result result;
    memset(&result, 0, sizeof(result));

    if (duckdb_open(NULL, &db) == DuckDBError) {
        free(sql);
        table_set_error(t, 2, "duckdb_open failed");
        return t;
    }
    if (duckdb_connect(db, &con) == DuckDBError) {
        duckdb_close(&db);
        free(sql);
        table_set_error(t, 2, "duckdb_connect failed");
        return t;
    }

    duckdb_state qstate = duckdb_query(con, sql, &result);
    free(sql);

    if (qstate == DuckDBError) {
        const char *err = duckdb_result_error(&result);
        table_set_error(t, 2, err ? err : "DuckDB query failed");
        duckdb_destroy_result(&result);
        duckdb_disconnect(&con);
        duckdb_close(&db);
        return t;
    }

    if (duckdb_column_count(&result) != BQN_PRICE_COLS) {
        table_set_error(t, 2, "unexpected result column count");
        duckdb_destroy_result(&result);
        duckdb_disconnect(&con);
        duckdb_close(&db);
        return t;
    }

    for (;;) {
        duckdb_data_chunk chunk = duckdb_fetch_chunk(result);
        if (!chunk) {
            break;
        }
        const bool ok = copy_chunk(t, chunk);
        duckdb_destroy_data_chunk(&chunk);
        if (!ok) {
            break;
        }
    }

    duckdb_destroy_result(&result);
    duckdb_disconnect(&con);
    duckdb_close(&db);
    return t;
}

int64_t bqn_price_table_status(const BqnPriceTable *t) {
    return t ? t->status : 1;
}

int64_t bqn_price_table_rows(const BqnPriceTable *t) {
    return t ? t->rows : 0;
}

int64_t bqn_price_table_cols(const BqnPriceTable *t) {
    return t ? t->cols : 0;
}

const char *bqn_price_table_error(const BqnPriceTable *t) {
    if (!t) {
        return "BqnPriceTable pointer is NULL";
    }
    return t->error ? t->error : "";
}

const double *bqn_price_table_col_ptr(const BqnPriceTable *t, int32_t col) {
    if (!t || col < 0 || col >= BQN_PRICE_COLS) {
        return NULL;
    }
    return t->data[col];
}

double bqn_price_table_get(const BqnPriceTable *t, int32_t col, int64_t row) {
    if (!t || col < 0 || col >= BQN_PRICE_COLS || row < 0 || row >= t->rows || !t->data[col]) {
        return 0.0 / 0.0;
    }
    return t->data[col][row];
}

const char *bqn_price_duckdb_version(void) {
    return duckdb_library_version();
}
