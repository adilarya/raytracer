#ifndef VEC3_H
#define VEC3_H

#include "tuple.h"
#include <cmath>

template<typename T>
struct Vec3 : public Tuple3<Vec3, T> {
    using Tuple3<Vec3, T>::Tuple3;

    // vector-specific operations
    Vec3 cross(const Vec3& other) const {
        return Vec3(
            this->y * other.z - this->z * other.y,
            this->z * other.x - this->x * other.z,
            this->x * other.y - this->y * other.x
        );
    }
    T dot(const Vec3& other) const {
        return this->x * other.x + this->y * other.y + this->z * other.z;
    }
    Vec3 normalize() const { // does NOT mutate the vector
        T length = std::sqrt(this->dot(*this));
        if (length == 0) return Vec3(0, 0, 0); // avoid division by zero
        return Vec3(this->x / length, this->y / length, this->z / length);
    }
    T length() const {
        return std::sqrt(this->dot(*this));
    }

};

// free-function operators for Vec3 (dot, cross)
template<typename T>
Vec3<T> cross(const Vec3<T>& a, const Vec3<T>& b) {
    return a.cross(b);
}
template<typename T>
T dot(const Vec3<T>& a, const Vec3<T>& b) {
    return a.dot(b);
}

#endif
