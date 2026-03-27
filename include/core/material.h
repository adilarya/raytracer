#ifndef MATERIAL_H
#define MATERIAL_H

#include "math/vec3.h"

template<typename T>
struct Material {
    using Color3 = Vec3<T>;
    // attributes
    Color3 Od; // diffuse reflectance color
    Color3 Os; // specular reflectance color
    T ka, kd, ks; // ambient, diffuse, specular coefficients
    T n; // shininess exponent
    T alpha; // opacity
    T eta; // index of refraction

    // constructors
    Material() : Od(1, 1, 1), Os(0, 0, 0), ka(0), kd(1), ks(0), n(1), alpha(1), eta(1) {}
    Material(const Color3& Od, const Color3& Os, T ka, T kd, T ks, T n, T alpha, T eta)
        : Od(Od), Os(Os), ka(ka), kd(kd), ks(ks), n(n), alpha(alpha), eta(eta) {}
};

#endif
