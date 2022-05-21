#ifndef RAYTRACER_CAMERA_HPP
#define RAYTRACER_CAMERA_HPP

#include "math.hpp"
#include "random.hpp"

#include <glm/glm.hpp>

namespace rt
{
    class ray
    {
    public:
        constexpr explicit ray(vec3 origin, vec3 direction)
            : origin_(origin)
            , direction_(direction)
        {
        }

        [[nodiscard]]
        constexpr vec3 at(real t) const
        {
            return origin_ + (t * direction_);
        }

        constexpr vec3 origin() const
        {
            return origin_;
        }

        constexpr vec3 direction() const
        {
            return direction_;
        }

    private:
        vec3 origin_;
        vec3 direction_;
    };

    struct camera
    {
        struct create_parameters
        {
            vec3 origin;
            vec3 target;
            vec3 up;
            real vertical_fov;
            real aspect_ratio;
            real aperture;
            real focal_length;
        };

        [[nodiscard]]
        virtual rt::ray shoot_ray_at(real u, real v) const = 0;
        virtual ~camera() = default;
    };

    class pinhole_camera : public camera
    {
    public:
        explicit pinhole_camera(const create_parameters &parameters)
            : origin_(parameters.origin)
        {
            const real optical_power = glm::tan(parameters.vertical_fov / real(2.0));
            const real viewport_height = real(1.0) * optical_power;
            const real viewport_width = parameters.aspect_ratio * viewport_height;

            const vec3 w = glm::normalize(parameters.target - parameters.origin);
            const vec3 u = glm::normalize(glm::cross(w, parameters.up));
            const vec3 v = glm::cross(w, u);

            vertical_ = parameters.focal_length * viewport_height * v;
            horizontal_ = parameters.focal_length * viewport_width * u;
            top_left_ = parameters.origin + (parameters.focal_length * w) - (real(0.5) * horizontal_) - (real(0.5) * vertical_);
        }

        [[nodiscard]]
        rt::ray shoot_ray_at(real u, real v) const override
        {
            const vec3 target = top_left_ + u * horizontal_ + v * vertical_;
            const vec3 direction = target - origin_;
            return rt::ray(origin_, direction);
        }

    private:
        vec3 origin_;
        vec3 vertical_;
        vec3 horizontal_;
        vec3 top_left_;
    };

    class thin_lens_camera : public camera
    {
    public:
        explicit thin_lens_camera(const create_parameters &parameters)
            : origin_(parameters.origin)
        {
            const real optical_power = glm::tan(parameters.vertical_fov / real(2.0));
            const real viewport_height = real(1.0) * optical_power;
            const real viewport_width = parameters.aspect_ratio * viewport_height;

            w_ = glm::normalize(parameters.target - parameters.origin);
            u_ = glm::normalize(glm::cross(w_, parameters.up));
            v_ = glm::cross(w_, u_);

            vertical_ = parameters.focal_length * viewport_height * v_;
            horizontal_ = parameters.focal_length * viewport_width * u_;
            top_left_ = parameters.origin + (parameters.focal_length * w_) - (real(0.5) * horizontal_) - (real(0.5) * vertical_);

            lens_radius_ = real(0.5) * parameters.aperture;
        }

        [[nodiscard]]
        rt::ray shoot_ray_at(real u, real v) const override
        {
            const vec3 target = top_left_ + u * horizontal_ + v * vertical_;

            const vec3 lens_point = lens_radius_ * random_in_unit_disk();
            const vec3 offset = u_ * lens_point.x + v_ * lens_point.y;

            const vec3 origin = origin_ + offset;
            const vec3 direction = target - origin;

            return rt::ray(origin, direction);
        }

    private:
        vec3 origin_;
        vec3 vertical_;
        vec3 horizontal_;
        vec3 top_left_;
        vec3 u_;
        vec3 v_;
        vec3 w_;
        real lens_radius_;
    };
}

#endif // !RAYTRACER_CAMERA_HPP
