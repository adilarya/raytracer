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
    int texture_idx = -1; // index of the texture to use (if textured)
    int bump_map_idx = -1; // index of the bump map to use (if has bump map)

    public:
        // constructors
        Sphere() : center(Point3<T>(0, 0, 0)), radius(1), material(Material<T>()), texture_idx(-1), bump_map_idx(-1) {}
        Sphere(const Point3<T>& center, T radius, const Material<T>& material, int texture_idx = -1, int bump_map_idx = -1) 
            : center(center), radius(radius), material(material), texture_idx(texture_idx), bump_map_idx(bump_map_idx) {}

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
            hit.texture_idx = texture_idx;
            hit.bump_map_idx = bump_map_idx;

            if (texture_idx >= 0 || bump_map_idx >= 0) {
                // compute texture coordinates for the hit point on the sphere
                T pi = std::acos(T(-1));
                T phi = std::atan2(outward_normal.y, outward_normal.x);
                if (phi < T(0)) phi += T(2) * pi;
                T theta = std::acos(std::clamp(outward_normal.z, T(-1), T(1)));
                T u = phi / (T(2) * pi);
                T v = theta / pi;
                hit.uv = Point2<T>(u, v);

                // compute tangent and bitangent for normal mapping
                Vec3<T> tangent(-std::sin(phi), std::cos(phi), T(0));

                // Bitangent = direction of increasing v (polar angle)
                Vec3<T> bitangent(
                    std::cos(phi) * -std::sin(theta),
                    std::sin(phi) * -std::sin(theta),
                    std::cos(theta)
                );

                // Make the frame consistent with the shading normal
                Vec3<T> N = hit.normal.toVec3();
                tangent = (tangent - N * tangent.dot(N)).normalize();
                bitangent = N.cross(tangent).normalize();

                hit.tangent = tangent;
                hit.bitangent = bitangent;
            } 

            return true;
        }
};

#endif
