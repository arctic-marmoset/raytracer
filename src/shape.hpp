#ifndef RAYTRACER_SHAPE_HPP
#define RAYTRACER_SHAPE_HPP

#include "camera.hpp"
#include "math.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include <algorithm>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace rt
{
    struct material;
}

namespace rt
{
    class hit
    {
    public:
        explicit hit(
            vec3 point,
            vec3 outward_normal,
            const rt::material &material,
            const rt::ray &ray,
            real t
        )
            : point_(point)
            , material_(&material)
            , t_(t)
        {
            front_face_ = glm::dot(ray.direction(), outward_normal) < real(0.0);
            normal_ = front_face_ ? outward_normal : -outward_normal;
        }

        vec3 point() const
        {
            return point_;
        }

        vec3 normal() const
        {
            return normal_;
        }

        const rt::material &material() const
        {
            return *material_;
        }

        real t() const
        {
            return t_;
        }

        bool front_face() const
        {
            return front_face_;
        }

    private:
        vec3 point_;
        vec3 normal_;
        const rt::material *material_;
        real t_;
        bool front_face_;
    };

    struct hittable
    {
        [[nodiscard]]
        virtual std::optional<rt::hit> hit(const rt::ray &ray, real t_min, real t_max) const = 0;
        virtual ~hittable() = default;
    };

    class sphere : public hittable
    {
    public:
        constexpr explicit sphere(vec3 center, real radius, const rt::material &material)
            : material_(&material)
            , center_(center)
            , radius_(radius)
        {
        }

        [[nodiscard]]
        std::optional<rt::hit> hit(const rt::ray &ray, real t_min, real t_max) const override
        {
            const vec3 oc = ray.origin() - center_;

            const real a = glm::length2(ray.direction());
            const real half_b = glm::dot(ray.direction(), oc);
            const real c = glm::length2(oc) - (radius_ * radius_);

            const real discriminant = (half_b * half_b) - (a * c);
            if (discriminant <= real(0.0))
            {
                return std::nullopt;
            }

            const real sqrt_discriminant = glm::sqrt(discriminant);

            const std::array roots = {
                (-half_b - sqrt_discriminant) / a,
                (-half_b + sqrt_discriminant) / a,
            };

            const auto is_outside_interval = [&](real t)
            {
                return t < t_min || t > t_max;
            };

            const auto candidate = std::ranges::find_if_not(roots, is_outside_interval);
            if (candidate == roots.end())
            {
                return std::nullopt;
            }

            const real t = *candidate;
            const vec3 point = ray.at(t);
            const vec3 outward_normal = (point - center_) / radius_;

            const rt::hit hit(point, outward_normal, *material_, ray, t);
            return hit;
        }

    private:
        const rt::material *material_;
        vec3 center_;
        real radius_;
    };

    class world : public hittable
    {
    public:
        explicit world(std::vector<std::unique_ptr<hittable>> objects)
            : objects_(std::move(objects))
        {
        }

        [[nodiscard]]
        std::optional<rt::hit> hit(const rt::ray &ray, real t_min, real t_max) const override
        {
            std::optional<rt::hit> hit;

            real t_nearest = t_max;
            for (const auto &object : objects_)
            {
                if (const std::optional maybe_hit = object->hit(ray, t_min, t_nearest))
                {
                    hit = maybe_hit;
                    t_nearest = hit->t();
                }
            }

            return hit;
        }

    private:
        std::vector<std::unique_ptr<hittable>> objects_;
    };
}

#endif // ! RAYTRACER_SHAPE_HPP
