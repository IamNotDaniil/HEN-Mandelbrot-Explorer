#include "henme/Cli.h"
#include "henme/ColorMapper.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace henme {
namespace {

bool parse_int(const char* text, int& value) {
    try {
        value = std::stoi(text);
        return true;
    } catch (...) {
        return false;
    }
}

bool parse_double(const char* text, double& value) {
    try {
        value = std::stod(text);
        return true;
    } catch (...) {
        return false;
    }
}

bool parse_thread_sweep(const std::string& text, std::vector<int>& values) {
    std::stringstream stream(text);
    std::string token;
    std::vector<int> parsed;
    while (std::getline(stream, token, ',')) {
        if (token.empty()) {
            return false;
        }
        int value = 0;
        try {
            value = std::stoi(token);
        } catch (...) {
            return false;
        }
        if (value <= 0) {
            return false;
        }
        parsed.push_back(value);
    }
    if (parsed.empty()) {
        return false;
    }
    values = parsed;
    return true;
}

std::string palette_list() {
    std::string result;
    const auto palettes = available_palettes();
    for (std::size_t index = 0; index < palettes.size(); ++index) {
        if (index > 0) {
            result += ", ";
        }
        result += palettes[index];
    }
    return result;
}

std::string viewport_list() {
    std::string result;
    const auto presets = available_viewport_presets();
    for (std::size_t index = 0; index < presets.size(); ++index) {
        if (index > 0) {
            result += ", ";
        }
        result += presets[index].name;
    }
    return result;
}

bool parse_common_options(int argc, char** argv, int& index, RenderConfig& config, std::vector<int>* thread_sweep = nullptr) {
    while (index < argc) {
        const std::string argument = argv[index];
        if (argument == "--palette") {
            if (index + 1 >= argc || !parse_palette(argv[index + 1], config.palette)) {
                return false;
            }
            index += 2;
            continue;
        }
        if (argument == "--preset") {
            if (index + 1 >= argc || !apply_viewport_preset(argv[index + 1], config)) {
                return false;
            }
            index += 2;
            continue;
        }
        if (argument == "--threads") {
            if (index + 1 >= argc || !parse_int(argv[index + 1], config.thread_count) || config.thread_count <= 0) {
                return false;
            }
            index += 2;
            continue;
        }
        if (argument == "--thread-sweep") {
            if (thread_sweep == nullptr || index + 1 >= argc || !parse_thread_sweep(argv[index + 1], *thread_sweep)) {
                return false;
            }
            index += 2;
            continue;
        }
        if (argument == "--fractal") {
            if (index + 1 >= argc || !parse_fractal(argv[index + 1], config.fractal)) {
                return false;
            }
            index += 2;
            continue;
        }
        if (argument == "--julia-c") {
            if (index + 2 >= argc || !parse_double(argv[index + 1], config.julia_c_real) || !parse_double(argv[index + 2], config.julia_c_imag)) {
                return false;
            }
            config.fractal = FractalKind::Julia;
            index += 3;
            continue;
        }
        break;
    }

    return true;
}

bool parse_render_arguments(int argc, char** argv, int first_index, RenderConfig& config) {
    int index = first_index;
    if (!parse_common_options(argc, argv, index, config)) {
        return false;
    }

    const int remaining = argc - index;
    if (remaining == 0) {
        return true;
    }
    if (remaining != 4 && remaining != 7) {
        return false;
    }

    if (!parse_int(argv[index], config.width) ||
        !parse_int(argv[index + 1], config.height) ||
        !parse_int(argv[index + 2], config.max_iterations)) {
        return false;
    }

    config.output_path = argv[index + 3];

    if (remaining == 7) {
        if (!parse_double(argv[index + 4], config.center_x) ||
            !parse_double(argv[index + 5], config.center_y) ||
            !parse_double(argv[index + 6], config.scale)) {
            return false;
        }
        config.viewport_name = "custom";
    }

    return config.width > 0 && config.height > 0 && config.max_iterations > 0 && config.scale > 0.0;
}

} // namespace

