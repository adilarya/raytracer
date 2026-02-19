#ifndef CAMERA_H
#define CAMERA_H

#include "math/point3.h"
#include "math/vec3.h"
#include "math/ray.h"
#include <cmath>

template <typename T>
struct Camera {
    // attributes
    Point3<T> eye; // camera pos
    Vec3<T> viewdir; 
    Vec3<T> updir; 
    T vfov; // degrees

    int width, height; // img dims

    // derived attributes
    Vec3<T> forward, right, up; // camera basis vectors
    T focal_length = T(1.0); // default focal length
    T half_h, half_w; 
    
    // parallel option
    bool is_parallel = false;
    T frustum_height = T(0); // only used if is_parallel is true

    // constructor
    Camera() : eye(T(0), T(0), T(0)), viewdir(T(0), T(0), T(-1)), updir(T(0), T(1), T(0)), vfov(T(90)), width(800), height(600), is_parallel(false), frustum_height(T(0)) { // default
        forward = viewdir.normalize();
        right = forward.cross(updir).normalize();
        up = right.cross(forward).normalize();
        // compute half dimensions of the image plane
        T aspect_ratio = static_cast<T>(width) / static_cast<T>(height);
        
        half_h = std::tan((vfov * std::acos(T(-1)) / T(180.0)) / T(2.0)) * focal_length;
        half_w = aspect_ratio * half_h;
    }
    Camera(Point3<T> eye, Vec3<T> viewdir, Vec3<T> updir, T vfov, int width, int height, bool is_parallel, T frustum_height) 
        : eye(eye), viewdir(viewdir), updir(updir), vfov(vfov), width(width), height(height), is_parallel(is_parallel), frustum_height(frustum_height) 
    {
        // compute camera basis vectors
        forward = viewdir.normalize();
        right = forward.cross(updir).normalize();
        up = right.cross(forward).normalize();
        // compute half dimensions of the image plane
        T aspect_ratio = static_cast<T>(width) / static_cast<T>(height);

        if (is_parallel) {
            half_h = frustum_height / T(2);
            half_w = aspect_ratio * half_h;
        } else {
            half_h = std::tan((vfov * std::acos(T(-1)) / T(180.0)) / T(2.0)) * focal_length;
            half_w = aspect_ratio * half_h;
        }        
    }

    // uv based ray generation
    Ray<T> get_ray(int i, int j) const {
        // compute normalized device coordinates (NDC)
        T u = (i + T(0.5)) / static_cast<T>(width);
        T v = (j + T(0.5)) / static_cast<T>(height);
        // compute offsets in camera space
        Vec3<T> offset_u = right * ((T(2.0) * u - T(1.0)) * half_w);
        Vec3<T> offset_v = up * ((T(1.0) - T(2.0) * v) * half_h); // flip v to match image coordinates
        // compute ray direction
        if (is_parallel) {
            return Ray<T>(eye + offset_u + offset_v, forward);
        }
        Vec3<T> ray_dir = (forward * focal_length + offset_u + offset_v).normalize();
        return Ray<T>(eye, ray_dir);
    }
};

#endif
