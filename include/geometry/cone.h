#ifndef CONE_H
#define CONE_H

#include "object.h"
#include "math/vec3.h"
#include "math/normal3.h"
#include "math/point3.h"
#include "math/ray.h"
#include "core/hit.h"
#include "core/material.h"

#include <cmath>

template <typename T>
class Cone : public Object<T> {
    // attributes
    Point3<T> tip;
    Vec3<T> direction; // of central axis
    T angle; // radians
    T height;
    Material<T> material;
    bool is_textured = false; // flag to indicate if the cone is textured
    int texture_idx = -1; // index of the texture to use (if textured)

    public:
        // constructors
        Cone() : tip(Point3<T>(T(0), T(0), T(0))), direction(Vec3<T>(T(0), T(1), T(0))), angle(T(45) * std::acos(T(-1)) / T(180)), height(T(1)), material(Material<T>()), is_textured(false), texture_idx(-1) {}
        Cone(const Point3<T>& tip, const Vec3<T>& direction, T angle, T height, const Material<T>& material) 
            : tip(tip), direction(direction.normalize()), angle(angle), height(height), material(material), is_textured(false), texture_idx(-1) {}
        Cone(const Point3<T>& tip, const Vec3<T>& direction, T angle, T height, const Material<T>& material, int texture_idx)
            : tip(tip), direction(direction.normalize()), angle(angle), height(height), material(material), is_textured(true), texture_idx(texture_idx) {}

        // specialized intersect function
        virtual bool intersect(const Ray<T>& ray, T tmin, T tmax, Hit<T>& hit) const override {
            // cylinder like workflow
            bool hit_any = false;
            T closest = tmax;

            Vec3<T> A = direction;
            T cosT = std::cos(angle);
            T cosT2 = cosT * cosT;
            
            Vec3<T> oc = ray.origin - tip;
            Vec3<T> R = ray.direction;

            T dDotA = R.dot(A);
            T ocDotA = oc.dot(A);
            T dDotOc = R.dot(oc);
            T ocDotOc = oc.dot(oc);
            T dDotD = R.dot(R);

            // coefficients
            T a = dDotA * dDotA - cosT2 * dDotD;
            T b = T(2) * (ocDotA * dDotA - cosT2 * dDotOc);
            T c = ocDotA * ocDotA - cosT2 * ocDotOc;

            Vec3<T> helper = (std::abs(A.x) < T(0.9)) ? Vec3<T>(T(1), T(0), T(0))
                                                : Vec3<T>(T(0), T(1), T(0));
            Vec3<T> tangent = A.cross(helper).normalize();
            Vec3<T> bitangent = A.cross(tangent).normalize();

            // SIDE INTERSECTIONS
            T eps = T(1e-6);
            if (std::abs(a) >= eps) {
                T discriminant = b * b - T(4) * a * c;
                if (discriminant >= 0) {
                    T sqrt_disc = std::sqrt(discriminant);
                    T t0 = (-b - sqrt_disc) / (T(2) * a);
                    T t1 = (-b + sqrt_disc) / (T(2) * a);

                    if (tmin <= t0 && t0 <= closest) {
                        Point3<T> p = ray.at(t0);
                        T m = A.dot(p - tip);
                        if (T(0) <= m && m <= height) {
                            if (!hit_any || t0 < closest) {
                                hit.t = t0;
                                hit.point = p;

                                Vec3<T> v = p - tip;
                                Vec3<T> normal = m * A - v * cosT2;
                                normal = normal.normalize();
                                hit.set_face_normal(ray, normal_from_vec(normal));
                                hit.material = material;
                                hit_any = true;
                                closest = t0;

                                if (is_textured) {
                                    // compute texture coordinates for the hit point on the cone surface
                                    Vec3<T> radial = (v - A * m).normalize(); // vector from axis to hit point, lies in the cone surface plane
                                    T phi = std::atan2(radial.dot(tangent), radial.dot(bitangent));
                                    T u = (phi + std::acos(T(-1))) / (T(2) * std::acos(T(-1)));
                                    T v_coord = m / height;
                                    hit.uv = Point2<T>(u, v_coord);
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
                        Point3<T> p = ray.at(t1);
                        T m = A.dot(p - tip);
                        if (T(0) <= m && m <= height) {
                            if (!hit_any || t1 < closest) {
                                hit.t = t1;
                                hit.point = p;

                                Vec3<T> v = p - tip;
                                Vec3<T> normal = m * A - v * cosT2;
                                normal = normal.normalize();
                                hit.set_face_normal(ray, normal_from_vec(normal));
                                hit.material = material;
                                hit_any = true;
                                closest = t1;

                                if (is_textured) {
                                    // compute texture coordinates for the hit point on the cone surface
                                    Vec3<T> radial = (v - A * m).normalize(); // vector from axis to hit point, lies in the cone surface plane
                                    T phi = std::atan2(radial.dot(tangent), radial.dot(bitangent));
                                    T u = (phi + std::acos(T(-1))) / (T(2) * std::acos(T(-1)));
                                    T v_coord = m / height;
                                    hit.uv = Point2<T>(u, v_coord);
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

            // CAP INTERSECTION
            Point3<T> base = tip + A * height;
            T r = height * std::tan(angle);
            T r2 = r * r;

            T den = R.dot(A);
            T eps2 = T(1e-8);
            if (std::abs(den) >= eps2) {
                T tcap = (base - ray.origin).dot(A) / den;
                if (tmin <= tcap && tcap <= closest) {
                    Point3<T> pcap = ray.at(tcap);
                    Vec3<T> d = pcap - base;

                    if (d.dot(d) <= r2 + eps2) {
                        hit.t = tcap;
                        hit.point = pcap;

                        hit.set_face_normal(ray, normal_from_vec(A));
                        hit.material = material;
                        hit_any = true;
                        closest = tcap;

                        if (is_textured) {
                            // compute texture coordinates for the hit point on the cone base
                            Vec3<T> radial = pcap - base;
                            T u = radial.dot(tangent) / (T(2) * r) + T(0.5);
                            T v_coord = radial.dot(bitangent) / (T(2) * r) + T(0.5);
                            hit.uv = Point2<T>(u, v_coord);
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

            return hit_any;
        }
};

#endif
