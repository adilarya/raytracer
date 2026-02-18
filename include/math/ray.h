#ifndef RAY_H
#define RAY_H

#include "point3.h"
#include "vec3.h"

template<typename T>
struct Ray {
    Point3<T> origin; // origin
    Vec3<T> direction; // direction

    // constructors
    Ray() : origin(T{0}, T{0}, T{0}), direction(T{0}, T{0}, T{0}) {}
    Ray(const Point3<T>& origin, const Vec3<T>& direction) : origin(origin), direction(direction) {}

    // r(t) = o + t*d
    Point3<T> at(T t) const {
        return origin + direction * t;
    }
};

#endif
