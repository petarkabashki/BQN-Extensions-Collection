#include "../include/bqn_plplot.h"

#if defined(__has_include)
#  if __has_include(<plplot/plplot.h>)
#    include <plplot/plplot.h>
#  else
#    include <plplot.h>
#  endif
#else
#  include <plplot/plplot.h>
#endif

#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef BQNPL_MAX_COLORS
#define BQNPL_MAX_COLORS 256
#endif

#ifndef BQNPL_DYNAMIC_COLOR_FLOOR
#define BQNPL_DYNAMIC_COLOR_FLOOR 32
#endif

typedef enum {
  BQNPL_COLOR_INDEX = 0,
  BQNPL_COLOR_RGBA  = 1
} BqnPlColorMode;

typedef struct {
  BqnPlColorMode mode;
  int index;
  int r;
  int g;
  int b;
  double a;
} BqnPlColor;

typedef struct {
  int used;
  int r;
  int g;
  int b;
  double a;
} BqnPlPaletteEntry;

typedef struct {
  int next_dynamic_color;
} BqnPlRenderCtx;

typedef struct {
  int kind;
  int panel;
  int axis;
  BqnPlColor color;
  BqnPlColor color2;
  int line_style;
  double line_width;
  int n;
  char *label;
  char *symbol;
  PLFLT *x;
  PLFLT *y;
  PLFLT *o;
  PLFLT *h;
  PLFLT *l;
  PLFLT *c;
  PLFLT *x1;
} BqnPlSeries;

typedef struct {
  double weight;
  char *ylabel;
  char *y2label;
} BqnPlPanel;

struct BqnPlChart {
  int width_px;
  int height_px;
  char *device;
  char *filename;
  char *title;
  BqnPlPanel *panels;
  int panel_count;
  int panel_cap;
  BqnPlSeries *series;
  int series_count;
  int series_cap;
  BqnPlPaletteEntry palette[BQNPL_MAX_COLORS];
};

static char g_last_error[512] = {0};

static int fail(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(g_last_error, sizeof(g_last_error), fmt, ap);
  va_end(ap);
  return -1;
}

const char *bqnpl_last_error(void) { return g_last_error; }
const char *bqnpl_version(void) { return "bqn-plplot-bridge/0.3-styling"; }

static char *dupstr(const char *s) {
  if (!s) return NULL;
  size_t n = strlen(s) + 1;
  char *r = (char *)malloc(n);
  if (!r) return NULL;
  memcpy(r, s, n);
  return r;
}

static PLFLT *copy_f64(const double *src, int n) {
  if (n <= 0 || !src) return NULL;
  PLFLT *dst = (PLFLT *)malloc((size_t)n * sizeof(PLFLT));
  if (!dst) return NULL;
  for (int i = 0; i < n; ++i) dst[i] = (PLFLT)src[i];
  return dst;
}

static BqnPlColor color_index(int idx) {
  BqnPlColor c;
  c.mode = BQNPL_COLOR_INDEX;
  c.index = idx;
  c.r = c.g = c.b = 0;
  c.a = 1.0;
  return c;
}

static BqnPlColor color_rgba(int r, int g, int b, double a) {
  BqnPlColor c;
  c.mode = BQNPL_COLOR_RGBA;
  c.index = 0;
  c.r = r < 0 ? 0 : (r > 255 ? 255 : r);
  c.g = g < 0 ? 0 : (g > 255 ? 255 : g);
  c.b = b < 0 ? 0 : (b > 255 ? 255 : b);
  c.a = a < 0.0 ? 0.0 : (a > 1.0 ? 1.0 : a);
  return c;
}

static void free_series(BqnPlSeries *s) {
  if (!s) return;
  free(s->label);
  free(s->symbol);
  free(s->x);
  free(s->y);
  free(s->o);
  free(s->h);
  free(s->l);
  free(s->c);
  free(s->x1);
  memset(s, 0, sizeof(*s));
}

