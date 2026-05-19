# Run from repository root after: make
#   /path/to/BQN examples/smoke.bqn

plt ← •Import "../bqn/plplot.bqn"

n ← 80
x ← ↕n
base ← 100 + +` 0.3 × 1 - 2 | x
open ← base + 0.8 × 1 - 2 | x+1
close ← base + 0.8 × 1 - 2 | x+2
high ← 1.5 + open⌈close
low ← (open⌊close) - 1.5
volume ← 1000 + 30 × x
emaFast ← close
emaSlow ← close + 2 × 1 - 2 | x
rsi ← 50 + 20 × 1 - 2 | x
macd ← emaFast - emaSlow

ch ← plt.New 1600‿1000
plt.SetOutput ch‿"pngcairo"‿"build/smoke.png"
plt.SetTitle ch‿"BQN → PLplot OHLCV smoke chart"
price ← plt.Panel ch‿4‿"Price"‿""
vol ← plt.Panel ch‿1‿"Volume"‿""
rsiP ← plt.Panel ch‿1.3‿"RSI"‿""
macdP ← plt.Panel ch‿1.3‿"MACD"‿""

plt.BackgroundSpans ch‿price‿14‿(10‿30‿55)‿(20‿38‿70)
plt.Candles ch‿price‿plt.axisPrimary‿"BTC"‿3‿2‿x‿open‿high‿low‿close
plt.Line ch‿price‿plt.axisPrimary‿"EMA fast"‿4‿x‿emaFast
plt.Line ch‿price‿plt.axisPrimary‿"EMA slow"‿5‿x‿emaSlow
plt.Markers ch‿price‿plt.axisPrimary‿"entries"‿3‿"▲"‿(10‿32‿58)‿(10‿32‿58⊏close)
plt.Markers ch‿price‿plt.axisPrimary‿"exits"‿2‿"▼"‿(20‿45‿72)‿(20‿45‿72⊏close)
plt.VolumeBars ch‿vol‿plt.axisPrimary‿"Volume"‿8‿x‿volume
plt.Line ch‿rsiP‿plt.axisPrimary‿"RSI"‿4‿x‿rsi
plt.Histogram ch‿macdP‿plt.axisPrimary‿"MACD"‿6‿x‿macd

plt.Render ch
plt.Free ch
"wrote build/smoke.png"
