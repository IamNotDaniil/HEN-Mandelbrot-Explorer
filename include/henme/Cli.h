#pragma once

#include "henme/RenderConfig.h"

#include <string>
#include <vector>

namespace henme {

enum class CommandMode {
    Render,
    Benchmark,
};

struct CommandLineOptions {
    CommandMode mode = CommandMode::Render;
    RenderConfig render_config;
    std::string benchmark_output_dir = "output/benchmarks";
    std::vector<int> benchmark_thread_sweep;
};

struct ViewportPreset {
    std::string name;
    double center_x;
    double center_y;
    double scale;
};

bool parse_args(int argc, char** argv, CommandLineOptions& options);
void print_usage(const char* executable_name);
std::vector<RenderConfig> make_benchmark_configs(const std::string& output_dir);
std::vector<ViewportPreset> available_viewport_presets();
bool apply_viewport_preset(const std::string& name, RenderConfig& config);
std::string fractal_name(FractalKind fractal);
bool parse_fractal(const std::string& text, FractalKind& fractal);

} // namespace henme