BqnPlChart *bqnpl_chart_new(int32_t width_px, int32_t height_px) {
  BqnPlChart *ch = (BqnPlChart *)calloc(1, sizeof(BqnPlChart));
  if (!ch) return NULL;
  ch->width_px = width_px > 0 ? width_px : 1600;
  ch->height_px = height_px > 0 ? height_px : 1000;
  ch->device = dupstr("pngcairo");
  ch->filename = dupstr("chart.png");
  ch->title = dupstr("");
  if (!ch->device || !ch->filename || !ch->title) {
    bqnpl_chart_free(ch);
    return NULL;
  }
  return ch;
}

void bqnpl_chart_free(BqnPlChart *ch) {
  if (!ch) return;
  for (int i = 0; i < ch->panel_count; ++i) {
    free(ch->panels[i].ylabel);
    free(ch->panels[i].y2label);
  }
  for (int i = 0; i < ch->series_count; ++i) free_series(&ch->series[i]);
  free(ch->panels);
  free(ch->series);
  free(ch->device);
  free(ch->filename);
  free(ch->title);
  free(ch);
}

static int reserve_panels(BqnPlChart *ch, int need) {
  if (ch->panel_cap >= need) return 0;
  int cap = ch->panel_cap ? ch->panel_cap * 2 : 4;
  if (cap < need) cap = need;
  BqnPlPanel *p = (BqnPlPanel *)realloc(ch->panels, (size_t)cap * sizeof(BqnPlPanel));
  if (!p) return fail("out of memory reserving panels");
  memset(p + ch->panel_cap, 0, (size_t)(cap - ch->panel_cap) * sizeof(BqnPlPanel));
  ch->panels = p;
  ch->panel_cap = cap;
  return 0;
}

static int reserve_series(BqnPlChart *ch, int need) {
  if (ch->series_cap >= need) return 0;
  int cap = ch->series_cap ? ch->series_cap * 2 : 16;
  if (cap < need) cap = need;
  BqnPlSeries *s = (BqnPlSeries *)realloc(ch->series, (size_t)cap * sizeof(BqnPlSeries));
  if (!s) return fail("out of memory reserving series");
  memset(s + ch->series_cap, 0, (size_t)(cap - ch->series_cap) * sizeof(BqnPlSeries));
  ch->series = s;
  ch->series_cap = cap;
  return 0;
}

static BqnPlSeries *get_series(BqnPlChart *ch, int series_id) {
  if (!ch) { fail("chart is null"); return NULL; }
  if (series_id < 0 || series_id >= ch->series_count) {
    fail("invalid series id %d", series_id);
    return NULL;
  }
  return &ch->series[series_id];
}

int32_t bqnpl_set_output(BqnPlChart *ch, const char *device, const char *filename) {
  if (!ch) return fail("chart is null");
  if (!device) return fail("device must not be null");
  char *d = dupstr(device);
  char *f = dupstr(filename ? filename : "");
  if (!d || !f) { free(d); free(f); return fail("out of memory setting output"); }
  free(ch->device); free(ch->filename);
  ch->device = d; ch->filename = f;
  return 0;
}

static int set_output_owned(BqnPlChart *ch, char *device, char *filename) {
  if (!ch || !device || !filename) { free(device); free(filename); return fail("invalid owned output strings"); }
  free(ch->device);
  free(ch->filename);
  ch->device = device;
  ch->filename = filename;
  return 0;
}

int32_t bqnpl_set_title(BqnPlChart *ch, const char *title) {
  if (!ch) return fail("chart is null");
  char *t = dupstr(title ? title : "");
  if (!t) return fail("out of memory setting title");
  free(ch->title); ch->title = t;
  return 0;
}

int32_t bqnpl_add_panel(BqnPlChart *ch, double height_weight, const char *ylabel, const char *y2label) {
  if (!ch) return fail("chart is null");
  if (reserve_panels(ch, ch->panel_count + 1) != 0) return -1;
  int id = ch->panel_count++;
  ch->panels[id].weight = height_weight > 0.0 ? height_weight : 1.0;
  ch->panels[id].ylabel = dupstr(ylabel ? ylabel : "");
  ch->panels[id].y2label = dupstr(y2label ? y2label : "");
  if (!ch->panels[id].ylabel || !ch->panels[id].y2label) return fail("out of memory adding panel");
  return id;
}

