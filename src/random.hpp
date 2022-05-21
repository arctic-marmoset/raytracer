/**
 * @file random.hpp
 * @brief Generating random numbers and other values.
 */

#ifndef RAYTRACER_RANDOM_HPP
#define RAYTRACER_RANDOM_HPP

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include <ctime>
#include <functional>
#include <random>
#include <thread>

/**
 * @brief Generates a float @f$\texttt{f} \in [\texttt{min}, \texttt{max})@f$ with uniform probability.
 * @param[in] min The lower bound of the range.
 * @param[in] max The upper bound of the range.
 * @note This function is thread-safe.
 */
inline real random_real(real min, real max)
{
    static thread_local std::mt19937 generator(std::clock() + std::hash<std::thread::id>()(std::this_thread::get_id()));
    static thread_local std::uniform_real_distribution<real> distribution;
    return distribution(generator, decltype(distribution)::param_type(min, max));
}

/**
 * @brief Generates a real @f$\texttt{d} \in [0, 1)@f$ with uniform probability.
 * @note This function is thread-safe.
 */
inline real random_real()
{
    return random_real(real(0.0), real(1.0));
}

inline vec3 random_vec3()
{
    return { random_real(), random_real(), random_real() };
}

inline vec3 random_vec3(real min, real max)
{
    return { random_real(min, max), random_real(min, max), random_real(min, max) };
}

inline vec3 random_in_unit_sphere()
{
    while (true)
    {
        if (const vec3 point = random_vec3(real(-1.0), real(1.0)); glm::length2(point) < real(1.0))
        {
            return point;
        }
    }
}

inline vec3 random_unit_vector()
{
    return glm::normalize(random_in_unit_sphere());
}

inline vec3 random_in_unit_disk()
{
    while (true)
    {
        if (const vec3 point = vec3(random_real(real(-1.0), real(1.0)), random_real(real(-1.0), real(1.0)), real(0.0)); glm::length2(point) < real(1.0))
        {
            return point;
        }
    }
}

#endif // !RAYTRACER_RANDOM_HPP
