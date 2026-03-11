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
    int texture_idx = -1; // index of the texture to use (if textured)
    int bump_map_idx = -1; // index of the bump map to use (if has bump map)

    public:
        // constructors
        Cylinder() : center(Point3<T>(T(0), T(0), T(0))), direction(Vec3<T>(T(0), T(1), T(0))), radius(T(1)), length(T(1)), material(Material<T>()), texture_idx(-1), bump_map_idx(-1) {}
        Cylinder(const Point3<T>& center, const Vec3<T>& direction, T radius, T length, const Material<T>& material, int texture_idx = -1, int bump_map_idx = -1) 
            : center(center), direction(direction.normalize()), radius(radius), length(length), material(material), texture_idx(texture_idx), bump_map_idx(bump_map_idx) {}

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

                                hit.texture_idx = texture_idx;
                                hit.bump_map_idx = bump_map_idx;

                                if (texture_idx >= 0 || bump_map_idx >= 0) {
                                    // compute texture coordinates for the hit point on the cylinder
                                    Vec3<T> radial = (hit.point - A - axis * s).normalize();
                                    T phi = std::atan2(radial.dot(bitangent), radial.dot(tangent));
                                    if (phi < T(0)) phi += T(2) * std::acos(T(-1));

                                    T u_coord = phi / (T(2) * std::acos(T(-1)));
                                    T v_coord = s / length;
                                    hit.uv = Point2<T>(u_coord, v_coord);

                                    // compute tangent and bitangent for normal mapping
                                    Vec3<T> Tside = (-std::sin(phi)) * tangent + (std::cos(phi)) * bitangent;

                                    // Bitangent = direction of increasing v along the axis
                                    Vec3<T> Bside = axis;

                                    Vec3<T> N = hit.normal.toVec3();
                                    Tside = (Tside - N * Tside.dot(N)).normalize();
                                    Bside = (Bside - N * Bside.dot(N)).normalize();

                                    hit.tangent = Tside;
                                    hit.bitangent = Bside;
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

                                hit.texture_idx = texture_idx;
                                hit.bump_map_idx = bump_map_idx;

                                if (texture_idx >= 0 || bump_map_idx >= 0) {
                                    // compute texture coordinates for the hit point on the cylinder
                                    Vec3<T> radial = (hit.point - A - axis * s).normalize();
                                    T phi = std::atan2(radial.dot(bitangent), radial.dot(tangent));
                                    if (phi < T(0)) phi += T(2) * std::acos(T(-1));

                                    T u_coord = phi / (T(2) * std::acos(T(-1)));
                                    T v_coord = s / length;
                                    hit.uv = Point2<T>(u_coord, v_coord);

                                    Vec3<T> Tside = (-std::sin(phi)) * tangent + (std::cos(phi)) * bitangent;

                                    // Bitangent = direction of increasing v along the axis
                                    Vec3<T> Bside = axis;

                                    Vec3<T> N = hit.normal.toVec3();
                                    Tside = (Tside - N * Tside.dot(N)).normalize();
                                    Bside = (Bside - N * Bside.dot(N)).normalize();

                                    hit.tangent = Tside;
                                    hit.bitangent = Bside;
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

                            hit.texture_idx = texture_idx;
                            hit.bump_map_idx = bump_map_idx;

                            if (texture_idx >= 0 || bump_map_idx >= 0) {
                                // compute texture coordinates for the hit point on the cylinder cap
                                Vec3<T> radial = w_perp; // already lies in the cap plane
                                T x = radial.dot(tangent);
                                T y = radial.dot(bitangent);

                                T u_coord = T(0.5) + x / (T(2) * radius);
                                T v_coord = T(0.5) + y / (T(2) * radius);
                                hit.uv = Point2<T>(u_coord, v_coord);

                                Vec3<T> N = hit.normal.toVec3();

                                // Tangent/bitangent on the cap plane
                                Vec3<T> Tcap = tangent;
                                Tcap = (Tcap - N * Tcap.dot(N)).normalize();
                                Vec3<T> Bcap = N.cross(Tcap).normalize();

                                hit.tangent = Tcap;
                                hit.bitangent = Bcap;
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

                            hit.texture_idx = texture_idx;
                            hit.bump_map_idx = bump_map_idx;

                            if (texture_idx >= 0 || bump_map_idx >= 0) {
                                // compute texture coordinates for the hit point on the cylinder cap
                                Vec3<T> radial = w_perp; // already lies in the cap plane
                                T x = radial.dot(tangent);
                                T y = radial.dot(bitangent);

                                T u_coord = T(0.5) + x / (T(2) * radius);
                                T v_coord = T(0.5) + y / (T(2) * radius);
                                hit.uv = Point2<T>(u_coord, v_coord);

                                Vec3<T> N = hit.normal.toVec3();

                                // Tangent/bitangent on the cap plane
                                Vec3<T> Tcap = tangent;
                                Tcap = (Tcap - N * Tcap.dot(N)).normalize();
                                Vec3<T> Bcap = N.cross(Tcap).normalize();

                                hit.tangent = Tcap;
                                hit.bitangent = Bcap;
                            } 
                        }
                    }
                }
            }

            return hit_any; // No valid intersection within bounds
        }
};

#endif