int32_t bqnpl_set_palette_rgba(BqnPlChart *ch, int32_t color_index_, int32_t r, int32_t g, int32_t b, double alpha) {
  if (!ch) return fail("chart is null");
  if (color_index_ < 0 || color_index_ >= BQNPL_MAX_COLORS) return fail("palette color index %d out of range", color_index_);
  ch->palette[color_index_].used = 1;
  ch->palette[color_index_].r = r < 0 ? 0 : (r > 255 ? 255 : r);
  ch->palette[color_index_].g = g < 0 ? 0 : (g > 255 ? 255 : g);
  ch->palette[color_index_].b = b < 0 ? 0 : (b > 255 ? 255 : b);
  ch->palette[color_index_].a = alpha < 0.0 ? 0.0 : (alpha > 1.0 ? 1.0 : alpha);
  return 0;
}

int32_t bqnpl_set_series_style(BqnPlChart *ch, int32_t series_id, int32_t line_style, double line_width) {
  BqnPlSeries *s = get_series(ch, series_id);
  if (!s) return -1;
  s->line_style = line_style > 0 ? line_style : BQNPL_LINE_SOLID;
  s->line_width = line_width > 0.0 ? line_width : 1.0;
  return 0;
}

int32_t bqnpl_set_series_color(BqnPlChart *ch, int32_t series_id, int32_t which_color, int32_t color_index_) {
  BqnPlSeries *s = get_series(ch, series_id);
  if (!s) return -1;
  if (which_color == BQNPL_COLOR_PRIMARY) s->color = color_index(color_index_);
  else if (which_color == BQNPL_COLOR_SECONDARY) s->color2 = color_index(color_index_);
  else return fail("invalid color selector %d", which_color);
  return 0;
}

int32_t bqnpl_set_series_rgba(BqnPlChart *ch, int32_t series_id, int32_t which_color,
                              int32_t r, int32_t g, int32_t b, double alpha) {
  BqnPlSeries *s = get_series(ch, series_id);
  if (!s) return -1;
  if (which_color == BQNPL_COLOR_PRIMARY) s->color = color_rgba(r, g, b, alpha);
  else if (which_color == BQNPL_COLOR_SECONDARY) s->color2 = color_rgba(r, g, b, alpha);
  else return fail("invalid color selector %d", which_color);
  return 0;
}

static BqnPlSeries *new_series(BqnPlChart *ch, int kind, int panel, int axis, const char *label, int color, int color2, int n) {
  if (!ch) { fail("chart is null"); return NULL; }
  if (panel < 0 || panel >= ch->panel_count) { fail("invalid panel index %d", panel); return NULL; }
  if (axis != BQNPL_AXIS_PRIMARY && axis != BQNPL_AXIS_SECONDARY) { fail("invalid axis %d", axis); return NULL; }
  if (n <= 0) { fail("series length must be positive"); return NULL; }
  if (reserve_series(ch, ch->series_count + 1) != 0) return NULL;
  BqnPlSeries *s = &ch->series[ch->series_count++];
  memset(s, 0, sizeof(*s));
  s->kind = kind;
  s->panel = panel;
  s->axis = axis;
  s->label = dupstr(label ? label : "");
  s->color = color_index(color);
  s->color2 = color_index(color2);
  s->line_style = BQNPL_LINE_SOLID;
  s->line_width = 1.0;
  s->n = n;
  if (!s->label) { fail("out of memory adding series"); return NULL; }
  return s;
}

int32_t bqnpl_add_line(BqnPlChart *ch, int32_t panel, int32_t axis, const char *label,
                       int32_t color, int32_t n, const double *x, const double *y) {
  BqnPlSeries *s = new_series(ch, BQNPL_LINE, panel, axis, label, color, 0, n);
  if (!s) return -1;
  s->x = copy_f64(x, n); s->y = copy_f64(y, n);
  if (!s->x || !s->y) return fail("out of memory copying line data");
  return ch->series_count - 1;
}

