#ifndef ELLIPSOID_H
#define ELLIPSOID_H

#include "object.h"
#include "math/vec3.h"
#include "core/material.h"
#include "math/normal3.h"
#include "math/point2.h"
#include <cmath>

template <typename T>
class Ellipsoid : public Object<T> {
    // attributes
    Point3<T> center;
    Vec3<T> radii; // (rx, ry, rz)
    Material<T> material;
    bool is_textured = false; // flag to indicate if the ellipsoid is textured
    int texture_idx = -1; // index of the texture to use (if textured)

    public:
        // constructors
        Ellipsoid() : center(Point3<T>(T(0), T(0), T(0))), radii(Vec3<T>(T(1), T(1), T(1))), material(Material<T>()), is_textured(false), texture_idx(-1) {}
        Ellipsoid(const Point3<T>& center, const Vec3<T>& radii, const Material<T>& material) 
            : center(center), radii(radii), material(material), is_textured(false), texture_idx(-1) {}
        Ellipsoid(const Point3<T>& center, const Vec3<T>& radii, const Material<T>& material, int texture_idx)
            : center(center), radii(radii), material(material), is_textured(true), texture_idx(texture_idx) {}

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

            if (is_textured) {
                Vec3<T> sphere_p(pc.x / radii.x, pc.y / radii.y, pc.z / radii.z);
                sphere_p = sphere_p.normalize();

                T pi = std::acos(T(-1));
                T phi = std::atan2(sphere_p.z, sphere_p.x);
                T u = (phi + pi) / (T(2) * pi);
                T v = std::acos(sphere_p.y) / pi;

                hit.uv = Point2<T>(u, v);
                hit.texture_idx = texture_idx;
                hit.is_textured = true;
            } else {
                hit.uv = Point2<T>(0, 0);
                hit.texture_idx = -1;
                hit.is_textured = false;
            }

            return true;
        }
};

#endif
