#include "image.h"

#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>

#include <png.h>

#define STB_IMAGE_IMPLEMENTATION
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <stb_image.h>
#pragma GCC diagnostic pop

#include "endian.h"
#include "utils.h"

namespace niu {
namespace {
// ----------------------------------------------------------------------------
template <class T>
inline std::shared_ptr<T>
malloc_shared_array(
        std::size_t size)
{
    return std::shared_ptr<T>(reinterpret_cast<T*>(std::malloc(size)),
                              [] (T* ptr) { std::free(ptr); });
}

// ----------------------------------------------------------------------------
inline std::size_t
image_memsize(
        std::size_t width,
        std::size_t height)
{
    std::size_t res = Image::channels;
    auto _safe_multiply = [&res] (std::size_t value)
    {
        if (value > std::numeric_limits<int32_t>::max()
                || std::numeric_limits<int32_t>::max() / value <= res) {
            throw std::overflow_error("integer overflow");
        }
        res *= value;
    };
    _safe_multiply(width);
    _safe_multiply(height);
    return res;
}

// ----------------------------------------------------------------------------
inline std::size_t
pixel_index(
        Image const& image,
        std::size_t y,
        std::size_t x)
{
    return Image::channels * (y * image.width() + x);
}

// ----------------------------------------------------------------------------
inline std::string
remove_extension(
        std::string const& path)
{
    return path.substr(0, path.find_last_of("."));
}

// ----------------------------------------------------------------------------
inline std::string
add_extension(
        std::string const& file,
        std::string const& ext)
{
    auto res = file;
    if (!utils::_ends_with(res, ext)) {
        res += ext;
    }
    return res;
}

// ----------------------------------------------------------------------------
inline bool
process_check_file(
        unsigned char const* data,
        std::string const& file)
{
    if (!data) {
        std::cerr << "empty image data: " << file << std::endl;
        return false;
    }
    std::string dir = utils::_directory_name(file);
    if (!dir.empty()
            && !utils::_is_directory_exists(dir)
            && !utils::_make_directory(dir)) {
        std::cerr << "can't create directory for image: " << file << std::endl;
        return false;
    }
    return true;
}

// ----------------------------------------------------------------------------
inline bool
save_by_libpng(
        std::string const& file,
        std::size_t const w,
        std::size_t const h,
        std::size_t const channels,
        unsigned char* image)
{
    png_structp png_ptr = png_create_write_struct(
                PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png_ptr) {
        return false;
    }

    FILE* f = fopen(file.c_str(), "wb");
    png_infop png_info = png_create_info_struct(png_ptr);
    if (!f || !png_info || setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, nullptr);
        return false;
    }

    png_init_io(png_ptr, f);

    png_set_IHDR(png_ptr, png_info, static_cast<png_uint_32>(w),
                 static_cast<png_uint_32>(h), 8,
                 channels == 4 ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);

    unsigned char* rows[h];
    for (std::size_t i = 0; i < h; ++i) {
        rows[i] = image + (i * w * channels);
    }

    png_set_rows(png_ptr, png_info, rows);
    png_write_png(png_ptr, png_info, PNG_TRANSFORM_IDENTITY, nullptr);
    png_write_end(png_ptr, png_info);

    png_destroy_write_struct(&png_ptr, nullptr);

    fclose(f);
    return true;
}
}  // namespace

// ----------------------------------------------------------------------------
std::istream&
operator >>(
        std::istream& os,
        Vector2& obj)
{
    os >> obj.x;
    os >> obj.y;
    return os;
}

// ----------------------------------------------------------------------------
std::istream&
operator >>(
        std::istream& os,
        Color& obj)
{
    std::string tmp;
    os >> tmp;
    if (tmp.size() != 6 && tmp.size() != 8) {
        throw std::invalid_argument("invalid Color format");
    }
    uint32_t value;
    std::stringstream ss;
    ss << std::hex << tmp;
    ss >> value;
    if (ss.fail() || !ss.eof()) {
        throw std::invalid_argument("incorrect Color format");
    }
    if (tmp.size() == 6) {
        value <<= 8;
        value |= 0xff;
    }
    obj.value = ntohl(value);
    return os;
}

// -- Image implementation ----------------------------------------------------
Image::Image()
    : m_width(),
      m_height(),
      m_data(nullptr)
{
}

// ----------------------------------------------------------------------------
Image
Image::make_image(
        std::size_t width,
        std::size_t height)
{
    std::size_t const size = image_memsize(width, height);
    Image res;
    res.m_width = width;
    res.m_height = height;
    res.m_data = malloc_shared_array<unsigned char>(size);
    memset(res.m_data.get(), 0, size);
    return res;
}

// ----------------------------------------------------------------------------
bool
Image::load(
        std::string const& file)
{
    int w, h, ch;
    std::shared_ptr<unsigned char> data(
                stbi_load(file.c_str(), &w, &h, &ch, 0),
                [] (unsigned char* ptr) { stbi_image_free(ptr); });
    if (!data.get()) {
        std::cerr << "Failed to load image: " << file << std::endl;
        return false;
    }
    if (ch == channels) {
        m_width = static_cast<std::size_t>(w);
        m_height = static_cast<std::size_t>(h);
        m_data = data;
        return true;
    } else if (ch == 3) {
        std::cout << "add alpha" << std::endl;
        std::size_t const size = image_memsize(static_cast<std::size_t>(w),
                                               static_cast<std::size_t>(h));
        m_width = static_cast<std::size_t>(w);
        m_height = static_cast<std::size_t>(h);
        m_data = malloc_shared_array<unsigned char>(size);
        memset(m_data.get(), 0, size);
        for (std::size_t i = 0; i < height(); ++i) {
            for (std::size_t j = 0; j < width(); ++j) {
                std::size_t const index = (i * width() + j);
                std::size_t i1 = index * channels;
                std::size_t i2 = index * static_cast<std::size_t>(ch);
                for (int c = 0; c < ch; ++c) {
                    m_data.get()[i1++] = data.get()[i2++];
                }
                m_data.get()[i1] = static_cast<unsigned char>(255);
            }
        }
        return true;
    } else {
        std::cerr << "Failed to load image: " << file
                  << ". unsupported image format" << std::endl;
        return false;
    }
}

