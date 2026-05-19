# CBQN wrapper for libbqnplplot.so.
# Import this file and call exported functions with BQN arrays.
# Run examples from repository root after `make`.

libPath ← "../build/libbqnplplot.so"
CStr ← {(•ToUTF8 𝕩)∾@}
Check ← {𝕊 r: r<0 ? ! "bqn-plplot C call failed; use bqnpl_last_error from C for details" ; 𝕊 r: r}

ffiNew      ← libPath •FFI "*"   ‿"bqnpl_chart_new"           ‿"i32"‿"i32"
ffiFree     ← libPath •FFI ""    ‿"bqnpl_chart_free"          ‿">*"
ffiOutput   ← libPath •FFI "i32" ‿"bqnpl_set_output"          ‿"*"‿"*u8:c8"‿"*u8:c8"
ffiTitle    ← libPath •FFI "i32" ‿"bqnpl_set_title"           ‿"*"‿"*u8:c8"
ffiPanel    ← libPath •FFI "i32" ‿"bqnpl_add_panel"           ‿"*"‿"f64"‿"*u8:c8"‿"*u8:c8"

ffiPalette  ← libPath •FFI "i32" ‿"bqnpl_set_palette_rgba"    ‿"*"‿"i32"‿"i32"‿"i32"‿"i32"‿"f64"
ffiStyle    ← libPath •FFI "i32" ‿"bqnpl_set_series_style"    ‿"*"‿"i32"‿"i32"‿"f64"
ffiSColor   ← libPath •FFI "i32" ‿"bqnpl_set_series_color"    ‿"*"‿"i32"‿"i32"‿"i32"
ffiSRGBA    ← libPath •FFI "i32" ‿"bqnpl_set_series_rgba"     ‿"*"‿"i32"‿"i32"‿"i32"‿"i32"‿"i32"‿"f64"

ffiLine     ← libPath •FFI "i32" ‿"bqnpl_add_line"            ‿"*"‿"i32"‿"i32"‿"*u8:c8"‿"i32"‿"i32"‿"*f64"‿"*f64"
ffiHist     ← libPath •FFI "i32" ‿"bqnpl_add_histogram"       ‿"*"‿"i32"‿"i32"‿"*u8:c8"‿"i32"‿"i32"‿"*f64"‿"*f64"
ffiMarkers  ← libPath •FFI "i32" ‿"bqnpl_add_markers"         ‿"*"‿"i32"‿"i32"‿"*u8:c8"‿"i32"‿"*u8:c8"‿"i32"‿"*f64"‿"*f64"
ffiCandles  ← libPath •FFI "i32" ‿"bqnpl_add_candles"         ‿"*"‿"i32"‿"i32"‿"*u8:c8"‿"i32"‿"i32"‿"i32"‿"*f64"‿"*f64"‿"*f64"‿"*f64"‿"*f64"
ffiOHLC     ← libPath •FFI "i32" ‿"bqnpl_add_ohlc_bars"       ‿"*"‿"i32"‿"i32"‿"*u8:c8"‿"i32"‿"i32"‿"i32"‿"*f64"‿"*f64"‿"*f64"‿"*f64"‿"*f64"
ffiVolume   ← libPath •FFI "i32" ‿"bqnpl_add_volume_bars"     ‿"*"‿"i32"‿"i32"‿"*u8:c8"‿"i32"‿"i32"‿"*f64"‿"*f64"
ffiSpans    ← libPath •FFI "i32" ‿"bqnpl_add_background_spans"‿"*"‿"i32"‿"i32"‿"i32"‿"*f64"‿"*f64"
ffiBand     ← libPath •FFI "i32" ‿"bqnpl_add_band"            ‿"*"‿"i32"‿"i32"‿"*u8:c8"‿"i32"‿"i32"‿"*f64"‿"*f64"‿"*f64"
ffiHLine    ← libPath •FFI "i32" ‿"bqnpl_add_hline"           ‿"*"‿"i32"‿"i32"‿"*u8:c8"‿"i32"‿"f64"
ffiVLine    ← libPath •FFI "i32" ‿"bqnpl_add_vline"           ‿"*"‿"i32"‿"i32"‿"*u8:c8"‿"i32"‿"f64"

ffiRender   ← libPath •FFI "i32" ‿"bqnpl_render"             ‿">*"
ffiShow     ← libPath •FFI "i32" ‿"bqnpl_show"               ‿"*"‿"*u8:c8"

