#ifndef ELLIPSOID_H
#define ELLIPSOID_H

#include "object.h"
#include "math/vec3.h"
#include "core/material.h"
#include "math/normal3.h"
#include "math/point2.h"
#include <cmath>
#include <algorithm>

template <typename T>
class Ellipsoid : public Object<T> {
    // attributes
    Point3<T> center;
    Vec3<T> radii; // (rx, ry, rz)
    Material<T> material;
    int texture_idx = -1; // index of the texture to use (if textured)
    int bump_map_idx = -1; // index of the bump map to use (if has bump map)

    public:
        // constructors
        Ellipsoid() : center(Point3<T>(T(0), T(0), T(0))), radii(Vec3<T>(T(1), T(1), T(1))), material(Material<T>()), texture_idx(-1), bump_map_idx(-1) {}
        Ellipsoid(const Point3<T>& center, const Vec3<T>& radii, const Material<T>& material, int texture_idx = -1, int bump_map_idx = -1) 
            : center(center), radii(radii), material(material), texture_idx(texture_idx), bump_map_idx(bump_map_idx) {}

        // specialized intersect function
        virtual bool intersect(const Ray<T>& ray, T tmin, T tmax, Hit<T>& hit) const override {
            Vec3<T> oc = ray.origin - center;
            Vec3<T> ocp(oc.x / radii.x, oc.y / radii.y, oc.z / radii.z);
            Vec3<T> dp(ray.direction.x / radii.x, ray.direction.y / radii.y, ray.direction.z / radii.z);

            // coefficients
            T a = dp.dot(dp);
            T b = T(2.0) * ocp.dot(dp);
            T c = ocp.dot(ocp) - T(1);

            // discriminant
            T discriminant = b * b - T(4) * a * c;
            if (discriminant < 0) {
                return false;
            } 

            T t = (-b - std::sqrt(discriminant)) / (T(2) * a);
            if (t < tmin || t > tmax) {
                t = (-b + std::sqrt(discriminant)) / (T(2) * a);
                if (t < tmin || t > tmax) {
                    return false;
                }
            }
            hit.t = t;
            hit.point = ray.at(t);
            Vec3<T> pc = hit.point - center;
            Vec3<T> outward_normal((pc.x / (radii.x * radii.x)), (pc.y / (radii.y * radii.y)), (pc.z / (radii.z * radii.z)));
            hit.set_face_normal(ray, normal_from_vec(outward_normal.normalize()));
            hit.material = material;
            hit.texture_idx = texture_idx;
            hit.bump_map_idx = bump_map_idx;

            if (texture_idx >= 0 || bump_map_idx >= 0) {
                Vec3<T> sphere_p(pc.x / radii.x, pc.y / radii.y, pc.z / radii.z);
                sphere_p = sphere_p.normalize();

                T pi = std::acos(T(-1));
                T phi = std::atan2(sphere_p.y, sphere_p.x);
                if (phi < T(0)) phi += T(2) * pi;
                T theta = std::acos(std::clamp(sphere_p.z, T(-1), T(1)));
                T u = phi / (T(2) * pi);
                T v = theta / pi;

                hit.uv = Point2<T>(u, v);

                Vec3<T> tangent(-std::sin(phi), std::cos(phi), T(0));

                // Use the actual ellipsoid shading normal for consistency
                Vec3<T> N = hit.normal.toVec3();
                tangent = (tangent - N * tangent.dot(N)).normalize();
                Vec3<T> bitangent = N.cross(tangent).normalize();

                hit.tangent = tangent;
                hit.bitangent = bitangent;
            } 

            return true;
        }
};

#endif
