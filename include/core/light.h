#ifndef LIGHT_H
#define LIGHT_H

#include "math/point3.h"
#include "math/vec3.h"
#include <limits>

template<typename T>
struct Light {
    // attributes
    Vec3<T> direction; // light direction, not specifying whether it's directional or point light
    bool type; // point = 1, directional = 0;
    T intensity; // intensity

    // constructors
    Light() : Light(Vec3<T>(0, 0, 0), false, 0) {}
    Light(const Vec3<T>& direction, bool type, T intensity) : direction(direction), type(type), intensity(intensity) {}
    // helper methods
    Vec3<T> direction_from(const Point3<T>& p) const {
        if (!type) { // directional light
            return -direction;
        } else { // point light
            return direction - Vec3<T>(p.x, p.y, p.z);
        }
    }

    T distance_from(const Point3<T>& p) const {
        if (!type) { // directional light
            return std::numeric_limits<T>::infinity();
        } else { // point light
            return (direction - Vec3<T>(p.x, p.y, p.z)).length();
        }
    }
};

#endif
