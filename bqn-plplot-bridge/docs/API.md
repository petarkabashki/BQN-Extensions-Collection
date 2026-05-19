# BQN wrapper API

Import the wrapper:

```bqn
plt ← •Import "bqn/plplot.bqn"
```

The wrapper is intentionally thin. Most functions return either a panel id or a series id. Store the series id if you want to style the layer after creation.

## Constants

```bqn
plt.axisPrimary
plt.axisSecondary

plt.colorPrimary
plt.colorSecondary

plt.styleSolid
plt.styleDash
plt.styleDot
plt.styleDashDot
plt.styleLongDash
```

`colorPrimary` and `colorSecondary` select which color to update in `SeriesColor` / `SeriesRGBA`. Secondary color is used by candles and OHLC bars for down bars.

## Lifecycle

```bqn
ch ← plt.New width‿height
plt.Free ch
```

`width` and `height` are pixel dimensions passed to PLplot through `plspage`.

## Output

```bqn
plt.SetOutput ch‿"pngcairo"‿"build/chart.png"
plt.SetOutput ch‿"svgcairo"‿"build/chart.svg"
plt.SetOutput ch‿"pdfcairo"‿"build/chart.pdf"
plt.Render ch

plt.Show ch
plt.ShowDevice ch‿"xcairo"
```

`Render` writes to the configured device/file. `Show` temporarily switches the device to `xwin`. `ShowDevice` lets you choose another interactive driver.

## Panels

```bqn
price ← plt.Panel ch‿4‿"Price"‿"ATR"
volume ← plt.Panel ch‿1‿"Volume"‿""
rsi ← plt.Panel ch‿1.2‿"RSI"‿""
```

The panel weight controls relative vertical height. The fourth argument is the secondary y-axis label. Leave it empty when unused.

## Palette and per-series styling

```bqn
plt.PaletteRGBA ch‿20‿80‿220‿120‿0.22
plt.PaletteRGBA ch‿21‿255‿180‿40‿0.80

s ← plt.Line ch‿price‿plt.axisPrimary‿"EMA"‿4‿x‿ema
plt.Style ch‿s‿plt.styleDash‿2.5
plt.SeriesRGBA ch‿s‿plt.colorPrimary‿255‿255‿255‿0.65
```

`PaletteRGBA` changes a PLplot color-map0 slot. Use indexes `16` and above for custom colors, because low indexes are commonly used by PLplot defaults.

`SeriesRGBA` overrides the selected series color with an explicit RGBA value. It is useful for one-off transparency.

`SeriesColor` can switch a series back to palette-index mode:

```bqn
plt.SeriesColor ch‿s‿plt.colorPrimary‿4
```

## Price layers

### Line

```bqn
sid ← plt.Line ch‿panel‿plt.axisPrimary‿"Close"‿4‿x‿close
```

### Candles

```bqn
sid ← plt.Candles ch‿panel‿plt.axisPrimary‿"BTC"‿3‿2‿x‿open‿high‿low‿close
```

The two color arguments are up color and down color.

### OHLC bars

```bqn
sid ← plt.OHLCBars ch‿panel‿plt.axisPrimary‿"BTC"‿3‿2‿x‿open‿high‿low‿close
```

## Overlays and annotations

### Indicator line

```bqn
emaSid ← plt.Line ch‿price‿plt.axisPrimary‿"EMA 20"‿4‿x‿ema20
plt.Style ch‿emaSid‿plt.styleDash‿2.2
```

### Filled band / cloud

```bqn
cloud ← plt.Band ch‿price‿plt.axisPrimary‿"Cloud"‿21‿x‿senkouA‿senkouB
```

Use a transparent palette color or `SeriesRGBA` for band fills.

### Entry/exit markers

```bqn
plt.Markers ch‿price‿plt.axisPrimary‿"Entries"‿3‿"▲"‿entryX‿(entryX⊏close)
plt.Markers ch‿price‿plt.axisPrimary‿"Exits"‿2‿"▼"‿exitX‿(exitX⊏close)
```

### Background spans

```bqn
plt.BackgroundSpans ch‿price‿20‿entryX‿exitX
```

Each pair `x0[i]` / `x1[i]` creates one shaded region across the full primary-axis y-range of the panel.

### Horizontal lines

```bqn
support ← plt.HLine ch‿price‿plt.axisPrimary‿"Support"‿21‿63450
plt.Style ch‿support‿plt.styleLongDash‿2.0
```

HLine spans the visible x-range of the panel.

### Vertical lines

```bqn
split ← plt.VLine ch‿price‿plt.axisPrimary‿"WFO split"‿21‿80
plt.Style ch‿split‿plt.styleDot‿1.3
```

VLine spans the visible y-range of the selected axis.

### Segments

```bqn
trend ← plt.Segment ch‿price‿plt.axisPrimary‿"Trend"‿22‿10‿63000‿80‿69000
plt.Style ch‿trend‿plt.styleDashDot‿2.5
```

Segment is a BQN convenience wrapper implemented as a two-point `Line`.

## Subplots

### Volume bars

```bqn
plt.VolumeBars ch‿volPanel‿plt.axisPrimary‿"Volume"‿8‿x‿volume
```

### Histogram

```bqn
hist ← plt.Histogram ch‿macdPanel‿plt.axisPrimary‿"MACD hist"‿24‿x‿histValues
```

### Lines and thresholds

```bqn
rsiLine ← plt.Line ch‿rsiPanel‿plt.axisPrimary‿"RSI"‿4‿x‿rsi
rsi30 ← plt.HLine ch‿rsiPanel‿plt.axisPrimary‿"RSI 30"‿21‿30
rsi70 ← plt.HLine ch‿rsiPanel‿plt.axisPrimary‿"RSI 70"‿21‿70
```

## Secondary axes

Any line-like layer can be attached to `axisSecondary`.

```bqn
price ← plt.Panel ch‿4‿"Price"‿"ATR"
atr ← plt.Line ch‿price‿plt.axisSecondary‿"ATR"‿8‿x‿atrValues
```

The bridge computes primary and secondary y-ranges independently.
