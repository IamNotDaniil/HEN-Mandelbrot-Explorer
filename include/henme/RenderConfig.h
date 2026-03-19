#pragma once

#include <string>

namespace henme {

enum class PaletteKind {
    Classic,
    Fire,
    Ice,
    Grayscale,
};

enum class FractalKind {
    Mandelbrot,
    Julia,
};

struct RenderConfig {
    int width = 1280;
    int height = 720;
    int max_iterations = 500;
    double center_x = -0.75;
    double center_y = 0.0;
    double scale = 3.0;
    std::string output_path = "output/mandelbrot.ppm";
    PaletteKind palette = PaletteKind::Classic;
    FractalKind fractal = FractalKind::Mandelbrot;
    std::string viewport_name = "default";
    int thread_count = 0;
    double julia_c_real = -0.8;
    double julia_c_imag = 0.156;
};

struct RenderStats {
    double elapsed_ms = 0.0;
    double megapixels_per_second = 0.0;
    int threads_used = 1;
    std::size_t interior_points_skipped = 0;
};

struct BenchmarkResult {
    std::string preset_name;
    RenderConfig config;
    RenderStats stats;
};

} // namespace henme
