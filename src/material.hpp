#ifndef RAYTRACER_MATERIAL_HPP
#define RAYTRACER_MATERIAL_HPP

#include "camera.hpp"
#include "random.hpp"
#include "shape.hpp"

#include <glm/glm.hpp>

#include <optional>

namespace rt
{
    struct scatter
    {
        rt::ray ray;
        vec3    color;
    };

    struct material
    {
        [[nodiscard]]
        virtual std::optional<rt::scatter> scatter(const rt::ray &ray, const rt::hit &hit) const = 0;
        virtual ~material() = default;
    };

    class lambertian : public material
    {
    public:
        constexpr explicit lambertian(vec3 albedo)
            : albedo_(albedo)
        {
        }

        [[nodiscard]]
        std::optional<rt::scatter> scatter(const rt::ray &ray, const rt::hit &hit) const override
        {
            vec3 direction = hit.normal() + random_unit_vector();
            if (glm::all(glm::epsilonEqual(direction, vec3(real(0.0)), glm::epsilon<real>())))
            {
                direction = hit.normal();
            }

            const rt::ray scattered_ray(hit.point(), direction);
            const vec3 attenuation = albedo_;

            return rt::scatter {
                .ray   = scattered_ray,
                .color = attenuation,
            };
        }

    private:
        vec3 albedo_;
    };

    class metal : public material
    {
    public:
        constexpr explicit metal(vec3 albedo)
            : albedo_(albedo)
        {
        }

        [[nodiscard]]
        std::optional<rt::scatter> scatter(const rt::ray &ray, const rt::hit &hit) const override
        {
            const vec3 reflected_direction = glm::reflect(glm::normalize(ray.direction()), hit.normal());
            const rt::ray scattered_ray(hit.point(), reflected_direction);
            const vec3 attenuation = albedo_;

            if (glm::dot(scattered_ray.direction(), hit.normal()) < real(0.0))
            {
                return std::nullopt;
            }

            return rt::scatter {
                .ray   = scattered_ray,
                .color = attenuation,
            };
        }
    private:
        vec3 albedo_;
    };

    class dielectric : public material
    {
    public:
        constexpr explicit dielectric(real refractive_index)
            : refractive_index_(refractive_index)
        {
        }

        [[nodiscard]]
        std::optional<rt::scatter> scatter(const rt::ray &ray, const rt::hit &hit) const override
        {
            const vec3 attenuation = vec3(real(1.0));
            const real eta = hit.front_face() ? (real(1.0) / refractive_index_) : refractive_index_;

            const vec3 unit_direction = glm::normalize(ray.direction());
            const real cos_theta = glm::min(glm::dot(-unit_direction, hit.normal()), real(1.0));
            const real sin_theta = glm::sqrt(real(1.0) - (cos_theta * cos_theta));

            const bool must_reflect = eta * sin_theta > real(1.0);
            const vec3 scattered_direction = [&]
            {
                if (must_reflect || reflectance(cos_theta, eta) > random_real())
                {
                    return glm::reflect(unit_direction, hit.normal());
                }

                return glm::refract(unit_direction, hit.normal(), eta);
            }();

            const rt::ray scattered_ray(hit.point(), scattered_direction);

            return rt::scatter {
                .ray   = scattered_ray,
                .color = attenuation,
            };
        }

    private:
        static real reflectance(real cosine, real eta)
        {
            const real sqrt_r0 = (real(1.0) - eta) / (real(1.0) + eta);
            const real r0 = sqrt_r0 * sqrt_r0;
            const real r = r0 + (real(1.0) - r0) * glm::pow(real(1.0) - cosine, real(5.0));

            return r;
        }

    private:
        real refractive_index_;
    };
}

#endif
