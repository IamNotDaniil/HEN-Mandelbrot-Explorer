#pragma once

#include "henme/ImageBuffer.h"
#include "henme/RenderConfig.h"

namespace henme {

class CpuRenderer {
public:
    RenderStats render(const RenderConfig& config, ImageBuffer& image) const;
};

} // namespace henme
