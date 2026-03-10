#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "object.h"
#include "math/vec3.h"
#include "core/material.h"
#include "math/normal3.h"
#include "math/point3.h"
#include <cmath>

template <typename T>
class Triangle : public Object<T> {
    // attributes
    Point3<T> v1, v2, v3; // vertices
    Material<T> material; 

    public:
        // constructors
        Triangle() : v1(Point3<T>(T(0), T(0), T(0))), v2(Point3<T>(T(1), T(0), T(0))), v3(Point3<T>(T(0), T(1), T(0))), material(Material<T>()) {}
        Triangle(const Point3<T>& v1, const Point3<T>& v2, const Point3<T>& v3, const Material<T>& material) 
            : v1(v1), v2(v2), v3(v3), material(material) {}

        // specialized intersect function
        virtual bool intersect(const Ray<T>& ray, T tmin, T tmax, Hit<T>& hit) const override {
            Vec3<T> edge1 = v2 - v1;
            Vec3<T> edge2 = v3 - v1;
            Vec3<T> h = ray.direction.cross(edge2);
            T a = edge1.dot(h);

            if (std::abs(a) < T(1e-6)) {
                return false; // Ray is parallel to triangle
            }

            T f = 1.0 / a;
            Vec3<T> s = ray.origin - v1;
            T u = f * s.dot(h);

            if (u < T(0.0) || u > T(1.0)) {
                return false; // Intersection is outside the triangle
            }

            Vec3<T> q = s.cross(edge1);
            T v = f * ray.direction.dot(q);

            if (v < T(0.0) || u + v > T(1.0)) {
                return false; // Intersection is outside the triangle
            }

            // At this stage we can compute t to find out where the intersection point is on the line
            T t = f * edge2.dot(q);

            if (tmin <= t && t <= tmax) {
                hit.t = t;
                hit.point = ray.at(t);
                Vec3<T> outward_normal = edge1.cross(edge2).normalize();
                hit.set_face_normal(ray, normal_from_vec(outward_normal));
                hit.material = material;
                return true;
            }

            return false; // No valid intersection within bounds
        }
};

#endif
