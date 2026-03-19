#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace henme {

struct Pixel {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
};

class ImageBuffer {
public:
    ImageBuffer(int width, int height);

    Pixel& at(int x, int y);
    [[nodiscard]] const Pixel& at(int x, int y) const;
    [[nodiscard]] int width() const;
    [[nodiscard]] int height() const;
    bool save(const std::string& path) const;
    bool save_as_ppm(const std::string& path) const;
    bool save_as_bmp(const std::string& path) const;

private:
    bool ensure_parent_directory_exists(const std::string& path) const;

    int width_;
    int height_;
    std::vector<Pixel> pixels_;
};

} // namespace henme
