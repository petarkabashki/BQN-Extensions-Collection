#ifndef BQN_PLPLOT_H
#define BQN_PLPLOT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BqnPlChart BqnPlChart;

/* Series kind IDs. Kept stable for BQN code. */
enum {
  BQNPL_PRICE_LINE = 1,
  BQNPL_OHLC_BAR   = 2,
  BQNPL_CANDLE     = 3,
  BQNPL_VOLUME_BAR = 4,
  BQNPL_LINE       = 5,
  BQNPL_HISTOGRAM  = 6,
  BQNPL_MARKER     = 7,
  BQNPL_BG_SPAN    = 8,
  BQNPL_BAND       = 9,
  BQNPL_HLINE      = 10,
  BQNPL_VLINE      = 11
};

enum {
  BQNPL_AXIS_PRIMARY   = 0,
  BQNPL_AXIS_SECONDARY = 1
};

enum {
  BQNPL_COLOR_PRIMARY   = 0,
  BQNPL_COLOR_SECONDARY = 1
};

enum {
  BQNPL_LINE_SOLID     = 1,
  BQNPL_LINE_DASH      = 2,
  BQNPL_LINE_DOT       = 3,
  BQNPL_LINE_DASH_DOT  = 4,
  BQNPL_LINE_LONG_DASH = 5
};

/* Lifecycle/config */
BqnPlChart *bqnpl_chart_new(int32_t width_px, int32_t height_px);
void        bqnpl_chart_free(BqnPlChart *chart);
int32_t     bqnpl_set_output(BqnPlChart *chart, const char *device, const char *filename);
int32_t     bqnpl_show(BqnPlChart *chart, const char *device);
int32_t     bqnpl_set_title(BqnPlChart *chart, const char *title);
int32_t     bqnpl_add_panel(BqnPlChart *chart, double height_weight, const char *ylabel, const char *y2label);

/* Palette and per-series styling. RGB values are 0..255; alpha is 0..1. */
int32_t bqnpl_set_palette_rgba(BqnPlChart *chart, int32_t color_index,
                               int32_t r, int32_t g, int32_t b, double alpha);
int32_t bqnpl_set_series_style(BqnPlChart *chart, int32_t series_id,
                               int32_t line_style, double line_width);
int32_t bqnpl_set_series_color(BqnPlChart *chart, int32_t series_id,
                               int32_t which_color, int32_t color_index);
int32_t bqnpl_set_series_rgba(BqnPlChart *chart, int32_t series_id,
                              int32_t which_color, int32_t r, int32_t g, int32_t b, double alpha);

/* Data layers. All arrays are copied by the C bridge. */
int32_t bqnpl_add_line(BqnPlChart *chart, int32_t panel, int32_t axis, const char *label,
                       int32_t color, int32_t n, const double *x, const double *y);
int32_t bqnpl_add_histogram(BqnPlChart *chart, int32_t panel, int32_t axis, const char *label,
                            int32_t color, int32_t n, const double *x, const double *y);
int32_t bqnpl_add_markers(BqnPlChart *chart, int32_t panel, int32_t axis, const char *label,
                          int32_t color, const char *symbol, int32_t n, const double *x, const double *y);
int32_t bqnpl_add_candles(BqnPlChart *chart, int32_t panel, int32_t axis, const char *label,
                          int32_t up_color, int32_t down_color, int32_t n,
                          const double *x, const double *open, const double *high,
                          const double *low, const double *close);
int32_t bqnpl_add_ohlc_bars(BqnPlChart *chart, int32_t panel, int32_t axis, const char *label,
                            int32_t up_color, int32_t down_color, int32_t n,
                            const double *x, const double *open, const double *high,
                            const double *low, const double *close);
int32_t bqnpl_add_volume_bars(BqnPlChart *chart, int32_t panel, int32_t axis, const char *label,
                              int32_t color, int32_t n, const double *x, const double *volume);
int32_t bqnpl_add_background_spans(BqnPlChart *chart, int32_t panel, int32_t color,
                                   int32_t n, const double *x0, const double *x1);
int32_t bqnpl_add_band(BqnPlChart *chart, int32_t panel, int32_t axis, const char *label,
                       int32_t color, int32_t n, const double *x, const double *y0, const double *y1);
int32_t bqnpl_add_hline(BqnPlChart *chart, int32_t panel, int32_t axis, const char *label,
                        int32_t color, double y);
int32_t bqnpl_add_vline(BqnPlChart *chart, int32_t panel, int32_t axis, const char *label,
                        int32_t color, double x);

/* Render/export */
int32_t     bqnpl_render(BqnPlChart *chart);
const char *bqnpl_last_error(void);
const char *bqnpl_version(void);

#ifdef __cplusplus
}
#endif

#endif /* BQN_PLPLOT_H */
