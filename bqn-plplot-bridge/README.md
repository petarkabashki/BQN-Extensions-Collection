# BQN → PLplot bridge for price/strategy charts

This is a small native bridge intended for CBQN `•FFI`. BQN owns the data and chart intent; the C bridge owns PLplot state, copies BQN arrays, draws panels, and exports to a PLplot device such as `pngcairo`, `svg`, `svgcairo`, `pdf`, or `pdfcairo`. It can also open interactive PLplot devices such as `xwin`, `xcairo`, or `qtwidget`.

Version 0.3 adds first-class chart styling:

- horizontal reference lines
- vertical event/session lines
- arbitrary two-point segments
- PLplot line styles: solid, dash, dot, dash-dot, long-dash
- line widths per series
- palette RGBA colors with alpha
- per-series explicit RGBA colors with alpha
- background spans and filled bands with transparency
- data-coordinate text labels for annotations

## Install dependencies on Ubuntu 24.04

```bash
sudo apt update
sudo apt install build-essential pkg-config libplplot-dev plplot-driver-cairo plplot-driver-xwin
```

Optional interactive drivers:

```bash
sudo apt install plplot-driver-qt
```

## Build

```bash
make
```

If `pkg-config` cannot find PLplot, edit `PLPLOT_CFLAGS` and `PLPLOT_LIBS` in the `Makefile`.

## Smoke tests

File export:

```bash
/mnt/data/BQN examples/smoke.bqn
xdg-open build/smoke.png
```

Interactive popup, similar to gnuplot:

```bash
/mnt/data/BQN examples/smoke_popup.bqn
```

Styling smoke test:

```bash
/mnt/data/BQN examples/styling.bqn
xdg-open build/styling.png
```

Comprehensive strategy visualization:

```bash
/mnt/data/BQN examples/strategy_visualization.bqn
xdg-open build/strategy_visualization.png
```

The default popup device is `xwin`, provided by `plplot-driver-xwin` on Ubuntu. Depending on installed drivers you can also use `xcairo` or `qtwidget`:

```bqn
plt.Show ch
plt.ShowDevice ch‿"xcairo"
plt.ShowDevice ch‿"qtwidget"
```

If you are on Wayland, `xwin` usually works through XWayland. If it does not, try `xcairo` or `qtwidget` after installing the corresponding PLplot driver package.

## BQN-level API quick start

```bqn
plt ← •Import "bqn/plplot.bqn"

ch ← plt.New 1600‿1000
plt.SetOutput ch‿"pngcairo"‿"build/chart.png"
plt.SetTitle ch‿"Strategy chart"

price ← plt.Panel ch‿4‿"Price"‿"ATR"
vol ← plt.Panel ch‿1‿"Volume"‿""

# Custom RGBA palette entries. Use indexes >=16 for your own palette.
plt.PaletteRGBA ch‿20‿80‿220‿120‿0.22
plt.PaletteRGBA ch‿21‿255‿180‿40‿0.80

candles ← plt.Candles ch‿price‿plt.axisPrimary‿"BTC"‿3‿2‿x‿open‿high‿low‿close
ema20 ← plt.Line ch‿price‿plt.axisPrimary‿"EMA 20"‿4‿x‿ema20Values
atr ← plt.Line ch‿price‿plt.axisSecondary‿"ATR"‿8‿x‿atrValues
support ← plt.HLine ch‿price‿plt.axisPrimary‿"Support"‿21‿63450
session ← plt.VLine ch‿price‿plt.axisPrimary‿"Session"‿21‿50
inPosition ← plt.BackgroundSpans ch‿price‿20‿entryX‿exitX
fibLabel ← plt.Text ch‿price‿plt.axisPrimary‿"Fib"‿5‿"1.618"‿1‿0‿0‿0.18‿⟨120⟩‿⟨69000⟩

plt.Style ch‿ema20‿plt.styleDash‿2.5
plt.Style ch‿support‿plt.styleLongDash‿2.0
plt.SeriesRGBA ch‿atr‿plt.colorPrimary‿255‿255‿255‿0.55

plt.VolumeBars ch‿vol‿plt.axisPrimary‿"Volume"‿8‿x‿volume
plt.Render ch
plt.Show ch
plt.Free ch
```

## Supported layers

- price as line, OHLC bars, or candlesticks
- overlay indicators as line series
- filled bands for Ichimoku clouds, Bollinger-like envelopes, confidence bands, or regime zones
- marker series for entries/exits
- background spans for in-position windows or regimes
- horizontal lines for support/resistance, RSI thresholds, stop-loss/take-profit levels
- vertical lines for sessions, news events, walk-forward split points, or trade lifecycle markers
- arbitrary two-point line segments
- text labels at data coordinates
- volume bars in a lower panel
- subplots for RSI, MACD, custom lines, histograms, or combinations
- primary and secondary y-axis per panel
- file export through PLplot devices
- interactive popup display through PLplot devices such as `xwin`, `xcairo`, or `qtwidget`

## Styling support matrix

| Feature | API | Notes |
|---|---|---|
| Indexed colors | layer creation color arguments | PLplot color-map0 index |
| Palette RGBA | `PaletteRGBA` | Good for reusable theme colors and transparent spans/bands |
| Per-series RGBA | `SeriesRGBA` | Good for one-off colors/transparency |
| Line style | `Style` | Solid/dash/dot/dash-dot/long-dash |
| Line width | `Style` | Applies to lines, markers, hlines/vlines, OHLC outlines/wicks |
| Horizontal line | `HLine` | Full panel x-range |
| Vertical line | `VLine` | Full panel y-range |
| Segment | `Segment` | Two-point arbitrary line |
| Transparent fills | `PaletteRGBA` or `SeriesRGBA` | Driver must support alpha; Cairo/SVG/PDF devices usually do |
| Text labels | `Text` plus `SeriesRGBA` | Places UTF-8 text at data coordinates with PLplot direction, justification, and size |

## Documentation

- [`docs/API.md`](docs/API.md) — complete BQN wrapper API
- [`docs/STYLING.md`](docs/STYLING.md) — colors, alpha, line styles, widths, annotations
- [`docs/EXAMPLES.md`](docs/EXAMPLES.md) — examples and expected output files
- [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) — bridge architecture and data flow

## Notes

The C bridge copies all input arrays. This is intentional: BQN arrays are immutable and the bridge stores series until `Render` or `Show` is called. Never store raw pointers into BQN-owned arrays beyond a single FFI call.

Alpha support depends on the PLplot driver. Prefer `pngcairo`, `svgcairo`, `pdfcairo`, `xcairo`, or other Cairo-capable drivers for transparent fills and lines. Non-alpha drivers may ignore alpha and render colors as opaque.

This is still a scaffold rather than a full production charting library. Next useful additions would be legends, date/time x-axis formatting, themes, persistent interactive windows, hover/pick callbacks where the selected PLplot driver supports them, and a higher-level BQN chart-spec builder.
