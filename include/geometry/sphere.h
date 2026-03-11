#ifndef SPHERE_H
#define SPHERE_H

#include "object.h"
#include "math/vec3.h"
#include "core/material.h"
#include "math/normal3.h"
#include "math/point2.h"
#include <cmath>
#include <algorithm>

template <typename T>
class Sphere : public Object<T> {
    // attributes
    Point3<T> center; // center
    T radius; // radius
    Material<T> material; // material
    bool is_textured = false; // flag to indicate if the sphere is textured
    int texture_idx = -1; // index of the texture to use (if textured)

    public:
        // constructors
        Sphere() : center(Point3<T>(0, 0, 0)), radius(1), material(Material<T>()), is_textured(false), texture_idx(-1) {}
        Sphere(const Point3<T>& center, T radius, const Material<T>& material) : center(center), radius(radius), material(material), is_textured(false), texture_idx(-1) {}
        Sphere(const Point3<T>& center, T radius, const Material<T>& material, int texture_idx) : center(center), radius(radius), material(material), is_textured(true), texture_idx(texture_idx) {}

        // specialized intersect function
        virtual bool intersect(const Ray<T>& ray, T tmin, T tmax, Hit<T>& hit) const override {
            // ray-sphere intersection algorithm
            Vec3<T> oc = ray.origin - center;
            T a = ray.direction.dot(ray.direction);
            T b = T(2.0) * oc.dot(ray.direction);
            T c = oc.dot(oc) - radius * radius;
            T discriminant = b * b - T(4) * a * c;
            if (discriminant < 0) {
                return false;
            }
            
            T t = (-b - std::sqrt(discriminant)) / (T(2.0) * a);
            if (t < tmin || t > tmax) {
                t = (-b + std::sqrt(discriminant)) / (T(2.0) * a);
                if (t < tmin || t > tmax) {
                    return false;
                }
            }
            hit.t = t;
            hit.point = ray.at(t);
            Vec3<T> outward_normal = (hit.point - center) / radius;
            hit.set_face_normal(ray, normal_from_vec(outward_normal));
            hit.material = material;

            if (is_textured) {
                // compute texture coordinates for the hit point on the sphere
                T pi = std::acos(T(-1));
                T phi = std::atan2(outward_normal.y, outward_normal.x);
                if (phi < T(0)) phi += T(2) * pi;
                T theta = std::acos(std::clamp(outward_normal.z, T(-1), T(1)));
                T u = phi / (T(2) * pi);
                T v = theta / pi;
                hit.uv = Point2<T>(u, v);
                hit.texture_idx = texture_idx;
                hit.is_textured = true;
            } else {
                hit.uv = Point2<T>(0, 0); // default texture coordinates if not textured
                hit.texture_idx = -1; // no texture
                hit.is_textured = false;
            }

            return true;
        }
};

#endif
