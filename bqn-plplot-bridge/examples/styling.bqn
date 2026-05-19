# Run from repository root after: make
#   /path/to/BQN examples/styling.bqn

plt ← •Import "../bqn/plplot.bqn"

n ← 60
x ← ↕n
y ← 100 + +` 0.15 × 1 - 2 | x
fast ← y + 0.8 × 1 - 2 | x
slow ← y - 0.8 × 1 - 2 | x

ch ← plt.New 1300‿700
plt.SetOutput ch‿"pngcairo"‿"build/styling.png"
plt.SetTitle ch‿"BQN → PLplot styling smoke chart"
price ← plt.Panel ch‿3‿"Price"‿""

# Palette-level RGBA colors. Use high indexes to avoid common PLplot built-ins.
plt.PaletteRGBA ch‿20‿255‿80‿70‿0.18   # translucent red background
plt.PaletteRGBA ch‿21‿255‿190‿40‿0.55  # orange annotation
plt.PaletteRGBA ch‿22‿80‿220‿255‿0.75  # cyan line

span ← plt.BackgroundSpans ch‿price‿20‿(8‿35)‿(18‿50)
base ← plt.Line ch‿price‿plt.axisPrimary‿"base"‿15‿x‿y
f ← plt.Line ch‿price‿plt.axisPrimary‿"fast dashed"‿3‿x‿fast
s ← plt.Line ch‿price‿plt.axisPrimary‿"slow dotted"‿4‿x‿slow
seg ← plt.Segment ch‿price‿plt.axisPrimary‿"trend segment"‿22‿5‿99‿55‿111
h ← plt.HLine ch‿price‿plt.axisPrimary‿"support"‿21‿101.5
v ← plt.VLine ch‿price‿plt.axisPrimary‿"event"‿21‿30

# Per-series style: PLplot line-style id + width.
plt.Style ch‿f‿plt.styleDash‿2.5
plt.Style ch‿s‿plt.styleDot‿2
plt.Style ch‿seg‿plt.styleDashDot‿3
plt.Style ch‿h‿plt.styleLongDash‿2.5
plt.Style ch‿v‿plt.styleDash‿2.5

# Per-series explicit RGBA. This overrides the palette/index color for the series.
plt.SeriesRGBA ch‿base‿plt.colorPrimary‿245‿245‿245‿1
plt.SeriesRGBA ch‿f‿plt.colorPrimary‿80‿255‿120‿0.9
plt.SeriesRGBA ch‿s‿plt.colorPrimary‿120‿180‿255‿0.9

plt.Render ch
plt.Free ch
"wrote build/styling.png"