axisPrimary   ⇐ 0
axisSecondary ⇐ 1

colorPrimary   ⇐ 0
colorSecondary ⇐ 1

styleSolid     ⇐ 1
styleDash      ⇐ 2
styleDot       ⇐ 3
styleDashDot   ⇐ 4
styleLongDash  ⇐ 5

# PLplot interactive devices depend on installed driver packages.
# Useful choices on Linux: "xwin", "xcairo", "qtwidget".

New ⇐ {width‿height: FfiNew width‿height}
Free ⇐ {FfiFree 𝕩}
SetOutput ⇐ {ch‿device‿file: Check FfiOutput ⟨ch,CStr device,CStr file⟩}
SetTitle ⇐ {ch‿title: Check FfiTitle ⟨ch,CStr title⟩}
Panel ⇐ {ch‿weight‿ylabel‿y2label: Check FfiPanel ⟨ch,weight,CStr ylabel,CStr y2label⟩}

# Define/redefine a PLplot palette entry. Use indexes >=16 to avoid common built-ins.
PaletteRGBA ⇐ {ch‿idx‿r‿g‿b‿a: Check FfiPalette ⟨ch,idx,r,g,b,a⟩}

# Change line style/width after creating a layer. Works for lines, bands, markers,
# histogram borders/fills, hlines, vlines, OHLC/candlestick wicks and outlines.
Style ⇐ {ch‿series‿lineStyle‿lineWidth: Check FfiStyle ⟨ch,series,lineStyle,lineWidth⟩}
SeriesColor ⇐ {ch‿series‿which‿idx: Check FfiSColor ⟨ch,series,which,idx⟩}
SeriesRGBA ⇐ {ch‿series‿which‿r‿g‿b‿a: Check FfiSRGBA ⟨ch,series,which,r,g,b,a⟩}

Line ⇐ {ch‿panel‿axis‿label‿color‿x‿y:
  Check FfiLine ⟨ch,panel,axis,CStr label,color,≠x,x,y⟩
}
Histogram ⇐ {ch‿panel‿axis‿label‿color‿x‿y:
  Check FfiHist ⟨ch,panel,axis,CStr label,color,≠x,x,y⟩
}
Markers ⇐ {ch‿panel‿axis‿label‿color‿symbol‿x‿y:
  Check FfiMarkers ⟨ch,panel,axis,CStr label,color,CStr symbol,≠x,x,y⟩
}
Candles ⇐ {ch‿panel‿axis‿label‿upColor‿downColor‿x‿open‿high‿low‿close:
  Check FfiCandles ⟨ch,panel,axis,CStr label,upColor,downColor,≠x,x,open,high,low,close⟩
}
OHLCBars ⇐ {ch‿panel‿axis‿label‿upColor‿downColor‿x‿open‿high‿low‿close:
  Check FfiOHLC ⟨ch,panel,axis,CStr label,upColor,downColor,≠x,x,open,high,low,close⟩
}
VolumeBars ⇐ {ch‿panel‿axis‿label‿color‿x‿volume:
  Check FfiVolume ⟨ch,panel,axis,CStr label,color,≠x,x,volume⟩
}
BackgroundSpans ⇐ {ch‿panel‿color‿x0‿x1:
  Check FfiSpans ⟨ch,panel,color,≠x0,x0,x1⟩
}
Band ⇐ {ch‿panel‿axis‿label‿color‿x‿y0‿y1:
  Check FfiBand ⟨ch,panel,axis,CStr label,color,≠x,x,y0,y1⟩
}
HLine ⇐ {ch‿panel‿axis‿label‿color‿y:
  Check FfiHLine ⟨ch,panel,axis,CStr label,color,y⟩
}
VLine ⇐ {ch‿panel‿axis‿label‿color‿x:
  Check FfiVLine ⟨ch,panel,axis,CStr label,color,x⟩
}
Segment ⇐ {ch‿panel‿axis‿label‿color‿x0‿y0‿x1‿y1:
  Line ch‿panel‿axis‿label‿color‿(x0‿x1)‿(y0‿y1)
}

Render ⇐ {Check FfiRender 𝕩}
Show ⇐ {Check FfiShow ⟨𝕩,CStr "xwin"⟩}
ShowDevice ⇐ {ch‿device: Check FfiShow ⟨ch,CStr device⟩}

# Convenience chart constructors for OHLCV + overlays/subplots can be built on top of this namespace.
