#pragma once

#include "henme/ImageBuffer.h"
#include "henme/RenderConfig.h"

#include <string>
#include <vector>

namespace henme {

class ColorMapper {
public:
    [[nodiscard]] Pixel map(double smooth_iteration, int max_iterations, PaletteKind palette) const;
};

[[nodiscard]] std::string palette_name(PaletteKind palette);
[[nodiscard]] bool parse_palette(const std::string& text, PaletteKind& palette);
[[nodiscard]] std::vector<std::string> available_palettes();

} // namespace henme
