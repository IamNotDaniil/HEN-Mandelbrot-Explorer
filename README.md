# HEN-ME — Mandelbrot Explorer

HEN-ME is a C++17 fractal renderer that is being developed **step by step**:

1. choose the main stack;
2. implement a CPU MVP with image export;
3. add OpenMP and performance measurements;
4. only after that move to GUI and CUDA.

This repository is still intentionally focused on the **CPU stage**. The current iteration strengthens that stage with:

- modular CPU renderer components;
- support for both **Mandelbrot** and **Julia set** rendering;
- palette presets;
- viewport presets;
- benchmark image generation;
- benchmark CSV and JSON export;
- interior-point skipping for Mandelbrot main cardioid and period-2 bulb;
- configurable thread counts for render and benchmark runs, including benchmark thread sweeps.

## Implemented now

- C++17 + CMake project structure;
- fractal rendering to `PPM (P6)` and `BMP`;
- supported fractals: `mandelbrot`, `julia`;
- smooth coloring;
- optional OpenMP acceleration;
- palette presets: `classic`, `fire`, `ice`, `grayscale`;
- viewport presets: `default`, `seahorse`, `elephant`, `spiral`;
- benchmark mode with table output plus `benchmark_results.csv` and `benchmark_results.json` export;
- interior tests for faster skipping of obvious Mandelbrot set-members;
- `--threads N` control for repeatable CPU comparisons;
- `--thread-sweep a,b,c` benchmark mode for comparing several thread counts in one run;
- configurable Julia constant through `--julia-c real imag`.

## Build

```bash
cmake -S . -B build
cmake --build build -j
```

## Run

### Default render

```bash
./build/hen_me
```

### Mandelbrot render

```bash
./build/hen_me render --fractal mandelbrot --palette fire --preset seahorse --threads 8 1920 1080 1000 output/mandelbrot.bmp
```

### Julia render

```bash
./build/hen_me render --fractal julia --julia-c -0.8 0.156 --palette ice 1920 1080 1000 output/julia.bmp
```

### Benchmark mode

```bash
./build/hen_me benchmark
./build/hen_me benchmark --fractal julia --julia-c -0.4 0.6 --palette ice --threads 4 output/benchmarks
./build/hen_me benchmark --fractal julia --julia-c -0.4 0.6 --thread-sweep 1,2,4,8 output/benchmarks
```

## CLI

```text
hen_me
hen_me render [--fractal type] [--julia-c real imag] [--palette name] [--preset name] [--threads n] [width height max_iter output_path [center_x center_y scale]]
hen_me benchmark [--fractal type] [--julia-c real imag] [--palette name] [--preset name] [--threads n] [--thread-sweep a,b,c] [output_dir]
```

## Current architecture

- `RenderConfig` / `RenderStats` / `BenchmarkResult` — shared configuration, fractal parameters, thread settings and metrics.
- `ImageBuffer` — pixel storage and export to PPM/BMP.
- `ColorMapper` — palette-aware smooth-iteration to RGB mapping.
- `CpuRenderer` — Mandelbrot and Julia calculations on CPU with optional OpenMP.
- `Cli` — command parsing, palette/preset/fractal support and benchmark preset generation.
- `main.cpp` — render/benchmark orchestration and CSV/JSON report writing.

## Benchmark output

Benchmark mode now produces:

- rendered `.ppm` or `.bmp` files depending on selected extensions, with unique filenames per benchmark run/sweep entry;
- a CSV report at `output/benchmarks/benchmark_results.csv`;
- a JSON report at `output/benchmarks/benchmark_results.json`;
- metrics for fractal type, Julia parameters, requested threads, actual threads used and skipped interior points.

## Testing

The project includes automated smoke tests for both main CPU workflows:

```bash
ctest --test-dir build --output-on-failure
```

The tests verify:

- render mode can generate a valid Julia BMP image;
- benchmark mode creates the benchmark images plus `benchmark_results.csv` and `benchmark_results.json`;
- CLI overrides such as `--fractal`, `--julia-c`, `--palette`, `--threads` and `--thread-sweep` are reflected in outputs.

## Why this still follows the plan

This work **does not jump ahead** to GUI or CUDA. It improves the CPU milestone by adding another real fractal, making the renderer more portfolio-worthy before moving to GPU or UI layers.

## Next steps

1. add SIMD optimization for the CPU path;
2. add automated benchmark sweeps built on top of the current CSV/JSON reports;
3. add PNG export;
4. only then move to GUI mode and CUDA backend.
