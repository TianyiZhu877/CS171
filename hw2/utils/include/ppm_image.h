#ifndef PPM_IMAGE_H
#define PPM_IMAGE_H

#include <cstring>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>

namespace ppm_image {

template<typename T>
struct Pixel {
    T r;
    T g;
    T b;
    
    constexpr Pixel() : r(0), g(0), b(0) {}
    constexpr Pixel(T r, T g, T b) : r(r), g(g), b(b) {}

    constexpr Pixel operator+(const Pixel& other) const {
        return Pixel(r + other.r, g + other.g, b + other.b);
    }
    
    constexpr Pixel operator-(const Pixel& other) const {
        return Pixel(r - other.r, g - other.g, b - other.b);
    }
    
    constexpr Pixel operator*(float scalar) const {
        return Pixel(static_cast<T>(r * scalar), static_cast<T>(g * scalar), static_cast<T>(b * scalar));
    }
    
    constexpr Pixel operator/(float scalar) const {
        return Pixel(static_cast<T>(r / scalar), static_cast<T>(g / scalar), static_cast<T>(b / scalar));
    }
    
    constexpr Pixel& operator+=(const Pixel& other) {
        r += other.r;
        g += other.g;
        b += other.b;
        return *this;
    }
    
    constexpr Pixel& operator-=(const Pixel& other) {
        r -= other.r;
        g -= other.g;
        b -= other.b;
        return *this;
    }
    
    constexpr Pixel& operator*=(float scalar) {
        r = static_cast<T>(r * scalar);
        g = static_cast<T>(g * scalar);
        b = static_cast<T>(b * scalar);
        return *this;
    }
    
    constexpr Pixel& operator/=(float scalar) {
        r = static_cast<T>(r / scalar);
        g = static_cast<T>(g / scalar);
        b = static_cast<T>(b / scalar);
        return *this;
    }

    void clamp(T color_scale) {
        r = std::clamp(r, 0.0f, color_scale);
        g = std::clamp(g, 0.0f, color_scale);
        b = std::clamp(b, 0.0f, color_scale);
    }
    
    // Friend function for scalar * Pixel (to support commutativity)
    friend constexpr Pixel operator*(float scalar, const Pixel& p) {
        return Pixel(static_cast<T>(scalar * p.r), static_cast<T>(scalar * p.g), static_cast<T>(scalar * p.b));
    }
};


// PPMImage class that stores the image data, and support loading from PPM filem.
// You can access the pixel data using image[y][x]
template<typename T>
class PPMImage {
public:
    PPMImage() : width(0), height(0), data(nullptr), color_scale(255) {}
    
    PPMImage(std::size_t height, std::size_t width, T color_scale = 255, Pixel<T> background_color = Pixel<T>{0, 0, 0}) 
        : width(width), height(height), color_scale(color_scale) {
        data = new Pixel<T>[width * height];
        for (std::size_t i = 0; i < width * height; ++i) 
            data[i] = background_color;
    }
    
    // Copy constructor
    PPMImage(const PPMImage& other) 
        : width(other.width), height(other.height), color_scale(other.color_scale) {
        if (other.data) {
            data = new Pixel<T>[width * height];
            std::memcpy(data, other.data, width * height * sizeof(Pixel<T>));
        } else {
            data = nullptr;
        }
    }
    
    // Move constructor
    PPMImage(PPMImage&& other) noexcept 
        : width(other.width), height(other.height), color_scale(other.color_scale), data(other.data) {
        other.width = 0;
        other.height = 0;
        other.data = nullptr;
    }
    
    // Copy assignment
    PPMImage& operator=(const PPMImage& other) {
        if (this != &other) {
            delete[] data;
            width = other.width;
            height = other.height;
            color_scale = other.color_scale;
            if (other.data) {
                data = new Pixel<T>[width * height];
                std::memcpy(data, other.data, width * height * sizeof(Pixel<T>));
            } else {
                data = nullptr;
            }
        }
        return *this;
    }
    
    // Move assignment
    PPMImage& operator=(PPMImage&& other) noexcept {
        if (this != &other) {
            delete[] data;
            width = other.width;
            height = other.height;
            color_scale = other.color_scale;
            data = other.data;
            other.width = 0;
            other.height = 0;
            other.data = nullptr;
        }
        return *this;
    }
    
    // Destructor
    ~PPMImage() {
        delete[] data;
    }
    
    // Allows image[y][x]
    Pixel<T>* operator[](std::size_t y) {
        return &data[y * width];
    }
    
    // Const version
    const Pixel<T>* operator[](std::size_t y) const {
        return &data[y * width];
    }
    
