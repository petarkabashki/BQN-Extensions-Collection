# Examples

Run all examples from repository root after building the shared library:

```bash
make
```

## 1. OHLCV smoke chart

```bash
/mnt/data/BQN examples/smoke.bqn
xdg-open build/smoke.png
```

Demonstrates:

- candlestick chart
- EMA-like overlays
- entry/exit markers
- background spans
- volume subplot
- RSI subplot
- MACD histogram subplot

## 2. Popup smoke chart

```bash
/mnt/data/BQN examples/smoke_popup.bqn
```

Demonstrates:

- interactive popup device through `plt.Show`
- same chart model rendered to a GUI-like PLplot device

Try alternative drivers:

```bqn
plt.ShowDevice ch‿"xcairo"
plt.ShowDevice ch‿"qtwidget"
```

## 3. Styling smoke chart

```bash
/mnt/data/BQN examples/styling.bqn
xdg-open build/styling.png
```

Demonstrates:

- `PaletteRGBA`
- `SeriesRGBA`
- `Style`
- dashed/dotted/dash-dot lines
- line widths
- horizontal lines
- vertical lines
- arbitrary line segments
- transparent background spans

## 4. Strategy visualization

```bash
/mnt/data/BQN examples/strategy_visualization.bqn
xdg-open build/strategy_visualization.png
```

Demonstrates a more realistic trading strategy view:

- candlestick price chart
- in-position background spans
- Ichimoku-like cloud band
- fast/slow EMA overlays
- Tenkan/Kijun overlays
- ATR on secondary y-axis
- support/resistance levels
- session/walk-forward split lines
- trend segment
- entry and exit markers
- volume bars
- RSI with 30/70 thresholds
- MACD line, signal line, histogram, and zero line

## Output file summary

| Example | Output |
|---|---|
| `smoke.bqn` | `build/smoke.png` |
| `smoke_popup.bqn` | interactive window |
| `styling.bqn` | `build/styling.png` |
| `strategy_visualization.bqn` | `build/strategy_visualization.png` |

## Pattern for your own examples

```bqn
plt ← •Import "../bqn/plplot.bqn"

ch ← plt.New 1600‿1000
plt.SetOutput ch‿"pngcairo"‿"build/my_chart.png"
plt.SetTitle ch‿"My chart"

price ← plt.Panel ch‿4‿"Price"‿""
sub ← plt.Panel ch‿1.2‿"Indicator"‿""

main ← plt.Candles ch‿price‿plt.axisPrimary‿"OHLC"‿3‿2‿x‿open‿high‿low‿close
line ← plt.Line ch‿price‿plt.axisPrimary‿"EMA"‿4‿x‿ema
threshold ← plt.HLine ch‿sub‿plt.axisPrimary‿"Threshold"‿21‿0

plt.Style ch‿line‿plt.styleDash‿2
plt.Render ch
plt.Free ch
```
