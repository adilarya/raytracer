#ifndef NORMAL3_H
#define NORMAL3_H

#include "tuple.h"
#include "vec3.h" 

template<typename T>
struct Normal3 : public Tuple3<Normal3, T> {
    using Tuple3<Normal3, T>::Tuple3;

    // operations
    Vec3<T> toVec3() const {
        return Vec3<T>(this->x, this->y, this->z);
    }

    // unary minus
    Normal3 operator-() const {
        return Normal3(-this->x, -this->y, -this->z);
    }
};

// Vec3 -> Normal3 
template<typename T>
inline Normal3<T> normal_from_vec(const Vec3<T>& v) {
    return Normal3<T>(v.x, v.y, v.z);
}

// Normal3 -> Vec3
template<typename T>
inline Vec3<T> vec_from_normal(const Normal3<T>& n) {
    return n.toVec3();
}

#endif
