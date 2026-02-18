#ifndef POINT3_H
#define POINT3_H

#include "tuple.h"
#include "vec3.h"

template<typename T>
struct Point3 : public Tuple3<Point3, T> {
    using Tuple3<Point3, T>::Tuple3;

    // point-specific operations
    Point3 operator+(const Vec3<T>& v) const {
        return Point3(this->x + v.x, this->y + v.y, this->z + v.z);
    }
    Point3 operator-(const Vec3<T>& v) const {
        return Point3(this->x - v.x, this->y - v.y, this->z - v.z);
    }
    Vec3<T> operator-(const Point3& other) const {
        return Vec3<T>(this->x - other.x, this->y - other.y, this->z - other.z);
    }
};

// free-function operators for Point3
template<typename T>
Point3<T> operator+(const Point3<T>& p, const Vec3<T>& v) {
    return p + v; // commutative
}
template<typename T>
Point3<T> operator-(const Point3<T>& p, const Vec3<T>& v) {
    return p - v; 
}
template<typename T>
Vec3<T> operator-(const Point3<T>& a, const Point3<T>& b) {
    return a - b; // returns vector from b to a
}

#endif
