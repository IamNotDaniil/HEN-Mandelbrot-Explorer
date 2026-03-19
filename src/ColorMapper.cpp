#include "henme/ColorMapper.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace henme {
namespace {

std::array<double, 3> palette_triplet(PaletteKind palette, double t) {
    const double tau = 6.28318530717958647692;
    switch (palette) {
        case PaletteKind::Classic:
            return {
                0.5 + 0.5 * std::sin((t + 0.00) * tau),
                0.5 + 0.5 * std::sin((t + 0.33) * tau),
                0.5 + 0.5 * std::sin((t + 0.67) * tau),
            };
        case PaletteKind::Fire:
            return {
                std::clamp(1.5 * t, 0.0, 1.0),
                std::clamp(1.5 * t * t, 0.0, 1.0),
                std::clamp(0.35 * t * t * t, 0.0, 1.0),
            };
        case PaletteKind::Ice:
            return {
                std::clamp(0.30 * t, 0.0, 1.0),
                std::clamp(0.55 + 0.45 * t, 0.0, 1.0),
                std::clamp(0.65 + 0.35 * t, 0.0, 1.0),
            };
        case PaletteKind::Grayscale:
            return {t, t, t};
    }

    return {t, t, t};
}

} // namespace

Pixel ColorMapper::map(double smooth_iteration, int max_iterations, PaletteKind palette) const {
    if (smooth_iteration >= static_cast<double>(max_iterations)) {
        return Pixel{0, 0, 0};
    }

    const double t = std::clamp(smooth_iteration / static_cast<double>(max_iterations), 0.0, 1.0);
    const auto [r, g, b] = palette_triplet(palette, t);
    return Pixel{
        static_cast<std::uint8_t>(std::round(255.0 * std::clamp(r, 0.0, 1.0))),
        static_cast<std::uint8_t>(std::round(255.0 * std::clamp(g, 0.0, 1.0))),
        static_cast<std::uint8_t>(std::round(255.0 * std::clamp(b, 0.0, 1.0))),
    };
}

std::string palette_name(PaletteKind palette) {
    switch (palette) {
        case PaletteKind::Classic:
            return "classic";
        case PaletteKind::Fire:
            return "fire";
        case PaletteKind::Ice:
            return "ice";
        case PaletteKind::Grayscale:
            return "grayscale";
    }

    return "unknown";
}

bool parse_palette(const std::string& text, PaletteKind& palette) {
    if (text == "classic") {
        palette = PaletteKind::Classic;
        return true;
    }
    if (text == "fire") {
        palette = PaletteKind::Fire;
        return true;
    }
    if (text == "ice") {
        palette = PaletteKind::Ice;
        return true;
    }
    if (text == "grayscale") {
        palette = PaletteKind::Grayscale;
        return true;
    }
    return false;
}

std::vector<std::string> available_palettes() {
    return {"classic", "fire", "ice", "grayscale"};
}

} // namespace henme