int32_t bqnpl_add_histogram(BqnPlChart *ch, int32_t panel, int32_t axis, const char *label,
                            int32_t color, int32_t n, const double *x, const double *y) {
  BqnPlSeries *s = new_series(ch, BQNPL_HISTOGRAM, panel, axis, label, color, 0, n);
  if (!s) return -1;
  s->x = copy_f64(x, n); s->y = copy_f64(y, n);
  if (!s->x || !s->y) return fail("out of memory copying histogram data");
  return ch->series_count - 1;
}

int32_t bqnpl_add_volume_bars(BqnPlChart *ch, int32_t panel, int32_t axis, const char *label,
                              int32_t color, int32_t n, const double *x, const double *volume) {
  BqnPlSeries *s = new_series(ch, BQNPL_VOLUME_BAR, panel, axis, label, color, 0, n);
  if (!s) return -1;
  s->x = copy_f64(x, n); s->y = copy_f64(volume, n);
  if (!s->x || !s->y) return fail("out of memory copying volume data");
  return ch->series_count - 1;
}

int32_t bqnpl_add_markers(BqnPlChart *ch, int32_t panel, int32_t axis, const char *label,
                          int32_t color, const char *symbol, int32_t n, const double *x, const double *y) {
  BqnPlSeries *s = new_series(ch, BQNPL_MARKER, panel, axis, label, color, 0, n);
  if (!s) return -1;
  s->symbol = dupstr(symbol ? symbol : "•");
  s->x = copy_f64(x, n); s->y = copy_f64(y, n);
  if (!s->symbol || !s->x || !s->y) return fail("out of memory copying marker data");
  return ch->series_count - 1;
}

int32_t bqnpl_add_candles(BqnPlChart *ch, int32_t panel, int32_t axis, const char *label,
                          int32_t up_color, int32_t down_color, int32_t n,
                          const double *x, const double *open, const double *high,
                          const double *low, const double *close) {
  BqnPlSeries *s = new_series(ch, BQNPL_CANDLE, panel, axis, label, up_color, down_color, n);
  if (!s) return -1;
  s->x = copy_f64(x, n); s->o = copy_f64(open, n); s->h = copy_f64(high, n);
  s->l = copy_f64(low, n); s->c = copy_f64(close, n);
  if (!s->x || !s->o || !s->h || !s->l || !s->c) return fail("out of memory copying candle data");
  return ch->series_count - 1;
}

int32_t bqnpl_add_ohlc_bars(BqnPlChart *ch, int32_t panel, int32_t axis, const char *label,
                            int32_t up_color, int32_t down_color, int32_t n,
                            const double *x, const double *open, const double *high,
                            const double *low, const double *close) {
  BqnPlSeries *s = new_series(ch, BQNPL_OHLC_BAR, panel, axis, label, up_color, down_color, n);
  if (!s) return -1;
  s->x = copy_f64(x, n); s->o = copy_f64(open, n); s->h = copy_f64(high, n);
  s->l = copy_f64(low, n); s->c = copy_f64(close, n);
  if (!s->x || !s->o || !s->h || !s->l || !s->c) return fail("out of memory copying OHLC data");
  return ch->series_count - 1;
}

int32_t bqnpl_add_background_spans(BqnPlChart *ch, int32_t panel, int32_t color,
                                   int32_t n, const double *x0, const double *x1) {
  BqnPlSeries *s = new_series(ch, BQNPL_BG_SPAN, panel, BQNPL_AXIS_PRIMARY, "background", color, 0, n);
  if (!s) return -1;
  s->x = copy_f64(x0, n); s->x1 = copy_f64(x1, n);
  if (!s->x || !s->x1) return fail("out of memory copying background spans");
  return ch->series_count - 1;
}

int32_t bqnpl_add_band(BqnPlChart *ch, int32_t panel, int32_t axis, const char *label,
                       int32_t color, int32_t n, const double *x, const double *y0, const double *y1) {
  BqnPlSeries *s = new_series(ch, BQNPL_BAND, panel, axis, label, color, 0, n);
  if (!s) return -1;
  s->x = copy_f64(x, n); s->y = copy_f64(y0, n); s->x1 = copy_f64(y1, n);
  if (!s->x || !s->y || !s->x1) return fail("out of memory copying band data");
  return ch->series_count - 1;
}

