#ifndef SPHERE_H
#define SPHERE_H

#include "object.h"
#include "math/vec3.h"
#include "core/material.h"
#include "math/normal3.h"
#include <cmath>

template <typename T>
class Sphere : public Object<T> {
    // attributes
    Point3<T> center; // center
    T radius; // radius
    Material<T> material; // material

    public:
        // constructors
        Sphere() : center(Point3<T>(0, 0, 0)), radius(1), material(Material<T>()) {}
        Sphere(const Point3<T>& center, T radius, const Material<T>& material) : center(center), radius(radius), material(material) {}

        // specialized intersect function
        virtual bool intersect(const Ray<T>& ray, T tmin, T tmax, Hit<T>& hit) const override {
            // ray-sphere intersection algorithm
            Vec3<T> oc = ray.origin - center;
            T a = ray.direction.dot(ray.direction);
            T b = 2.0 * oc.dot(ray.direction);
            T c = oc.dot(oc) - radius * radius;
            T discriminant = b * b - 4 * a * c;
            if (discriminant < 0) {
                return false;
            } else {
                T t = (-b - std::sqrt(discriminant)) / (2.0 * a);
                if (t < tmin || t > tmax) {
                    t = (-b + std::sqrt(discriminant)) / (2.0 * a);
                    if (t < tmin || t > tmax) {
                        return false;
                    }
                }
                hit.t = t;
                hit.point = ray.at(t);
                Vec3<T> outward_normal = (hit.point - center) / radius;
                hit.set_face_normal(ray, normal_from_vec(outward_normal));
                hit.material = material;
                return true;
            }
        }
};

#endif
