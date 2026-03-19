#include "henme/Cli.h"
#include "henme/CpuRenderer.h"
#include "henme/ImageBuffer.h"
#include "henme/ColorMapper.h"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

namespace henme {
namespace {

std::string json_escape(const std::string& value) {
    std::ostringstream escaped;
    for (const char ch : value) {
        switch (ch) {
            case '\\': escaped << "\\\\"; break;
            case '"': escaped << "\\\""; break;
            case '\n': escaped << "\\n"; break;
            case '\r': escaped << "\\r"; break;
            case '\t': escaped << "\\t"; break;
            default: escaped << ch; break;
        }
    }
    return escaped.str();
}

std::string benchmark_output_path_for(const RenderConfig& config, const std::string& preset_name) {
    const std::filesystem::path original_path(config.output_path);
    const std::string extension = original_path.extension().string();
    const std::string stem = original_path.stem().string();
    const std::string fractal_suffix = fractal_name(config.fractal);
    const std::string thread_suffix = config.thread_count > 0 ? std::to_string(config.thread_count) : std::string("auto");
    const std::filesystem::path parent = original_path.parent_path();
    return (parent / (stem + "_" + fractal_suffix + "_t" + thread_suffix + "_" + preset_name + extension)).string();
}

void print_render_summary(const RenderConfig& config, const RenderStats& stats) {
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "HEN-ME CPU fractal render complete\n";
    std::cout << "Fractal: " << fractal_name(config.fractal) << '\n';
    if (config.fractal == FractalKind::Julia) {
        std::cout << "Julia c: (" << config.julia_c_real << ", " << config.julia_c_imag << ")\n";
    }
    std::cout << "Resolution: " << config.width << "x" << config.height << '\n';
    std::cout << "Max iterations: " << config.max_iterations << '\n';
    std::cout << "Center: (" << config.center_x << ", " << config.center_y << ")\n";
    std::cout << "Scale: " << config.scale << '\n';
    std::cout << "Palette: " << palette_name(config.palette) << '\n';
    std::cout << "Viewport preset: " << config.viewport_name << '\n';
    std::cout << "Threads requested: " << (config.thread_count > 0 ? std::to_string(config.thread_count) : std::string("auto")) << '\n';
    std::cout << "Threads used: " << stats.threads_used << '\n';
    std::cout << "Interior points skipped: " << stats.interior_points_skipped << '\n';
    std::cout << "Render time: " << stats.elapsed_ms << " ms\n";
    std::cout << "Throughput: " << stats.megapixels_per_second << " Mpx/s\n";
    std::cout << "Saved image: " << config.output_path << '\n';
}

bool write_benchmark_csv(const std::string& output_dir, const std::vector<BenchmarkResult>& results) {
    std::error_code error;
    std::filesystem::create_directories(output_dir, error);
    if (error) {
        return false;
    }

    const std::filesystem::path csv_path = std::filesystem::path(output_dir) / "benchmark_results.csv";
    std::ofstream csv(csv_path);
    if (!csv) {
        return false;
    }

    csv << "preset,fractal,resolution,max_iterations,palette,viewport,julia_c_real,julia_c_imag,threads_requested,threads_used,interior_points_skipped,time_ms,mpx_per_s,output\n";
    for (const auto& result : results) {
        csv << result.preset_name << ','
            << fractal_name(result.config.fractal) << ','
            << result.config.width << 'x' << result.config.height << ','
            << result.config.max_iterations << ','
            << palette_name(result.config.palette) << ','
            << result.config.viewport_name << ','
            << result.config.julia_c_real << ','
            << result.config.julia_c_imag << ','
            << (result.config.thread_count > 0 ? std::to_string(result.config.thread_count) : std::string("auto")) << ','
            << result.stats.threads_used << ','
            << result.stats.interior_points_skipped << ','
            << std::fixed << std::setprecision(2) << result.stats.elapsed_ms << ','
            << std::fixed << std::setprecision(2) << result.stats.megapixels_per_second << ','
            << result.config.output_path << '\n';
    }

    std::cout << "Benchmark CSV: " << csv_path.string() << '\n';
    return true;
}

bool write_benchmark_json(const std::string& output_dir, const std::vector<BenchmarkResult>& results) {
    std::error_code error;
    std::filesystem::create_directories(output_dir, error);
    if (error) {
        return false;
    }

    const std::filesystem::path json_path = std::filesystem::path(output_dir) / "benchmark_results.json";
    std::ofstream json(json_path);
    if (!json) {
        return false;
    }

    json << "{\n  \"results\": [\n";
    for (std::size_t index = 0; index < results.size(); ++index) {
        const auto& result = results[index];
        json << "    {\n"
             << "      \"preset\": \"" << json_escape(result.preset_name) << "\",\n"
             << "      \"fractal\": \"" << json_escape(fractal_name(result.config.fractal)) << "\",\n"
             << "      \"resolution\": \"" << result.config.width << 'x' << result.config.height << "\",\n"
             << "      \"max_iterations\": " << result.config.max_iterations << ",\n"
             << "      \"palette\": \"" << json_escape(palette_name(result.config.palette)) << "\",\n"
             << "      \"viewport\": \"" << json_escape(result.config.viewport_name) << "\",\n"
             << "      \"julia_c_real\": " << result.config.julia_c_real << ",\n"
             << "      \"julia_c_imag\": " << result.config.julia_c_imag << ",\n"
             << "      \"threads_requested\": \"" << json_escape(result.config.thread_count > 0 ? std::to_string(result.config.thread_count) : std::string("auto")) << "\",\n"
             << "      \"threads_used\": " << result.stats.threads_used << ",\n"
             << "      \"interior_points_skipped\": " << result.stats.interior_points_skipped << ",\n"
             << "      \"time_ms\": " << std::fixed << std::setprecision(2) << result.stats.elapsed_ms << ",\n"
             << "      \"mpx_per_s\": " << std::fixed << std::setprecision(2) << result.stats.megapixels_per_second << ",\n"
             << "      \"output\": \"" << json_escape(result.config.output_path) << "\"\n"
             << "    }";
        if (index + 1 != results.size()) {
            json << ',';
        }
        json << '\n';
    }
    json << "  ]\n}\n";

    std::cout << "Benchmark JSON: " << json_path.string() << '\n';
    return true;
}

int run_render(const RenderConfig& config) {
    ImageBuffer image(config.width, config.height);
    CpuRenderer renderer;
    const RenderStats stats = renderer.render(config, image);

    if (!image.save(config.output_path)) {
        std::cerr << "Failed to save image to: " << config.output_path << '\n';
        return 2;
    }

    print_render_summary(config, stats);
    return 0;
}

int run_benchmark(const CommandLineOptions& options) {
    CpuRenderer renderer;
    auto base_configs = make_benchmark_configs(options.benchmark_output_dir);
    std::vector<RenderConfig> configs;
    for (auto& config : base_configs) {
        if (options.render_config.palette != PaletteKind::Classic) {
            config.palette = options.render_config.palette;
        }
        config.fractal = options.render_config.fractal;
        config.julia_c_real = options.render_config.julia_c_real;
        config.julia_c_imag = options.render_config.julia_c_imag;
        if (options.render_config.viewport_name != "default") {
            config.center_x = options.render_config.center_x;
            config.center_y = options.render_config.center_y;
            config.scale = options.render_config.scale;
            config.viewport_name = options.render_config.viewport_name;
        }
        if (!options.benchmark_thread_sweep.empty()) {
            for (const int thread_count : options.benchmark_thread_sweep) {
                RenderConfig sweep_config = config;
                sweep_config.thread_count = thread_count;
                configs.push_back(sweep_config);
            }
        } else {
            if (options.render_config.thread_count > 0) {
                config.thread_count = options.render_config.thread_count;
            }
            configs.push_back(config);
        }
    }

    std::vector<BenchmarkResult> results;
    std::cout << "Running CPU benchmark suite...\n";
    std::cout << std::left
              << std::setw(18) << "preset"
              << std::setw(12) << "fractal"
              << std::setw(14) << "resolution"
              << std::setw(12) << "iterations"
              << std::setw(12) << "palette"
              << std::setw(12) << "threads"
              << std::setw(16) << "skipped"
              << std::setw(14) << "time_ms"
              << std::setw(14) << "Mpx/s"
              << "output\n";

    for (std::size_t index = 0; index < configs.size(); ++index) {
        const RenderConfig& config = configs[index];
        const std::string preset_name = "preset_" + std::to_string(index + 1) + "_t" + (config.thread_count > 0 ? std::to_string(config.thread_count) : std::string("auto"));
        RenderConfig run_config = config;
        run_config.output_path = benchmark_output_path_for(run_config, preset_name);

        ImageBuffer image(run_config.width, run_config.height);
        const RenderStats stats = renderer.render(run_config, image);

        if (!image.save(run_config.output_path)) {
            std::cerr << "Failed to save benchmark image to: " << run_config.output_path << '\n';
            return 3;
        }

        const std::string resolution = std::to_string(run_config.width) + "x" + std::to_string(run_config.height);
        std::cout << std::left
                  << std::setw(18) << preset_name
                  << std::setw(12) << fractal_name(config.fractal)
                  << std::setw(14) << resolution
                  << std::setw(12) << run_config.max_iterations
                  << std::setw(12) << palette_name(run_config.palette)
                  << std::setw(12) << (run_config.thread_count > 0 ? std::to_string(run_config.thread_count) : std::string("auto"))
                  << std::setw(16) << stats.interior_points_skipped
                  << std::setw(14) << std::fixed << std::setprecision(2) << stats.elapsed_ms
                  << std::setw(14) << std::fixed << std::setprecision(2) << stats.megapixels_per_second
                  << run_config.output_path << '\n';

        results.push_back(BenchmarkResult{preset_name, run_config, stats});
    }

    if (!write_benchmark_csv(options.benchmark_output_dir, results)) {
        std::cerr << "Failed to write benchmark CSV report\n";
        return 4;
    }
    if (!write_benchmark_json(options.benchmark_output_dir, results)) {
        std::cerr << "Failed to write benchmark JSON report\n";
        return 5;
    }

    return 0;
}

} // namespace
} // namespace henme

int main(int argc, char** argv) {
    henme::CommandLineOptions options;
    if (!henme::parse_args(argc, argv, options)) {
        henme::print_usage(argv[0]);
        return 1;
    }

    if (options.mode == henme::CommandMode::Benchmark) {
        return henme::run_benchmark(options);
    }

    return henme::run_render(options.render_config);
}