std::string fractal_name(FractalKind fractal) {
    switch (fractal) {
        case FractalKind::Mandelbrot:
            return "mandelbrot";
        case FractalKind::Julia:
            return "julia";
    }
    return "unknown";
}

bool parse_fractal(const std::string& text, FractalKind& fractal) {
    if (text == "mandelbrot") {
        fractal = FractalKind::Mandelbrot;
        return true;
    }
    if (text == "julia") {
        fractal = FractalKind::Julia;
        return true;
    }
    return false;
}

bool parse_args(int argc, char** argv, CommandLineOptions& options) {
    if (argc == 1) {
        return true;
    }

    const std::string first_argument = argv[1];
    if (first_argument == "benchmark") {
        options.mode = CommandMode::Benchmark;
        int index = 2;
        if (!parse_common_options(argc, argv, index, options.render_config, &options.benchmark_thread_sweep)) {
            return false;
        }
        if (index < argc) {
            options.benchmark_output_dir = argv[index];
            ++index;
        }
        return index == argc;
    }

    if (first_argument == "render") {
        options.mode = CommandMode::Render;
        return parse_render_arguments(argc, argv, 2, options.render_config);
    }

    options.mode = CommandMode::Render;
    return parse_render_arguments(argc, argv, 1, options.render_config);
}

void print_usage(const char* executable_name) {
    std::cout
        << "Usage:\n"
        << "  " << executable_name << "\n"
        << "  " << executable_name << " render [--fractal type] [--julia-c real imag] [--palette name] [--preset name] [--threads n] [width height max_iter output_path [center_x center_y scale]]\n"
        << "  " << executable_name << " benchmark [--fractal type] [--julia-c real imag] [--palette name] [--preset name] [--threads n] [--thread-sweep a,b,c] [output_dir]\n\n"
        << "Fractals: mandelbrot, julia\n"
        << "Palettes: " << palette_list() << "\n"
        << "Viewport presets: " << viewport_list() << "\n\n"
        << "Examples:\n"
        << "  " << executable_name << "\n"
        << "  " << executable_name << " render --fractal mandelbrot --palette fire --preset seahorse --threads 8 1920 1080 1000 output/mandelbrot.bmp\n"
        << "  " << executable_name << " render --fractal julia --julia-c -0.8 0.156 --palette ice 1920 1080 1000 output/julia.bmp\n"
        << "  " << executable_name << " benchmark --fractal julia --julia-c -0.4 0.6 --threads 4 output/benchmarks\n"
        << "  " << executable_name << " benchmark --thread-sweep 1,2,4,8 output/benchmarks\n";
}

std::vector<RenderConfig> make_benchmark_configs(const std::string& output_dir) {
    return {
        RenderConfig{1280, 720, 500, -0.75, 0.0, 3.0, output_dir + "/benchmark_720p.ppm", PaletteKind::Classic, FractalKind::Mandelbrot, "default", 0, -0.8, 0.156},
        RenderConfig{1920, 1080, 750, -0.75, 0.0, 3.0, output_dir + "/benchmark_1080p.ppm", PaletteKind::Fire, FractalKind::Mandelbrot, "default", 0, -0.8, 0.156},
        RenderConfig{1920, 1080, 1200, -0.743643887037151, 0.131825904205330, 0.0005, output_dir + "/benchmark_zoom.ppm", PaletteKind::Ice, FractalKind::Mandelbrot, "seahorse", 0, -0.8, 0.156},
    };
}

std::vector<ViewportPreset> available_viewport_presets() {
    return {
        {"default", -0.75, 0.0, 3.0},
        {"seahorse", -0.743643887037151, 0.131825904205330, 0.0005},
        {"elephant", 0.282, 0.01, 0.008},
        {"spiral", -0.761574, -0.0847596, 0.01},
    };
}

bool apply_viewport_preset(const std::string& name, RenderConfig& config) {
    for (const auto& preset : available_viewport_presets()) {
        if (preset.name == name) {
            config.center_x = preset.center_x;
            config.center_y = preset.center_y;
            config.scale = preset.scale;
            config.viewport_name = preset.name;
            return true;
        }
    }
    return false;
}

} // namespace henme