int32_t bqnpl_add_hline(BqnPlChart *ch, int32_t panel, int32_t axis, const char *label,
                        int32_t color, double y) {
  BqnPlSeries *s = new_series(ch, BQNPL_HLINE, panel, axis, label, color, 0, 1);
  if (!s) return -1;
  s->y = (PLFLT *)malloc(sizeof(PLFLT));
  if (!s->y) return fail("out of memory copying hline data");
  s->y[0] = (PLFLT)y;
  return ch->series_count - 1;
}

int32_t bqnpl_add_vline(BqnPlChart *ch, int32_t panel, int32_t axis, const char *label,
                        int32_t color, double x) {
  BqnPlSeries *s = new_series(ch, BQNPL_VLINE, panel, axis, label, color, 0, 1);
  if (!s) return -1;
  s->x = (PLFLT *)malloc(sizeof(PLFLT));
  if (!s->x) return fail("out of memory copying vline data");
  s->x[0] = (PLFLT)x;
  return ch->series_count - 1;
}

static void expand_range(double *mn, double *mx, double v) {
  if (!isfinite(v)) return;
  if (v < *mn) *mn = v;
  if (v > *mx) *mx = v;
}

static double bar_width(const BqnPlSeries *s) {
  if (!s || s->n < 2 || !s->x) return 0.6;
  double acc = 0.0; int cnt = 0;
  for (int i = 1; i < s->n; ++i) {
    double d = fabs(s->x[i] - s->x[i-1]);
    if (d > 0 && isfinite(d)) { acc += d; ++cnt; }
  }
  return cnt ? 0.65 * (acc / cnt) : 0.6;
}

static void panel_ranges(BqnPlChart *ch, int panel, double *xmin, double *xmax, double y_min[2], double y_max[2], int axis_used[2]) {
  *xmin = INFINITY; *xmax = -INFINITY;
  y_min[0] = y_min[1] = INFINITY;
  y_max[0] = y_max[1] = -INFINITY;
  axis_used[0] = axis_used[1] = 0;
  for (int si = 0; si < ch->series_count; ++si) {
    BqnPlSeries *s = &ch->series[si];
    if (s->panel != panel) continue;
    int ax = s->axis;
    for (int i = 0; i < s->n; ++i) {
      switch (s->kind) {
        case BQNPL_BG_SPAN:
          expand_range(xmin, xmax, s->x[i]);
          expand_range(xmin, xmax, s->x1[i]);
          break;
        case BQNPL_HLINE:
          axis_used[ax] = 1;
          expand_range(&y_min[ax], &y_max[ax], s->y[0]);
          break;
        case BQNPL_VLINE:
          expand_range(xmin, xmax, s->x[0]);
          break;
        default:
          expand_range(xmin, xmax, s->x[i]);
          break;
      }
      switch (s->kind) {
        case BQNPL_CANDLE:
        case BQNPL_OHLC_BAR:
          expand_range(&y_min[ax], &y_max[ax], s->l[i]);
          expand_range(&y_min[ax], &y_max[ax], s->h[i]);
          axis_used[ax] = 1;
          break;
        case BQNPL_HISTOGRAM:
        case BQNPL_VOLUME_BAR:
          expand_range(&y_min[ax], &y_max[ax], 0.0);
          expand_range(&y_min[ax], &y_max[ax], s->y[i]);
          axis_used[ax] = 1;
          break;
        case BQNPL_LINE:
        case BQNPL_MARKER:
          expand_range(&y_min[ax], &y_max[ax], s->y[i]);
          axis_used[ax] = 1;
          break;
        case BQNPL_BAND:
          expand_range(&y_min[ax], &y_max[ax], s->y[i]);
          expand_range(&y_min[ax], &y_max[ax], s->x1[i]);
          axis_used[ax] = 1;
          break;
        default: break;
      }
    }
  }
  if (!isfinite(*xmin) || !isfinite(*xmax) || *xmin == *xmax) { *xmin = 0.0; *xmax = 1.0; }
  double xpad = 0.02 * (*xmax - *xmin);
  if (xpad > 0.0 && isfinite(xpad)) { *xmin -= xpad; *xmax += xpad; }
  for (int ax = 0; ax < 2; ++ax) {
    if (!axis_used[ax] || !isfinite(y_min[ax]) || !isfinite(y_max[ax]) || y_min[ax] == y_max[ax]) {
      y_min[ax] = 0.0; y_max[ax] = 1.0;
    }
    double pad = 0.05 * (y_max[ax] - y_min[ax]);
    if (pad <= 0.0 || !isfinite(pad)) pad = 1.0;
    y_min[ax] -= pad; y_max[ax] += pad;
  }
}

