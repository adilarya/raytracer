#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "object.h"
#include "math/vec3.h"
#include "core/material.h"
#include "math/normal3.h"
#include "math/point3.h"
#include "math/point2.h"
#include <cmath>

template <typename T>
class Triangle : public Object<T> {
    // attributes
    Point3<T> v1, v2, v3; // vertices
    Vec3<T> vn1, vn2, vn3; // normals
    Point2<T> uv1, uv2, uv3; // texture coordinates
    Material<T> material;
    bool has_vertex_normals = false; // flag to indicate if vertex normals are provided
    bool is_textured = false; // flag to indicate if the triangle is textured
    int texture_idx = -1; // index of the texture to use (if textured)

    public:
        // constructors
        Triangle() : v1(Point3<T>(T(0), T(0), T(0))), v2(Point3<T>(T(1), T(0), T(0))), v3(Point3<T>(T(0), T(1), T(0))), material(Material<T>()) {}
        Triangle(const Point3<T>& v1, const Point3<T>& v2, const Point3<T>& v3, const Material<T>& material) 
            : v1(v1), v2(v2), v3(v3), material(material) {} // for flat triangles
        Triangle(const Point3<T>& v1, const Point3<T>& v2, const Point3<T>& v3, const Vec3<T>& vn1, const Vec3<T>& vn2, const Vec3<T>& vn3, const Material<T>& material) 
            : v1(v1), v2(v2), v3(v3), vn1(vn1), vn2(vn2), vn3(vn3), material(material), has_vertex_normals(true) {} // for smooth triangles
        Triangle(const Point3<T>& v1, const Point3<T>& v2, const Point3<T>& v3, const Point2<T>& uv1, const Point2<T>& uv2, const Point2<T>& uv3, const Material<T>& material, int texture_idx) 
            : v1(v1), v2(v2), v3(v3), uv1(uv1), uv2(uv2), uv3(uv3), material(material), is_textured(true), texture_idx(texture_idx) {} // for textured flat triangles
        Triangle(const Point3<T>& v1, const Point3<T>& v2, const Point3<T>& v3, const Vec3<T>& vn1, const Vec3<T>& vn2, const Vec3<T>& vn3, const Point2<T>& uv1, const Point2<T>& uv2, const Point2<T>& uv3, const Material<T>& material, int texture_idx)
            : v1(v1), v2(v2), v3(v3), vn1(vn1), vn2(vn2), vn3(vn3), uv1(uv1), uv2(uv2), uv3(uv3), material(material), has_vertex_normals(true), is_textured(true), texture_idx(texture_idx) {} // for textured and smooth triangles

        // specialized intersect function
        virtual bool intersect(const Ray<T>& ray, T tmin, T tmax, Hit<T>& hit) const override {
            Vec3<T> edge1 = v2 - v1;
            Vec3<T> edge2 = v3 - v1;
            Vec3<T> h = ray.direction.cross(edge2);
            T a = edge1.dot(h);

            if (std::abs(a) < T(1e-6)) {
                return false; // Ray is parallel to triangle
            }

            T f = T(1.0) / a;
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
                if (has_vertex_normals) { // for smooth triangles.
                    // Interpolate vertex normals using barycentric coordinates
                    Vec3<T> interpolated_normal = (T(1.0) - u - v) * vn1 + u * vn2 + v * vn3;
                    outward_normal = interpolated_normal.normalize();
                }
                hit.set_face_normal(ray, normal_from_vec(outward_normal));
                hit.material = material;

                if (is_textured) {
                    // Interpolate texture coordinates using barycentric coordinates
                    Point2<T> interpolated_uv = (T(1.0) - u - v) * uv1 + u * uv2 + v * uv3;
                    hit.is_textured = true;
                    hit.uv = interpolated_uv;
                    hit.texture_idx = texture_idx;
                } else {
                    hit.is_textured = false;
                    hit.texture_idx = -1;
                }

                return true;
            }

            return false; // No valid intersection within bounds
        }
};

#endif
