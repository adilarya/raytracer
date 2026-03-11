#ifndef HIT_H
#define HIT_H

#include "math/point3.h"
#include "math/normal3.h"
#include "material.h"
#include "math/ray.h"
#include "math/point2.h"

template<typename T>
struct Hit {
    Point3<T> point; // hit point
    Normal3<T> normal; // normal at hit point
    T t; // ray parameter at hit point
    bool front_face; // whether the ray hits the front face
    Material<T> material; // material at hit point
    Point2<T> uv; // surface UV coordinates
    int texture_idx = -1; // index of the texture (if textured)
    int bump_map_idx = -1; // index of the bump map (if has bump map)
    Vec3<T> tangent; // tangent vector for bump mapping
    Vec3<T> bitangent; // bitangent vector for bump mapping

    // default constructor
    Hit() : point(), normal(), t(T(0)), front_face(true), material(), uv(), texture_idx(-1), bump_map_idx(-1) {}
    
    // set the normal direction based on ray direction
    inline void set_face_normal(const Ray<T>& r, const Normal3<T>& outward_normal) {
        front_face = dot(r.direction, outward_normal.toVec3()) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

#endif