static void apply_palette(BqnPlChart *ch) {
  plscmap0n(BQNPL_MAX_COLORS);
  for (int i = 0; i < BQNPL_MAX_COLORS; ++i) {
    if (ch->palette[i].used) {
      plscol0a((PLINT)i, (PLINT)ch->palette[i].r, (PLINT)ch->palette[i].g,
               (PLINT)ch->palette[i].b, (PLFLT)ch->palette[i].a);
    }
  }
}

static void apply_color(BqnPlRenderCtx *ctx, const BqnPlColor *c) {
  if (c->mode == BQNPL_COLOR_RGBA) {
    int slot = ctx->next_dynamic_color--;
    if (slot < BQNPL_DYNAMIC_COLOR_FLOOR) slot = BQNPL_DYNAMIC_COLOR_FLOOR - 1;
    plscol0a((PLINT)slot, (PLINT)c->r, (PLINT)c->g, (PLINT)c->b, (PLFLT)c->a);
    plcol0((PLINT)slot);
  } else {
    plcol0((PLINT)c->index);
  }
}

static void apply_series_style(BqnPlRenderCtx *ctx, const BqnPlSeries *s, int use_secondary_color) {
  apply_color(ctx, use_secondary_color ? &s->color2 : &s->color);
  pllsty((PLINT)(s->line_style > 0 ? s->line_style : BQNPL_LINE_SOLID));
  plwidth((PLFLT)(s->line_width > 0.0 ? s->line_width : 1.0));
}

static void reset_draw_style(void) {
  pllsty((PLINT)BQNPL_LINE_SOLID);
  plwidth((PLFLT)1.0);
}

static void draw_rect(double x0, double x1, double y0, double y1) {
  PLFLT xs[4] = {(PLFLT)x0, (PLFLT)x1, (PLFLT)x1, (PLFLT)x0};
  PLFLT ys[4] = {(PLFLT)y0, (PLFLT)y0, (PLFLT)y1, (PLFLT)y1};
  plfill(4, xs, ys);
}

static void draw_hist_bars(const BqnPlSeries *s) {
  double w = bar_width(s);
  for (int i = 0; i < s->n; ++i) {
    double x0 = s->x[i] - 0.5*w;
    double x1 = s->x[i] + 0.5*w;
    draw_rect(x0, x1, 0.0, s->y[i]);
  }
}

static void draw_band(BqnPlRenderCtx *ctx, const BqnPlSeries *s) {
  if (!s || s->n <= 1) return;
  int m = 2 * s->n;
  PLFLT *xs = (PLFLT *)malloc((size_t)m * sizeof(PLFLT));
  PLFLT *ys = (PLFLT *)malloc((size_t)m * sizeof(PLFLT));
  if (!xs || !ys) { free(xs); free(ys); return; }
  for (int i = 0; i < s->n; ++i) { xs[i] = s->x[i]; ys[i] = s->y[i]; }
  for (int i = 0; i < s->n; ++i) { int j = s->n - 1 - i; xs[s->n+i] = s->x[j]; ys[s->n+i] = s->x1[j]; }
  apply_series_style(ctx, s, 0);
  plfill((PLINT)m, xs, ys);
  free(xs); free(ys);
}

