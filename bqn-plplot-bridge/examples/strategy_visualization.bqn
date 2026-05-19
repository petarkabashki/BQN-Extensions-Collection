# Comprehensive synthetic strategy visualization.
# Run from repository root after: make
#   /path/to/BQN examples/strategy_visualization.bqn

plt ← •Import "../bqn/plplot.bqn"

n ← 120
x ← ↕n
zig ← 1 - 2 | x
regime ← 1 - 2 | ⌊x÷14
trend ← 100 + 0.10 × x
close ← trend + 2.0×regime + 0.5×zig
open ← close - 0.7×zig
high ← 1.4 + open⌈close
low ← (open⌊close) - 1.2
volume ← 1500 + 12×x + 260×|regime

emaFast ← close + 0.35×zig
emaSlow ← close - 1.25
atr ← 1.5 + 0.03×x

tenkan ← close + 0.6×zig
kijun ← close - 0.6×zig
senkouA ← close + 1.1
senkouB ← close - 1.4

rsi ← 50 + 22 × 1 - 2 | ⌊x÷3
macd ← emaFast - emaSlow
signal ← 0.75 × macd
hist ← macd - signal

enX ← 18‿52‿88
exX ← 35‿74‿110

ch ← plt.New 1700‿1150
plt.SetOutput ch‿"pngcairo"‿"build/strategy_visualization.png"
plt.SetTitle ch‿"BQN → PLplot strategy visualization"

# RGBA palette: use slots >= 20 for custom colors/transparency.
plt.PaletteRGBA ch‿20‿80‿220‿120‿0.22   # in-position green spans
plt.PaletteRGBA ch‿21‿80‿180‿255‿0.22   # Ichimoku cloud fill
plt.PaletteRGBA ch‿22‿255‿180‿40‿0.85   # support/resistance annotations
plt.PaletteRGBA ch‿23‿255‿255‿255‿0.35  # subtle session lines
plt.PaletteRGBA ch‿24‿255‿255‿255‿0.16  # transparent histogram

price ← plt.Panel ch‿4.5‿"Price"‿"ATR"
vol ← plt.Panel ch‿1.0‿"Volume"‿""
rsiP ← plt.Panel ch‿1.3‿"RSI"‿""
macdP ← plt.Panel ch‿1.4‿"MACD"‿""

# Main chart: spans, cloud, candles, overlays, annotations, entries/exits.
inpos ← plt.BackgroundSpans ch‿price‿20‿enX‿exX
cloud ← plt.Band ch‿price‿plt.axisPrimary‿"Ichimoku cloud"‿21‿x‿senkouA‿senkouB
cand ← plt.Candles ch‿price‿plt.axisPrimary‿"OHLC"‿3‿2‿x‿open‿high‿low‿close
fast ← plt.Line ch‿price‿plt.axisPrimary‿"EMA fast"‿4‿x‿emaFast
slow ← plt.Line ch‿price‿plt.axisPrimary‿"EMA slow"‿5‿x‿emaSlow
ten ← plt.Line ch‿price‿plt.axisPrimary‿"Tenkan"‿6‿x‿tenkan
kij ← plt.Line ch‿price‿plt.axisPrimary‿"Kijun"‿7‿x‿kijun
atrL ← plt.Line ch‿price‿plt.axisSecondary‿"ATR"‿8‿x‿atr
sup ← plt.HLine ch‿price‿plt.axisPrimary‿"Support"‿22‿101.5
res ← plt.HLine ch‿price‿plt.axisPrimary‿"Resistance"‿22‿111.5
trendLine ← plt.Segment ch‿price‿plt.axisPrimary‿"Trend segment"‿22‿12‿100‿115‿114
sessionA ← plt.VLine ch‿price‿plt.axisPrimary‿"Session"‿23‿40
sessionB ← plt.VLine ch‿price‿plt.axisPrimary‿"Session"‿23‿80
entries ← plt.Markers ch‿price‿plt.axisPrimary‿"Entries"‿3‿"▲"‿enX‿(enX⊏close)
exits ← plt.Markers ch‿price‿plt.axisPrimary‿"Exits"‿2‿"▼"‿exX‿(exX⊏close)

plt.Style ch‿fast‿plt.styleSolid‿2.0
plt.Style ch‿slow‿plt.styleDash‿2.2
plt.Style ch‿ten‿plt.styleDot‿1.5
plt.Style ch‿kij‿plt.styleDashDot‿1.5
plt.Style ch‿atrL‿plt.styleDash‿1.4
plt.Style ch‿sup‿plt.styleLongDash‿2.0
plt.Style ch‿res‿plt.styleLongDash‿2.0
plt.Style ch‿trendLine‿plt.styleDashDot‿2.5
plt.Style ch‿sessionA‿plt.styleDot‿1.0
plt.Style ch‿sessionB‿plt.styleDot‿1.0
plt.SeriesRGBA ch‿atrL‿plt.colorPrimary‿255‿255‿255‿0.55

# Volume panel.
volBars ← plt.VolumeBars ch‿vol‿plt.axisPrimary‿"Volume"‿8‿x‿volume
plt.SeriesRGBA ch‿volBars‿plt.colorPrimary‿190‿80‿80‿0.75

# RSI subplot with threshold lines.
rsiLine ← plt.Line ch‿rsiP‿plt.axisPrimary‿"RSI"‿4‿x‿rsi
rsi30 ← plt.HLine ch‿rsiP‿plt.axisPrimary‿"RSI 30"‿22‿30
rsi70 ← plt.HLine ch‿rsiP‿plt.axisPrimary‿"RSI 70"‿22‿70
plt.Style ch‿rsiLine‿plt.styleSolid‿1.7
plt.Style ch‿rsi30‿plt.styleDash‿1.4
plt.Style ch‿rsi70‿plt.styleDash‿1.4

# MACD subplot: histogram + line + signal.
macdHist ← plt.Histogram ch‿macdP‿plt.axisPrimary‿"MACD hist"‿24‿x‿hist
macdLine ← plt.Line ch‿macdP‿plt.axisPrimary‿"MACD"‿4‿x‿macd
sigLine ← plt.Line ch‿macdP‿plt.axisPrimary‿"Signal"‿5‿x‿signal
zero ← plt.HLine ch‿macdP‿plt.axisPrimary‿"Zero"‿23‿0
plt.Style ch‿macdLine‿plt.styleSolid‿1.8
plt.Style ch‿sigLine‿plt.styleDash‿1.6
plt.Style ch‿zero‿plt.styleDot‿1.2

plt.Render ch
plt.Free ch
"wrote build/strategy_visualization.png"
