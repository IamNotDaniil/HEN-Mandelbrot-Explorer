#include "henme/ImageBuffer.h"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

namespace henme {
namespace {

void write_u16(std::ofstream& output, std::uint16_t value) {
    output.put(static_cast<char>(value & 0xFF));
    output.put(static_cast<char>((value >> 8) & 0xFF));
}

void write_u32(std::ofstream& output, std::uint32_t value) {
    output.put(static_cast<char>(value & 0xFF));
    output.put(static_cast<char>((value >> 8) & 0xFF));
    output.put(static_cast<char>((value >> 16) & 0xFF));
    output.put(static_cast<char>((value >> 24) & 0xFF));
}

std::string lowercase_extension(const std::string& path) {
    std::string extension = std::filesystem::path(path).extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return extension;
}

} // namespace

ImageBuffer::ImageBuffer(int width, int height)
    : width_(width), height_(height), pixels_(static_cast<std::size_t>(width) * static_cast<std::size_t>(height)) {}

Pixel& ImageBuffer::at(int x, int y) {
    return pixels_[static_cast<std::size_t>(y) * static_cast<std::size_t>(width_) + static_cast<std::size_t>(x)];
}

const Pixel& ImageBuffer::at(int x, int y) const {
    return pixels_[static_cast<std::size_t>(y) * static_cast<std::size_t>(width_) + static_cast<std::size_t>(x)];
}

int ImageBuffer::width() const {
    return width_;
}

int ImageBuffer::height() const {
    return height_;
}

bool ImageBuffer::save(const std::string& path) const {
    const std::string extension = lowercase_extension(path);
    if (extension == ".bmp") {
        return save_as_bmp(path);
    }
    return save_as_ppm(path);
}

bool ImageBuffer::save_as_ppm(const std::string& path) const {
    if (!ensure_parent_directory_exists(path)) {
        return false;
    }

    std::ofstream output(path, std::ios::binary);
    if (!output) {
        return false;
    }

    output << "P6\n" << width_ << ' ' << height_ << "\n255\n";
    for (const auto& pixel : pixels_) {
        output.write(reinterpret_cast<const char*>(&pixel), sizeof(Pixel));
    }

    return static_cast<bool>(output);
}

bool ImageBuffer::save_as_bmp(const std::string& path) const {
    if (!ensure_parent_directory_exists(path)) {
        return false;
    }

    std::ofstream output(path, std::ios::binary);
    if (!output) {
        return false;
    }

    const int row_stride = width_ * 3;
    const int row_padding = (4 - (row_stride % 4)) % 4;
    const std::uint32_t pixel_data_size = static_cast<std::uint32_t>((row_stride + row_padding) * height_);
    const std::uint32_t file_size = 14u + 40u + pixel_data_size;

    output.put('B');
    output.put('M');
    write_u32(output, file_size);
    write_u16(output, 0);
    write_u16(output, 0);
    write_u32(output, 54);

    write_u32(output, 40);
    write_u32(output, static_cast<std::uint32_t>(width_));
    write_u32(output, static_cast<std::uint32_t>(height_));
    write_u16(output, 1);
    write_u16(output, 24);
    write_u32(output, 0);
    write_u32(output, pixel_data_size);
    write_u32(output, 2835);
    write_u32(output, 2835);
    write_u32(output, 0);
    write_u32(output, 0);

    for (int y = height_ - 1; y >= 0; --y) {
        for (int x = 0; x < width_; ++x) {
            const Pixel& pixel = at(x, y);
            output.put(static_cast<char>(pixel.b));
            output.put(static_cast<char>(pixel.g));
            output.put(static_cast<char>(pixel.r));
        }
        for (int padding = 0; padding < row_padding; ++padding) {
            output.put('\0');
        }
    }

    return static_cast<bool>(output);
}

bool ImageBuffer::ensure_parent_directory_exists(const std::string& path) const {
    const std::filesystem::path file_path(path);
    if (file_path.has_parent_path()) {
        std::error_code error;
        std::filesystem::create_directories(file_path.parent_path(), error);
        if (error) {
            return false;
        }
    }
    return true;
}

} // namespace henme
