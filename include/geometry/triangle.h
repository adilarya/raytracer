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
    Material<T> material;
    Vec3<T> vn1, vn2, vn3; // normals
    Point2<T> uv1, uv2, uv3; // texture coordinates
    bool has_vertex_normals = false; // flag to indicate if vertex normals are provided
    int texture_idx = -1; // index of the texture to use (if textured)
    int bump_map_idx = -1; // index of the bump map to use (if has bump map)

    public:
        // constructors
        Triangle() : v1(Point3<T>(T(0), T(0), T(0))), v2(Point3<T>(T(1), T(0), T(0))), v3(Point3<T>(T(0), T(1), T(0))), material(Material<T>()) {}
        Triangle(const Point3<T>& v1, const Point3<T>& v2, const Point3<T>& v3, const Material<T>& material,
            const Vec3<T>& vn1 = Vec3<T>(T(0), T(0), T(0)), const Vec3<T>& vn2 = Vec3<T>(T(0), T(0), T(0)), const Vec3<T>& vn3 = Vec3<T>(T(0), T(0), T(0)), 
            const Point2<T>& uv1 = Point2<T>(T(0), T(0)), const Point2<T>& uv2 = Point2<T>(T(0), T(0)), const Point2<T>& uv3 = Point2<T>(T(0), T(0)), 
            bool has_vertex_normals = false, int texture_idx = -1, int bump_map_idx = -1)
            : v1(v1), v2(v2), v3(v3), material(material),
            vn1(vn1), vn2(vn2), vn3(vn3), 
            uv1(uv1), uv2(uv2), uv3(uv3), 
            has_vertex_normals(has_vertex_normals), 
            texture_idx(texture_idx), bump_map_idx(bump_map_idx) {} 

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
                hit.bump_map_idx = bump_map_idx;
                hit.texture_idx = texture_idx;

                if (texture_idx >= 0 || bump_map_idx >= 0) {
                    // Interpolate texture coordinates using barycentric coordinates
                    hit.uv = (T(1.0) - u - v) * uv1 + u * uv2 + v * uv3;
                    // Compute tangent and bitangent from geometry + UVs
                    Point2<T> duv1 = uv2 - uv1;
                    Point2<T> duv2 = uv3 - uv1;

                    T det = duv1.x * duv2.y - duv1.y * duv2.x;

                    if (std::abs(det) > T(1e-8)) {
                        T invDet = T(1.0) / det;

                        Vec3<T> tangent =
                            (edge1 * duv2.y - edge2 * duv1.y) * invDet;

                        Vec3<T> bitangent =
                            (edge2 * duv1.x - edge1 * duv2.x) * invDet;

                        // Orthonormalize against the shading normal
                        Vec3<T> N = hit.normal.toVec3();
                        tangent = (tangent - N * tangent.dot(N)).normalize();

                        // Make bitangent consistent with tangent and normal
                        bitangent = N.cross(tangent).normalize();

                        hit.tangent = tangent;
                        hit.bitangent = bitangent;
                    } else {
                        // Fallback basis if UVs are degenerate
                        Vec3<T> N = hit.normal.toVec3();
                        Vec3<T> helper = (std::abs(N.x) < T(0.9))
                            ? Vec3<T>(T(1), T(0), T(0))
                            : Vec3<T>(T(0), T(1), T(0));

                        hit.tangent = (helper - N * helper.dot(N)).normalize();
                        hit.bitangent = N.cross(hit.tangent).normalize();
                    }
                }

                return true;
            }

            return false; // No valid intersection within bounds
        }
};

#endif
