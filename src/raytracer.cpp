/**
 * @file raytracer.cpp
 * @brief Set-up and execution of the raytracer program.
 */

#include "camera.hpp"
#include "color.hpp"
#include "material.hpp"
#include "math.hpp"
#include "png.hpp"
#include "random.hpp"
#include "shape.hpp"

#include <fmt/core.h>
#include <glm/glm.hpp>

#include <cstdio>
#include <exception>
#include <fstream>
#include <limits>
#include <memory>
#include <vector>

enum class sample_method
{
    single     = 0,
    stratified = 1,
};

/**
 * @brief The real entry point of the program.
 * @throws std::exception Thrown if a fatal error occurs.
 */
void run();

static vec3 sample_at(
    std::uint32_t x,
    std::uint32_t y,
    std::uint32_t width,
    std::uint32_t height,
    std::uint32_t count,
    sample_method method,
    const rt::camera &camera,
    const rt::world &world,
    std::uint32_t max_depth
);

static vec3 color_in_direction(const rt::ray &ray, const rt::world &world, std::uint32_t depth);

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
    constexpr real aspect_ratio = static_cast<real>(width) / static_cast<real>(height);

    constexpr std::uint32_t sample_count = 400;
    constexpr std::uint32_t depth_max = 64;

    constexpr real glass_refractive_index = real(1.52);

    constexpr rt::lambertian matte_white  (vec3(real(1.0)));
    constexpr rt::lambertian matte_grey   (vec3(real(0.5), real(0.5), real(0.5)));
    constexpr rt::metal      metallic_gold(vec3(real(0.8), real(0.6), real(0.2)));
    constexpr rt::dielectric glass        (glass_refractive_index);

    std::vector<std::unique_ptr<rt::hittable>> objects;
    objects.push_back(std::make_unique<rt::sphere>(vec3(real(-1.0), real(   0.0), real(1.0)), real(   0.5), glass));
    objects.push_back(std::make_unique<rt::sphere>(vec3(real( 0.0), real(   0.0), real(1.0)), real(   0.5), metallic_gold));
    objects.push_back(std::make_unique<rt::sphere>(vec3(real( 1.0), real(   0.0), real(1.0)), real(   0.5), matte_white));
    objects.push_back(std::make_unique<rt::sphere>(vec3(real( 0.0), real(1000.5), real(1.0)), real(1000.0), matte_grey));
    const rt::world world(std::move(objects));

    constexpr vec3 camera_position(real(-3.0), real(-2.0), real(-3.0));
    constexpr vec3 camera_target  (real( 0.0), real( 0.0), real( 1.0));
    constexpr vec3 camera_up      (real( 0.0), real(-1.0), real( 0.0));
    const rt::thin_lens_camera camera({
        .origin       = camera_position,
        .target       = camera_target,
        .up           = camera_up,
        .vertical_fov = glm::radians(real(47.0)),
        .aspect_ratio = aspect_ratio,
        .aperture     = real(0.1),
        .focal_length = glm::distance(camera_target, camera_position),
    });

    constexpr std::size_t size = png::image::uncompressed_size(width, height);
    std::vector<std::uint8_t> raw_bytes;
    raw_bytes.reserve(size);
    for (std::uint32_t row = 0; row < height; ++row)
    {
        fmt::print("\rRemaining rows: {}", height - row);
        std::fflush(stdout);
        for (std::uint32_t column = 0; column < width; ++column)
        {
            const std::uint32_t x = column;
            const std::uint32_t y = row;

            const vec3 color = sample_at(x, y, width, height, sample_count, sample_method::stratified, camera, world, depth_max);
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

vec3 sample_at(
    std::uint32_t x,
    std::uint32_t y,
    std::uint32_t width,
    std::uint32_t height,
    std::uint32_t count,
    sample_method method,
    const rt::camera &camera,
    const rt::world &world,
    std::uint32_t max_depth
)
{
    const real delta_u = real(1.0) / static_cast<real>(width);
    const real delta_v = real(1.0) / static_cast<real>(height);

    const real u0 = static_cast<real>(x) * delta_u;
    const real v0 = static_cast<real>(y) * delta_v;

    if (method == sample_method::single)
    {
        const rt::ray ray = camera.shoot_ray_at(u0, v0);
        return color_in_direction(ray, world, max_depth);
    }

    const std::uint32_t grid_size = static_cast<std::uint32_t>(std::ceil(std::sqrt(count)));

    const real du = delta_u / static_cast<real>(grid_size);
    const real dv = delta_v / static_cast<real>(grid_size);

    vec3 color(real(0.0));
    for (std::uint32_t i = 0; i < grid_size; ++i)
    {
        for (std::uint32_t j = 0; j < grid_size; ++j)
        {
            const real u = u0 + (static_cast<real>(i) * du * random_real());
            const real v = v0 + (static_cast<real>(j) * dv * random_real());

            const rt::ray ray = camera.shoot_ray_at(u, v);
            color += color_in_direction(ray, world, max_depth) / static_cast<real>(count);
        }
    }

    return color;
}

vec3 color_in_direction(const rt::ray &ray, const rt::world &world, std::uint32_t depth)
{
    if (depth == 0)
    {
        return vec3(real(0.0));
    }

    constexpr real t_max = std::numeric_limits<real>::infinity();
    if (const std::optional maybe_hit = world.hit(ray, t_min, t_max))
    {
        const rt::hit &hit = *maybe_hit;

        if (const std::optional maybe_scattered = hit.material().scatter(ray, hit))
        {
            const rt::scatter &scattered = *maybe_scattered;
            return scattered.color * color_in_direction(scattered.ray, world, depth - 1);
        }

        return vec3(real(0.0));
    }

    const vec3 direction = glm::normalize(ray.direction());

    const real t = real(0.5) * (direction.y + real(1.0));

    constexpr vec3 blue (real(0.5), real(0.7), real(1.0));
    constexpr vec3 white(real(1.0), real(1.0), real(1.0));

    const vec3 color = lerp(blue, white, t);
    return color;
}
