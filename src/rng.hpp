/**
 * @file rng.hpp
 * @brief Generating random numbers and other values.
 */

#ifndef RAYTRACER_RNG_HPP
#define RAYTRACER_RNG_HPP

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
inline float random_float(float min, float max)
{
    static thread_local std::mt19937 generator(std::clock() + std::hash<std::thread::id>()(std::this_thread::get_id()));
    static thread_local std::uniform_real_distribution<float> distribution;
    return distribution(generator, decltype(distribution)::param_type(min, max));
}

/**
 * @brief Generates a float @f$\texttt{f} \in [0, 1)@f$ with uniform probability.
 * @note This function is thread-safe.
 */
inline float random_float()
{
    return random_float(0.0F, 1.0F);
}

#endif // !RAYTRACER_RNG_HPP