    // Get dimensions
    std::size_t w() const { return width; }
    std::size_t h() const { return height; }

    
    void serialize(std::ostream& os = std::cout) const {
        os << "P3\n";
        os << width << " " << height << "\n";
        os << "255\n";
        
        for (std::size_t y = 0; y < height; ++y) {
            for (std::size_t x = 0; x < width; ++x) {
                const Pixel<T>& pixel = (*this)[height-1-y][x];
                if (color_scale == 255) {
                    os << std::clamp(static_cast<int>(pixel.r), 0, 255) << " "
                       << std::clamp(static_cast<int>(pixel.g), 0, 255) << " "
                       << std::clamp(static_cast<int>(pixel.b), 0, 255) << " ";
                } else {
                    os << std::clamp(static_cast<int>(static_cast<float>(pixel.r) * 255.0 / color_scale), 0, 255) << " "
                       << std::clamp(static_cast<int>(static_cast<float>(pixel.g) * 255.0 / color_scale), 0, 255) << " "
                       << std::clamp(static_cast<int>(static_cast<float>(pixel.b) * 255.0 / color_scale), 0, 255) << " ";
                }
            }
            os << "\n";
        }
    }

private:
    std::size_t width;
    std::size_t height;
    Pixel<T>* data;
    T color_scale;
};

// Color constants for uint8_t pixels (0-255 range)
namespace colors_u8 {
    constexpr Pixel<uint8_t> WHITE = Pixel<uint8_t>{255, 255, 255};
    constexpr Pixel<uint8_t> BLACK = Pixel<uint8_t>{0, 0, 0};
    constexpr Pixel<uint8_t> RED = Pixel<uint8_t>{255, 0, 0};
    constexpr Pixel<uint8_t> GREEN = Pixel<uint8_t>{0, 255, 0};
    constexpr Pixel<uint8_t> BLUE = Pixel<uint8_t>{0, 0, 255};
    constexpr Pixel<uint8_t> YELLOW = Pixel<uint8_t>{255, 255, 0};
    constexpr Pixel<uint8_t> MAGENTA = Pixel<uint8_t>{255, 0, 255};
    constexpr Pixel<uint8_t> CYAN = Pixel<uint8_t>{0, 255, 255};
    constexpr Pixel<uint8_t> GRAY = Pixel<uint8_t>{128, 128, 128};
    constexpr Pixel<uint8_t> BROWN = Pixel<uint8_t>{165, 42, 42};
    constexpr Pixel<uint8_t> ORANGE = Pixel<uint8_t>{255, 165, 0};
    constexpr Pixel<uint8_t> PINK = Pixel<uint8_t>{255, 192, 203};
    constexpr Pixel<uint8_t> PURPLE = Pixel<uint8_t>{128, 0, 128};
    constexpr Pixel<uint8_t> LIME = Pixel<uint8_t>{0, 255, 0};
    constexpr Pixel<uint8_t> TEAL = Pixel<uint8_t>{0, 128, 128};
    constexpr Pixel<uint8_t> NAVY = Pixel<uint8_t>{0, 0, 128};
    constexpr Pixel<uint8_t> INDIGO = Pixel<uint8_t>{75, 0, 130};
    constexpr Pixel<uint8_t> VIOLET = Pixel<uint8_t>{230, 130, 230};
    constexpr Pixel<uint8_t> LAVENDER = Pixel<uint8_t>{230, 230, 250};
    constexpr Pixel<uint8_t> MAROON = Pixel<uint8_t>{128, 0, 0};
    constexpr Pixel<uint8_t> OLIVE = Pixel<uint8_t>{128, 128, 0};
}

// Color constants for float pixels (0.0-1.0 range)
namespace colors_f {
    constexpr Pixel<float> WHITE = Pixel<float>{1.0f, 1.0f, 1.0f};
    constexpr Pixel<float> BLACK = Pixel<float>{0.0f, 0.0f, 0.0f};
    constexpr Pixel<float> RED = Pixel<float>{1.0f, 0.0f, 0.0f};
    constexpr Pixel<float> GREEN = Pixel<float>{0.0f, 1.0f, 0.0f};
    constexpr Pixel<float> BLUE = Pixel<float>{0.0f, 0.0f, 1.0f};
    constexpr Pixel<float> YELLOW = Pixel<float>{1.0f, 1.0f, 0.0f};
    constexpr Pixel<float> MAGENTA = Pixel<float>{1.0f, 0.0f, 1.0f};
    constexpr Pixel<float> CYAN = Pixel<float>{0.0f, 1.0f, 1.0f};
    constexpr Pixel<float> GRAY = Pixel<float>{0.502f, 0.502f, 0.502f};
    constexpr Pixel<float> BROWN = Pixel<float>{0.647f, 0.165f, 0.165f};
    constexpr Pixel<float> ORANGE = Pixel<float>{1.0f, 0.647f, 0.0f};
    constexpr Pixel<float> PINK = Pixel<float>{1.0f, 0.753f, 0.796f};
    constexpr Pixel<float> PURPLE = Pixel<float>{0.502f, 0.0f, 0.502f};
    constexpr Pixel<float> LIME = Pixel<float>{0.0f, 1.0f, 0.0f};
    constexpr Pixel<float> TEAL = Pixel<float>{0.0f, 0.502f, 0.502f};
    constexpr Pixel<float> NAVY = Pixel<float>{0.0f, 0.0f, 0.502f};
    constexpr Pixel<float> INDIGO = Pixel<float>{0.294f, 0.0f, 0.510f};
    constexpr Pixel<float> VIOLET = Pixel<float>{0.902f, 0.510f, 0.902f};
    constexpr Pixel<float> LAVENDER = Pixel<float>{0.902f, 0.902f, 0.980f};
    constexpr Pixel<float> MAROON = Pixel<float>{0.502f, 0.0f, 0.0f};
    constexpr Pixel<float> OLIVE = Pixel<float>{0.502f, 0.502f, 0.0f};
}


} // namespace ppm_image

#endif // PPM_IMAGE_H