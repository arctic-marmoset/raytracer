/**
 * @file color.hpp
 * @brief Working with floating-point and integer colors.
 */

#ifndef RAYTRACER_COLOR_HPP
#define RAYTRACER_COLOR_HPP

#include <glm/glm.hpp>

#include <array>
#include <cstdint>
#include <limits>

constexpr float color_max = 256.0F - std::numeric_limits<float>::epsilon() * 128.0F;

inline std::array<std::uint8_t, 4> to_rgba(glm::vec3 color)
{
    const float r_float = color.x;
    const float g_float = color.y;
    const float b_float = color.z;

    const std::uint8_t r = static_cast<std::uint8_t>(color_max * r_float);
    const std::uint8_t g = static_cast<std::uint8_t>(color_max * g_float);
    const std::uint8_t b = static_cast<std::uint8_t>(color_max * b_float);
    const std::uint8_t a = std::numeric_limits<std::uint8_t>::max();

    return { r, g, b, a };
}

#endif // !RAYTRACER_COLOR_HPP
