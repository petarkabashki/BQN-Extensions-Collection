# BQN Extensions Collection

This repository contains a collection of native extensions and bridges for [BQN](https://mlochbaum.github.io/BQN/), specifically designed for use with [CBQN](https://github.com/dzaima/CBQN) via its `•FFI` interface.

## Projects

### [bqn-duckdb-priceio-bridge-v5](bqn-duckdb-priceio-bridge-v5)
A high-performance bridge for reading OHLCV (Open, High, Low, Close, Volume) market data from Parquet files using DuckDB.

- **Features**:
  - Efficiently queries local Parquet files via DuckDB's C API.
  - Automatically handles various timestamp formats (milliseconds, seconds, DuckDB timestamps, etc.).
  - Provides a clean BQN API (`priceio.bqn`) for data ingestion.
  - Supports range filtering directly in the C/DuckDB layer.
- **Dependencies**: DuckDB (fetched via Makefile), `build-essential`.

### [bqn-plplot-bridge](bqn-plplot-bridge)
A native bridge to the [PLplot](http://plplot.org/) scientific plotting library, optimized for financial and strategy visualization.

- **Features**:
  - Supports multiple export formats: PNG, SVG, PDF (via Cairo drivers).
  - Interactive window support (X11, Qt, Cairo).
  - Specialized financial charting: Candlestick plots, multi-panel layouts, secondary axes.
  - Rich styling: Custom RGBA palettes, line styles, horizontal/vertical reference lines, and background spans for trade entries/exits.
- **Dependencies**: `libplplot-dev`, PLplot drivers (Cairo, Xwin, etc.).

## Getting Started

Each project contains its own `Makefile` and `README.md` with specific build and usage instructions. Generally, you will need `build-essential` and the relevant library headers installed on your system.

### Common Requirements
- **CBQN**: The extensions are designed for the `•FFI` interface of CBQN.
- **C Compiler**: GCC or Clang for building the shared libraries (`.so`).

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Repository Structure

```text
.
├── bqn-duckdb-priceio-bridge-v5/  # DuckDB & Parquet bridge
│   ├── bqn/                       # BQN wrappers and API
│   ├── include/                   # C headers
│   └── src/                       # C implementation
└── bqn-plplot-bridge/             # PLplot charting bridge
    ├── bqn/                       # BQN charting API
    ├── include/                   # C headers
    └── src/                       # C implementation
```
