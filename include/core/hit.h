#ifndef HIT_H
#define HIT_H

#include "math/point3.h"
#include "math/normal3.h"
#include "material.h"
#include "math/ray.h"

template<typename T>
struct Hit {
    Point3<T> point; // hit point
    Normal3<T> normal; // normal at hit point
    T t; // ray parameter at hit point
    bool front_face; // whether the ray hits the front face
    Material<T> material; // material at hit point

    // default constructor
    Hit() : point(), normal(), t(T(0)), front_face(true), material() {}

    // set the normal direction based on ray direction
    inline void set_face_normal(const Ray<T>& r, const Normal3<T>& outward_normal) {
        front_face = dot(r.direction, outward_normal.toVec3()) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

#endif
