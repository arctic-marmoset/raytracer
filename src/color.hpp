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

constexpr real color_max = real(256.0) - std::numeric_limits<real>::epsilon() * real(128.0);

inline std::array<std::uint8_t, 4> to_rgba(vec3 color)
{
    const real r_real = glm::clamp(color.x, real(0.0), real(1.0));
    const real g_real = glm::clamp(color.y, real(0.0), real(1.0));
    const real b_real = glm::clamp(color.z, real(0.0), real(1.0));

    const std::uint8_t r = static_cast<std::uint8_t>(color_max * r_real);
    const std::uint8_t g = static_cast<std::uint8_t>(color_max * g_real);
    const std::uint8_t b = static_cast<std::uint8_t>(color_max * b_real);
    const std::uint8_t a = std::numeric_limits<std::uint8_t>::max();

    return { r, g, b, a };
}

#endif // !RAYTRACER_COLOR_HPP
