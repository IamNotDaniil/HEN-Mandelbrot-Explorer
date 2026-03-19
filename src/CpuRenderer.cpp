#include "henme/CpuRenderer.h"
#include "henme/ColorMapper.h"
#include "henme/Cli.h"

#include <algorithm>
#include <chrono>
#include <cmath>

#ifdef HENME_HAS_OPENMP
#include <omp.h>
#endif

namespace henme {
namespace {

bool is_in_main_cardioid_or_period2_bulb(double cr, double ci) {
    const double x_minus_quarter = cr - 0.25;
    const double q = x_minus_quarter * x_minus_quarter + ci * ci;
    const bool in_main_cardioid = q * (q + x_minus_quarter) <= 0.25 * ci * ci;

    const double x_plus_one = cr + 1.0;
    const bool in_period2_bulb = x_plus_one * x_plus_one + ci * ci <= 0.0625;
    return in_main_cardioid || in_period2_bulb;
}

} // namespace

RenderStats CpuRenderer::render(const RenderConfig& config, ImageBuffer& image) const {
    const auto start = std::chrono::high_resolution_clock::now();
    const double aspect_ratio = static_cast<double>(config.width) / static_cast<double>(config.height);
    const double x_span = config.scale * aspect_ratio;
    const double y_span = config.scale;
    const double x_min = config.center_x - x_span * 0.5;
    const double y_max = config.center_y + y_span * 0.5;
    const double dx = x_span / static_cast<double>(config.width);
    const double dy = y_span / static_cast<double>(config.height);
    const ColorMapper color_mapper;

    int threads_used = 1;
    std::size_t interior_points_skipped = 0;
#ifdef HENME_HAS_OPENMP
    if (config.thread_count > 0) {
        omp_set_num_threads(config.thread_count);
    }
    threads_used = config.thread_count > 0 ? config.thread_count : omp_get_max_threads();
#pragma omp parallel for schedule(dynamic) reduction(+ : interior_points_skipped)
#endif
    for (int y = 0; y < config.height; ++y) {
        for (int x = 0; x < config.width; ++x) {
            const double plane_x = x_min + static_cast<double>(x) * dx;
            const double plane_y = y_max - static_cast<double>(y) * dy;

            double zr = 0.0;
            double zi = 0.0;
            double cr = plane_x;
            double ci = plane_y;

            if (config.fractal == FractalKind::Mandelbrot) {
                if (is_in_main_cardioid_or_period2_bulb(cr, ci)) {
                    image.at(x, y) = Pixel{0, 0, 0};
                    ++interior_points_skipped;
                    continue;
                }
            } else {
                zr = plane_x;
                zi = plane_y;
                cr = config.julia_c_real;
                ci = config.julia_c_imag;
            }

            double magnitude_squared = zr * zr + zi * zi;
            int iteration = 0;

            while (iteration < config.max_iterations && magnitude_squared <= 4.0) {
                const double zr2 = zr * zr;
                const double zi2 = zi * zi;
                zi = 2.0 * zr * zi + ci;
                zr = zr2 - zi2 + cr;
                magnitude_squared = zr * zr + zi * zi;
                ++iteration;
            }

            double smooth_iteration = static_cast<double>(iteration);
            if (iteration < config.max_iterations) {
                const double magnitude = std::sqrt(magnitude_squared);
                if (magnitude > 0.0) {
                    smooth_iteration = static_cast<double>(iteration) + 1.0 - std::log2(std::log2(std::max(magnitude, 1.0000001)));
                }
            }

            image.at(x, y) = color_mapper.map(smooth_iteration, config.max_iterations, config.palette);
        }
    }

    const auto end = std::chrono::high_resolution_clock::now();
    const double elapsed_ms = std::chrono::duration<double, std::milli>(end - start).count();
    const double total_pixels = static_cast<double>(config.width) * static_cast<double>(config.height);
    const double megapixels_per_second = elapsed_ms > 0.0 ? (total_pixels / 1'000'000.0) / (elapsed_ms / 1000.0) : 0.0;

    return RenderStats{elapsed_ms, megapixels_per_second, threads_used, interior_points_skipped};
}

} // namespace henme
