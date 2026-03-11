#ifndef CYLINDER_H
#define CYLINDER_H

#include "object.h"
#include "math/vec3.h"
#include "core/material.h"
#include "math/normal3.h"
#include "math/point3.h"
#include "math/point2.h"
#include <cmath>

template <typename T>
class Cylinder : public Object<T> {
    // attributes
    Point3<T> center; 
    Vec3<T> direction;
    T radius;
    T length;
    Material<T> material; 
    bool is_textured = false; // flag to indicate if the cylinder is textured
    int texture_idx = -1; // index of the texture to use (if textured)

    public:
        // constructors
        Cylinder() : center(Point3<T>(T(0), T(0), T(0))), direction(Vec3<T>(T(0), T(1), T(0))), radius(T(1)), length(T(1)), material(Material<T>()), is_textured(false), texture_idx(-1) {}
        Cylinder(const Point3<T>& center, const Vec3<T>& direction, T radius, T length, const Material<T>& material) 
            : center(center), direction(direction.normalize()), radius(radius), length(length), material(material), is_textured(false), texture_idx(-1) {}
        Cylinder(const Point3<T>& center, const Vec3<T>& direction, T radius, T length, const Material<T>& material, int texture_idx) 
            : center(center), direction(direction.normalize()), radius(radius), length(length), material(material), is_textured(true), texture_idx(texture_idx) {}

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

            Vec3<T> axis = direction.normalize();
            Vec3<T> helper = (std::abs(axis.x) < T(0.9))
                ? Vec3<T>(T(1), T(0), T(0))
                : Vec3<T>(T(0), T(1), T(0));
            Vec3<T> tangent = axis.cross(helper).normalize();
            Vec3<T> bitangent = axis.cross(tangent).normalize();

            // SIDE INTERSECTIONS
            T eps = T(1e-6);
            if (std::abs(a) >= eps) {
                T discriminant = b * b - T(4) * a * c;
                if (discriminant >= 0) {
                    T sqrt_disc = std::sqrt(discriminant);
                    T t0 = (-b - sqrt_disc) / (T(2) * a);
                    T t1 = (-b + sqrt_disc) / (T(2) * a);

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

                                if (is_textured) {
                                    // compute texture coordinates for the hit point on the cylinder
                                    Vec3<T> radial = (hit.point - A - axis * s).normalize();
                                    T phi = std::atan2(radial.dot(bitangent), radial.dot(tangent));
                                    if (phi < T(0)) phi += T(2) * std::acos(T(-1));

                                    T u_coord = phi / (T(2) * std::acos(T(-1)));
                                    T v_coord = s / length;
                                    hit.uv = Point2<T>(u_coord, v_coord);
                                    hit.texture_idx = texture_idx;
                                    hit.is_textured = true;
                                } else {
                                    hit.uv = Point2<T>(0, 0); // default texture coordinates if not textured
                                    hit.texture_idx = -1; // no texture
                                    hit.is_textured = false;
                                }
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

                                if (is_textured) {
                                    // compute texture coordinates for the hit point on the cylinder
                                    Vec3<T> radial = (hit.point - A - axis * s).normalize();
                                    T phi = std::atan2(radial.dot(bitangent), radial.dot(tangent));
                                    if (phi < T(0)) phi += T(2) * std::acos(T(-1));

                                    T u_coord = phi / (T(2) * std::acos(T(-1)));
                                    T v_coord = s / length;
                                    hit.uv = Point2<T>(u_coord, v_coord);
                                    hit.texture_idx = texture_idx;
                                    hit.is_textured = true;
                                } else {
                                    hit.uv = Point2<T>(0, 0); // default texture coordinates if not textured
                                    hit.texture_idx = -1; // no texture
                                    hit.is_textured = false;
                                }
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

                            if (is_textured) {
                                // compute texture coordinates for the hit point on the cylinder cap
                                Vec3<T> radial = w_perp; // already lies in the cap plane
                                T x = radial.dot(tangent);
                                T y = radial.dot(bitangent);

                                T u_coord = T(0.5) + x / (T(2) * radius);
                                T v_coord = T(0.5) + y / (T(2) * radius);
                                hit.uv = Point2<T>(u_coord, v_coord);
                                hit.texture_idx = texture_idx;
                                hit.is_textured = true;
                            } else {
                                hit.uv = Point2<T>(0, 0); // default texture coordinates if not textured
                                hit.texture_idx = -1; // no texture
                                hit.is_textured = false;
                            }
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

                            if (is_textured) {
                                // compute texture coordinates for the hit point on the cylinder cap
                                Vec3<T> radial = w_perp; // already lies in the cap plane
                                T x = radial.dot(tangent);
                                T y = radial.dot(bitangent);

                                T u_coord = T(0.5) + x / (T(2) * radius);
                                T v_coord = T(0.5) + y / (T(2) * radius);
                                hit.uv = Point2<T>(u_coord, v_coord);
                                hit.texture_idx = texture_idx;
                                hit.is_textured = true;
                            } else {
                                hit.uv = Point2<T>(0, 0); // default texture coordinates if not textured
                                hit.texture_idx = -1; // no texture
                                hit.is_textured = false;
                            }
                        }
                    }
                }
            }

            return hit_any; // No valid intersection within bounds
        }
};

#endif
