#ifndef ELLIPSOID_H
#define ELLIPSOID_H

template <typename T>
class Ellipsoid : public Object<T> {
    // attributes
    Point3<T> center;
    Vec3<T> radii; // (rx, ry, rz)
    Material<T> material;

    public:
    // constructors
    Ellipsoid() : center(Point3<T>(T(0), T(0), T(0))), radii(Vec3<T>(T(1), T(1), T(1))), material(Material<T>()) {}
    Ellipsoid(const Point3<T>& center, const Vec3<T>& radii, const Material<T>& material) 
        : center(center), radii(radii), material(material) {}

    // specialized intersect function
    virtual bool intersect(const Ray<T>& ray, T tmin, T tmax, Hit<T>& hit) const override {
        Vec3<T> oc = ray.origin - center;
        Vec3<T> ocp(oc.x / radii.x, oc.y / radii.y, oc.z / radii.z);
        Vec3<T> dp(ray.direction.x / radii.x, ray.direction.y / radii.y, ray.direction.z / radii.z);

        // coefficients
        T a = dp.dot(dp);
        T b = T(2.0) * ocp.dot(dp);
        T c = ocp.dot(ocp) - T(1);

        // discriminant
        T discriminant = b * b - T(4) * a * c;
        if (discriminant < 0) {
            return false;
        } 

        T t = (-b - std::sqrt(discriminant)) / (T(2) * a);
        if (t < tmin || t > tmax) {
            t = (-b + std::sqrt(discriminant)) / (T(2) * a);
            if (t < tmin || t > tmax) {
                return false;
            }
        }
        hit.t = t;
        hit.point = ray.at(t);
        Vec3<T> pc = hit.point - center;
        Vec3<T> outward_normal((pc.x / (radii.x * radii.x)), (pc.y / (radii.y * radii.y)), (pc.z / (radii.z * radii.z)));
        hit.set_face_normal(ray, normal_from_vec(outward_normal.normalize()));
        hit.material = material;
        return true;
    }
};

#endif
