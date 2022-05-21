#ifndef RAYTRACER_MATH_HPP
#define RAYTRACER_MATH_HPP

#include <glm/glm.hpp>

#define RAYTRACER_USE_FLOAT
#ifdef RAYTRACER_USE_FLOAT
using real = float;
constexpr real t_min = real(0.5e-2);
#else
using real = double;
constexpr real t_min = real(1.0e-6);
#endif

using vec3 = glm::vec<3, real, glm::defaultp>;

inline vec3 lerp(vec3 from, vec3 to, real t)
{
    return (real(1.0) - t) * from + t * to;
}

#endif // !RAYTRACER_MATH_HPP
