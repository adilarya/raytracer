#ifndef CYLINDER_H
#define CYLINDER_H

#include "object.h"
#include "math/vec3.h"
#include "core/material.h"
#include "math/normal3.h"
#include "math/point3.h"
#include <cmath>

template <typename T>
class Cylinder : public Object<T> {
    // attributes
    Point3<T> center; 
    Vec3<T> direction;
    T radius;
    T length;
    Material<T> material; 

    public:
        // constructors
        Cylinder() : center(Point3<T>(0, 0, 0)), direction(Vec3<T>(0, 1, 0)), radius(1), length(1), material(Material<T>()) {}
        Cylinder(const Point3<T>& center, const Vec3<T>& direction, T radius, T length, const Material<T>& material) 
            : center(center), direction(direction.normalize()), radius(radius), length(length), material(material) {}

        // specialized intersect function
        virtual bool intersect(const Ray<T>& ray, T tmin, T tmax, Hit<T>& hit) const override {
            bool hit_any = false;
            T closest = tmax;

            Point3<T> A = center;
            Vec3<T> u = direction;
            Vec3<T> AO = ray.origin - A;

            Vec3<T> D_perp = ray.direction - u * ray.direction.dot(u);
            Vec3<T> AO_perp = AO - u * AO.dot(u);

            // quadratic coefficients
            T a = D_perp.dot(D_perp);
            T b = 2 * AO_perp.dot(D_perp);
            T c = AO_perp.dot(AO_perp) - radius * radius;

            // SIDE INTERSECTIONS
            T eps = T(1e-6);
            if (std::abs(a) >= eps) {
                T discriminant = b * b - 4 * a * c;
                if (discriminant >= 0) {
                    T sqrt_disc = std::sqrt(discriminant);
                    T t0 = (-b - sqrt_disc) / (2 * a);
                    T t1 = (-b + sqrt_disc) / (2 * a);

                    if (tmin <= t0 && t0 <= closest) {
                        T s = (ray.at(t0) - A).dot(u);
                        if (-eps <= s && s <= length + eps) {
                            if (!hit_any || t0 < closest) {
                                hit.t = t0;
                                hit.point = ray.at(t0);
                                Vec3<T> outward_normal = (hit.point - A - u * s) / radius;
                                hit.set_face_normal(ray, normal_from_vec(outward_normal));
                                hit.material = material;
                                hit_any = true;
                                closest = t0;
                            }
                        }
                    }

                    if (tmin <= t1 && t1 <= closest) {
                        T s = (ray.at(t1) - A).dot(u);
                        if (-eps <= s && s <= length + eps) {
                            if (!hit_any || t1 < closest) {
                                hit.t = t1;
                                hit.point = ray.at(t1);
                                Vec3<T> outward_normal = (hit.point - A - u * s) / radius;
                                hit.set_face_normal(ray, normal_from_vec(outward_normal));
                                hit.material = material;
                                hit_any = true;
                                closest = t1;
                            }
                        }
                    }
                }
                
            }

            // CAP INTERSECTIONS
            Point3<T> B = A + u * length;
            T den = ray.direction.dot(u);
            if (std::abs(den) >= eps) {
                // Cap B
                T t = ((B - ray.origin).dot(u)) / den;
                if (tmin <= t && t <= closest) {
                    Point3<T> p = ray.at(t);
                    Vec3<T> w = p - B;
                    Vec3<T> w_perp = w - u * w.dot(u);

                    if (w_perp.dot(w_perp) <= radius * radius) {
                        if (!hit_any || t < closest) {
                            hit.t = t;
                            hit.point = p;
                            hit.set_face_normal(ray, normal_from_vec(u));
                            hit.material = material;
                            hit_any = true;
                            closest = t;
                        }
                    }
                }
                
                // Cap A
                t = ((A - ray.origin).dot(u)) / den;
                if (tmin <= t && t <= closest) {
                    Point3<T> p = ray.at(t);
                    Vec3<T> w = p - A;
                    Vec3<T> w_perp = w - u * w.dot(u);

                    if (w_perp.dot(w_perp) <= radius * radius) {
                        if (!hit_any || t < closest) {
                            hit.t = t;
                            hit.point = p;
                            hit.set_face_normal(ray, normal_from_vec(-u));
                            hit.material = material;
                            hit_any = true;
                            closest = t;
                        }
                    }
                }
            }

            return hit_any; // No valid intersection within bounds
        }
};

#endif
