#ifndef OBJECT_LIST_H
#define OBJECT_LIST_H

#include "object.h"
#include <memory>
#include <vector>

template <typename T>
class ObjectList : public Object<T> {
public:
    std::vector<std::shared_ptr<Object<T>>> objects;

    ObjectList() = default;

    void clear() { objects.clear(); }

    void add(const std::shared_ptr<Object<T>>& obj) { objects.push_back(obj); }

    bool intersect(const Ray<T>& ray, T tmin, T tmax, Hit<T>& hit) const override {
        Hit<T> temp_hit;
        bool hit_anything = false;
        T closest_so_far = tmax;

        for (const auto& obj : objects) {
            if (obj && obj->intersect(ray, tmin, closest_so_far, temp_hit)) {
                hit_anything = true;
                closest_so_far = temp_hit.t;
                hit = temp_hit;
            }
        }
        return hit_anything;
    }
};

#endif
