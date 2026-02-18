#ifndef OBJECT_H
#define OBJECT_H

#include "math/ray.h"
#include "core/hit.h"

template <typename T>
class Object {
    public:
        virtual ~Object() = default;
        virtual bool intersect(const Ray<T>& ray, T tmin, T tmax, Hit<T>& hit) const = 0;
};

#endif
