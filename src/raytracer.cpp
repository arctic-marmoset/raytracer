/**
 * @file raytracer.cpp
 * @brief Set-up and execution of the raytracer program.
 */

#include "color.hpp"
#include "png.hpp"
#include "rng.hpp"

#include <fmt/core.h>
#include <glm/glm.hpp>

#include <cstdio>
#include <exception>
#include <fstream>

/**
 * @brief The real entry point of the program.
 * @throws std::exception Thrown if a fatal error occurs.
 */
void run();

/**
 * @brief Wraps run() in a try-catch to handle printing fatal errors.
 */
int main()
{
    try
    {
        run();
    }
    catch (const std::exception &e)
    {
        fmt::print(stderr, "fatal error: {}\n", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void run()
{
    constexpr std::uint32_t width = 1280;
    constexpr std::uint32_t height = 720;
    constexpr std::size_t size = png::image::uncompressed_size(width, height);

    std::vector<std::uint8_t> raw_bytes;
    raw_bytes.reserve(size);
    for (std::uint32_t row = 0; row < height; ++row)
    {
        fmt::print("\rRemaining rows: {}", height - row);
        std::fflush(stdout);
        for (std::uint32_t column = 0; column < width; ++column)
        {
            const float u = static_cast<float>(column) / static_cast<float>(width);
            const float v = static_cast<float>(row) / static_cast<float>(height);

            const glm::vec3 color(u, v, 0.5F);
            const std::array rgba = to_rgba(color);
            raw_bytes.insert(raw_bytes.end(), rgba.begin(), rgba.end());
        }
    }

    const png::image image = {
        .raw_bytes = raw_bytes,
        .width     = width,
        .height    = height,
    };

    constexpr const char output_filepath[] = "output.png";
    std::ofstream output(output_filepath, std::ios::binary);
    image.write_to(output);
}