static void draw_ohlc(BqnPlRenderCtx *ctx, const BqnPlSeries *s, int candle) {
  double w = bar_width(s);
  for (int i = 0; i < s->n; ++i) {
    int secondary_color = s->c[i] >= s->o[i] ? 0 : 1;
    apply_series_style(ctx, s, secondary_color);
    pljoin((PLFLT)s->x[i], (PLFLT)s->l[i], (PLFLT)s->x[i], (PLFLT)s->h[i]);
    if (candle) {
      double y0 = s->o[i], y1 = s->c[i];
      if (y0 == y1) {
        pljoin((PLFLT)(s->x[i]-0.35*w), (PLFLT)y0, (PLFLT)(s->x[i]+0.35*w), (PLFLT)y1);
      } else {
        if (y0 > y1) { double t = y0; y0 = y1; y1 = t; }
        draw_rect(s->x[i]-0.35*w, s->x[i]+0.35*w, y0, y1);
      }
    } else {
      pljoin((PLFLT)(s->x[i]-0.35*w), (PLFLT)s->o[i], (PLFLT)s->x[i], (PLFLT)s->o[i]);
      pljoin((PLFLT)s->x[i], (PLFLT)s->c[i], (PLFLT)(s->x[i]+0.35*w), (PLFLT)s->c[i]);
    }
  }
}

static void draw_series(BqnPlRenderCtx *ctx, const BqnPlSeries *s, int axis, double xmin, double xmax, double ymin, double ymax) {
  if (s->axis != axis && s->kind != BQNPL_BG_SPAN) return;
  switch (s->kind) {
    case BQNPL_BG_SPAN:
      apply_series_style(ctx, s, 0);
      for (int i = 0; i < s->n; ++i) draw_rect(s->x[i], s->x1[i], ymin, ymax);
      break;
    case BQNPL_HLINE:
      apply_series_style(ctx, s, 0);
      pljoin((PLFLT)xmin, s->y[0], (PLFLT)xmax, s->y[0]);
      break;
    case BQNPL_VLINE:
      apply_series_style(ctx, s, 0);
      pljoin(s->x[0], (PLFLT)ymin, s->x[0], (PLFLT)ymax);
      break;
    case BQNPL_LINE:
      apply_series_style(ctx, s, 0);
      plline((PLINT)s->n, s->x, s->y);
      break;
    case BQNPL_BAND:
      draw_band(ctx, s);
      break;
    case BQNPL_HISTOGRAM:
    case BQNPL_VOLUME_BAR:
      apply_series_style(ctx, s, 0);
      draw_hist_bars(s);
      break;
    case BQNPL_MARKER:
      apply_series_style(ctx, s, 0);
      plstring((PLINT)s->n, s->x, s->y, s->symbol ? s->symbol : "•");
      break;
    case BQNPL_CANDLE:
      draw_ohlc(ctx, s, 1);
      break;
    case BQNPL_OHLC_BAR:
      draw_ohlc(ctx, s, 0);
      break;
    default:
      break;
  }
  reset_draw_style();
}

static int needs_output_file(const char *device) {
  if (!device || !device[0]) return 0;
  const char *interactive[] = {"xwin", "xcairo", "qtwidget", "tk", "ntk", "wingcc", "wincairo", "wxwidgets", NULL};
  for (int i = 0; interactive[i]; ++i) {
    if (strcmp(device, interactive[i]) == 0) return 0;
  }
  return 1;
}

