# Styling guide

The bridge supports two color paths:

1. **Palette-index colors**: use PLplot color-map0 integer indexes.
2. **Explicit per-series RGBA**: attach RGB + alpha directly to a series.

It also supports line styles and line widths per series.

## Palette colors

Use `PaletteRGBA` to define reusable colors:

```bqn
plt.PaletteRGBA ch‿20‿80‿220‿120‿0.22
plt.PaletteRGBA ch‿21‿255‿180‿40‿0.80
```

The arguments are:

```text
chart ‿ paletteIndex ‿ red ‿ green ‿ blue ‿ alpha
```

RGB values are `0..255`. Alpha is `0..1`.

Use indexes `16` and above for your theme colors. PLplot's low indexes are commonly used for default foreground/background colors and basic plot colors.

## Per-series RGBA

Use `SeriesRGBA` when a layer needs its own custom color/transparency:

```bqn
ema ← plt.Line ch‿price‿plt.axisPrimary‿"EMA"‿4‿x‿ema
plt.SeriesRGBA ch‿ema‿plt.colorPrimary‿80‿255‿120‿0.85
```

For candles and OHLC bars, use `colorPrimary` for up bars and `colorSecondary` for down bars:

```bqn
candles ← plt.Candles ch‿price‿plt.axisPrimary‿"OHLC"‿3‿2‿x‿open‿high‿low‿close
plt.SeriesRGBA ch‿candles‿plt.colorPrimary‿40‿230‿110‿0.90
plt.SeriesRGBA ch‿candles‿plt.colorSecondary‿255‿80‿80‿0.90
```

## Switching back to palette index

```bqn
plt.SeriesColor ch‿ema‿plt.colorPrimary‿4
```

## Line styles

```bqn
plt.Style ch‿series‿plt.styleSolid‿1.0
plt.Style ch‿series‿plt.styleDash‿2.5
plt.Style ch‿series‿plt.styleDot‿1.8
plt.Style ch‿series‿plt.styleDashDot‿2.0
plt.Style ch‿series‿plt.styleLongDash‿2.0
```

The fourth argument is line width. PLplot interprets width in device-dependent line units.

## Horizontal and vertical annotations

Support/resistance:

```bqn
sup ← plt.HLine ch‿price‿plt.axisPrimary‿"Support"‿21‿101.5
res ← plt.HLine ch‿price‿plt.axisPrimary‿"Resistance"‿21‿111.5
plt.Style ch‿sup‿plt.styleLongDash‿2.0
plt.Style ch‿res‿plt.styleLongDash‿2.0
```

Events, sessions, train/test split points:

```bqn
split ← plt.VLine ch‿price‿plt.axisPrimary‿"WFO split"‿23‿80
plt.Style ch‿split‿plt.styleDot‿1.2
```

## Segments and trend lines

```bqn
trend ← plt.Segment ch‿price‿plt.axisPrimary‿"Trend"‿22‿12‿100‿115‿114
plt.Style ch‿trend‿plt.styleDashDot‿2.5
```

`Segment` is just a two-point line. It is suitable for fitted trend lines, local support/resistance, slope channels, and diagnostic regression segments.

## Transparent spans and bands

Background spans use the panel y-range:

```bqn
plt.PaletteRGBA ch‿20‿80‿220‿120‿0.22
plt.BackgroundSpans ch‿price‿20‿entryX‿exitX
```

Bands fill between two y-series:

```bqn
plt.PaletteRGBA ch‿21‿80‿180‿255‿0.22
cloud ← plt.Band ch‿price‿plt.axisPrimary‿"Cloud"‿21‿x‿senkouA‿senkouB
```

Alpha support depends on the PLplot driver. Cairo-backed and vector devices are the best options for transparency:

- `pngcairo`
- `svgcairo`
- `pdfcairo`
- `xcairo`

Non-alpha drivers may render transparent colors as opaque.