// ----------------------------------------------------------------------------
bool
Image::save(
        std::string const& file,
        Format format) const
{
    switch (format) {
        case Format::png :
            {
                auto filename = add_extension(remove_extension(file), ".png");
                if (!process_check_file(m_data.get(), filename)) {
                    return false;
                }
                if (save_by_libpng(filename, width(), height(), channels, m_data.get())) {
                    return true;
                }
            }
            return false;
        default :
            std::cerr << "Failed to save image: " << file
                      << ". unsupported image format" << std::endl;
            return false;
    }
}

// ----------------------------------------------------------------------------
Image
Image::sub_image(
        std::size_t x,
        std::size_t y,
        std::size_t w,
        std::size_t h) const
{
    std::size_t const size = image_memsize(w, h);
    if (x + w >= width() || y + h >= height()) {
        throw std::invalid_argument("invalid sub image parameters");
    }
    Image res;
    res.m_width = w;
    res.m_height = h;
    res.m_data = malloc_shared_array<unsigned char>(size);
    memset(res.m_data.get(), 0, size);
    for (std::size_t ix = 0; ix < w; ++ix) {
        for (std::size_t iy = 0; iy < h; ++iy) {
            std::size_t i1 = channels * (ix + iy * w);
            std::size_t i2 = channels * ((ix + x) + (iy + y) * m_width);
            for (std::size_t c = 0; c < channels; ++c) {
                res.m_data.get()[i1++] = m_data.get()[i2++];
            }
        }
    }
    return res;
}

// ----------------------------------------------------------------------------
void
Image::inverse_x()
{
    for (std::size_t i = 0; i < height(); ++i) {
        for (std::size_t j = 0; j < width() / 2; ++j) {
            std::size_t i1 = pixel_index(*this, i, j);
            std::size_t i2 = channels * (i * (width() - j - 1) + j);
            for (std::size_t c = 0; c < channels; ++c) {
                std::swap(m_data.get()[i1++], m_data.get()[i2++]);
            }
        }
    }
}

// ----------------------------------------------------------------------------
void
Image::inverse_y()
{
    for (std::size_t i = 0; i < height() / 2; ++i) {
        for (std::size_t j = 0; j < width(); ++j) {
            std::size_t i1 = pixel_index(*this, i, j);
            std::size_t i2 = channels * ((height() - i - 1) * width() + j);
            for (std::size_t c = 0; c < channels; ++c) {
                std::swap(m_data.get()[i1++], m_data.get()[i2++]);
            }
        }
    }
}

// ----------------------------------------------------------------------------
void
Image::upscale(
        std::size_t n)
{
    Image res = upscaled(n);
    std::swap(*this, res);
}

// ----------------------------------------------------------------------------
Image
Image::upscaled(std::size_t n) const
{
    std::size_t const size = image_memsize(m_width * n, m_height * n);
    Image res;
    res.m_width = m_width * n;
    res.m_height = m_height * n;
    res.m_data = malloc_shared_array<unsigned char>(size);
    memset(res.m_data.get(), 0, size);
    for (std::size_t ix = 0; ix < width(); ++ix) {
        for (std::size_t iy = 0; iy < height(); ++iy) {
            std::size_t index2 = pixel_index(*this, iy, ix);
            for (std::size_t nx = 0; nx < n; ++nx) {
                for (std::size_t ny = 0; ny < n; ++ny) {
                    std::size_t index1 = channels * (ix * n + nx + (iy * n + ny) * res.width());
                    for (std::size_t c = 0; c < channels; ++c) {
                        res.m_data.get()[index1 + c] = m_data.get()[index2 + c];
                    }
                }
            }
        }
    }
    return res;
}

// ----------------------------------------------------------------------------
void Image::set_color(
        std::size_t x,
        std::size_t y,
        Color color)
{
    if (x >= width() || y >= height()) {
        throw std::invalid_argument("invalid image parameters");
    }
    std::size_t index = pixel_index(*this, y, x);
    m_data.get()[index++] = color.r;
    m_data.get()[index++] = color.g;
    m_data.get()[index++] = color.b;
    m_data.get()[index++] = color.a;
}

// ----------------------------------------------------------------------------
void
Image::fill(
        Color color)
{
    for (std::size_t i = 0; i < height(); ++i) {
        for (std::size_t j = 0; j < width(); ++j) {
            std::size_t index = pixel_index(*this, i, j);
            m_data.get()[index++] = color.r;
            m_data.get()[index++] = color.g;
            m_data.get()[index++] = color.b;
            m_data.get()[index++] = color.a;
        }
    }
}

// ----------------------------------------------------------------------------
std::size_t
Image::width() const noexcept
{
    return m_width;
}

// ----------------------------------------------------------------------------
std::size_t
Image::height() const noexcept
{
    return m_height;
}
}  // namespace niu