static int render_impl(BqnPlChart *ch) {
  if (!ch) return fail("chart is null");
  if (ch->panel_count <= 0) return fail("no panels have been added");
  if (ch->series_count <= 0) return fail("no series have been added");

  const char *device = (ch->device && ch->device[0]) ? ch->device : "pngcairo";
  const char *filename = ch->filename ? ch->filename : "";
  if (needs_output_file(device) && !filename[0]) return fail("device '%s' requires a non-empty output filename", device);

  plsdev(device);
  if (filename[0]) plsfnam(filename);
  plspage(0, 0, ch->width_px, ch->height_px, 0, 0);
  plinit();
  apply_palette(ch);
  pladv(0);
  plschr(0.0, 0.1875);

  BqnPlRenderCtx ctx;
  ctx.next_dynamic_color = BQNPL_MAX_COLORS - 1;

  double total_w = 0.0;
  for (int i = 0; i < ch->panel_count; ++i) total_w += ch->panels[i].weight;
  if (total_w <= 0.0) total_w = (double)ch->panel_count;

  const double left = 0.10, right = 0.87, top = 0.92, bottom = 0.08, gap = 0.035;
  const double avail = top - bottom - gap * (ch->panel_count - 1);
  double y_top = top;

  for (int p = 0; p < ch->panel_count; ++p) {
    double ph = avail * (ch->panels[p].weight / total_w);
    double y_bot = y_top - ph;
    double xmin, xmax, ymin[2], ymax[2]; int used[2];
    panel_ranges(ch, p, &xmin, &xmax, ymin, ymax, used);

    plvpor((PLFLT)left, (PLFLT)right, (PLFLT)y_bot, (PLFLT)y_top);
    plwind((PLFLT)xmin, (PLFLT)xmax, (PLFLT)ymin[0], (PLFLT)ymax[0]);
    plcol0(15);
    reset_draw_style();
    plbox("bcnst", 0.0, 0, "bcnstv", 0.0, 0);
    if (p == 0 && ch->title && ch->title[0]) pllab("", ch->panels[p].ylabel ? ch->panels[p].ylabel : "", ch->title);
    else pllab("", ch->panels[p].ylabel ? ch->panels[p].ylabel : "", "");

    /* Background spans first on primary axis. */
    for (int si = 0; si < ch->series_count; ++si) {
      BqnPlSeries *s = &ch->series[si];
      if (s->panel == p && s->kind == BQNPL_BG_SPAN) draw_series(&ctx, s, BQNPL_AXIS_PRIMARY, xmin, xmax, ymin[0], ymax[0]);
    }
    for (int si = 0; si < ch->series_count; ++si) {
      BqnPlSeries *s = &ch->series[si];
      if (s->panel == p && s->kind != BQNPL_BG_SPAN) draw_series(&ctx, s, BQNPL_AXIS_PRIMARY, xmin, xmax, ymin[0], ymax[0]);
    }

    if (used[1]) {
      plvpor((PLFLT)left, (PLFLT)right, (PLFLT)y_bot, (PLFLT)y_top);
      plwind((PLFLT)xmin, (PLFLT)xmax, (PLFLT)ymin[1], (PLFLT)ymax[1]);
      plcol0(15);
      reset_draw_style();
      plbox("", 0.0, 0, "cmstv", 0.0, 0);
      if (ch->panels[p].y2label && ch->panels[p].y2label[0]) {
        plmtex("r", 3.0, 0.5, 0.5, ch->panels[p].y2label);
      }
      for (int si = 0; si < ch->series_count; ++si) {
        BqnPlSeries *s = &ch->series[si];
        if (s->panel == p && s->kind != BQNPL_BG_SPAN) draw_series(&ctx, s, BQNPL_AXIS_SECONDARY, xmin, xmax, ymin[1], ymax[1]);
      }
    }

    y_top = y_bot - gap;
  }

  plend();
  return 0;
}

int32_t bqnpl_render(BqnPlChart *ch) {
  return render_impl(ch);
}

int32_t bqnpl_show(BqnPlChart *ch, const char *device) {
  if (!ch) return fail("chart is null");
  char *old_device = ch->device ? dupstr(ch->device) : dupstr("pngcairo");
  char *old_filename = ch->filename ? dupstr(ch->filename) : dupstr("chart.png");
  char *show_device = dupstr((device && device[0]) ? device : "xwin");
  char *show_filename = dupstr("");
  if (!old_device || !old_filename || !show_device || !show_filename) {
    free(old_device); free(old_filename); free(show_device); free(show_filename);
    return fail("out of memory preparing interactive output");
  }

  int rc = set_output_owned(ch, show_device, show_filename);
  if (rc == 0) rc = render_impl(ch);

  /* Restore previous output even if rendering failed. */
  set_output_owned(ch, old_device, old_filename);
  return rc;
}
