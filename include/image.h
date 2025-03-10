#ifndef _NIU_IMAGE_H_
#define _NIU_IMAGE_H_

#include <cstdint>
#include <cstdlib>
#include <memory>
#include <string>

namespace niu {
// -- Vector2 -----------------------------------------------------------------
typedef union
{
    struct { std::size_t x, y; };
    struct { std::size_t w, h; };
} Vector2;

std::istream&
operator >>(
        std::istream& os,
        Vector2& obj);

// -- Color -------------------------------------------------------------------
typedef union
{
    struct { uint8_t r, g, b, a; };
    uint32_t value;
} Color;

std::istream&
operator >>(
        std::istream& os,
        Color& obj);

// -- Format ------------------------------------------------------------------
enum class Format
{
    unknown,
    png,
};

// -- Image declaration -------------------------------------------------------
class Image
{
public:
    static std::size_t const channels = 4;

    // -- constructor ---------------------------------------------------------
    Image();

    // -- static --------------------------------------------------------------
    static Image
    make_image(
            std::size_t width,
            std::size_t height);

    // -- functions -----------------------------------------------------------
    bool
    load(std::string const& file);

    bool
    save(std::string const& file,
            Format format = Format::png) const;

    bool
    dump(std::string const& file) const;

    Image
    sub_image(
            std::size_t x,
            std::size_t y,
            std::size_t w,
            std::size_t h) const;

    // -- modifications -------------------------------------------------------
    void
    inverse_x();

    void
    inverse_y();

    void
    upscale(std::size_t n);

    Image
    upscaled(
            std::size_t n) const;

    void
    set_color(
            std::size_t x,
            std::size_t y,
            Color color);

    void
    fill(Color color);

    // -- data ----------------------------------------------------------------
    std::size_t
    width() const noexcept;

    std::size_t
    height() const noexcept;

private:
    // -- data ----------------------------------------------------------------
    std::size_t m_width;
    std::size_t m_height;
    std::shared_ptr<unsigned char> m_data;
};
}  // namespace niu

#endif  // _NIU_IMAGE_H_
