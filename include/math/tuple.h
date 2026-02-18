#ifndef TUPLE_H
#define TUPLE_H

#include <stdexcept>
#include <cstddef>

template <template <typename> class Child, typename T>
class Tuple2 {
    public:
        T x, y; // member variables

        // constructors
        Tuple2() : x(0), y(0) {} // default constructor
        Tuple2(T x, T y) : x(x), y(y) {} // parameterized constructor

        // operator overloads
        Child<T> operator+(const Child<T>& other) const {
            return Child<T>{x + other.x, y + other.y};
        }
        Child<T> operator-(const Child<T>& other) const {
            return Child<T>{x - other.x, y - other.y};
        }
        Child<T> operator*(const T& scalar) const {
            return Child<T>{x * scalar, y * scalar};
        }
        Child<T> operator/(const T& scalar) const {
            if (scalar == 0) throw std::invalid_argument("Division by zero");
            return Child<T>{x / scalar, y / scalar};
        }

        // compound assignment operators
        Child<T>& operator+=(const Child<T>& other) {
            x += other.x;
            y += other.y;
            return static_cast<Child<T>&>(*this);
        }
        Child<T>& operator-=(const Child<T>& other) {
            x -= other.x;
            y -= other.y;
            return static_cast<Child<T>&>(*this);
        }
        Child<T>& operator*=(const T& scalar) {
            x *= scalar;
            y *= scalar;
            return static_cast<Child<T>&>(*this);
        }
        Child<T>& operator/=(const T& scalar) {
            if (scalar == 0) throw std::invalid_argument("Division by zero");
            x /= scalar;
            y /= scalar;
            return static_cast<Child<T>&>(*this);
        }

        // unary operators
        Child<T> operator+() const {
            return Child<T>{x, y};
        }
        Child<T> operator-() const {
            return Child<T>{-x, -y};
        }

        // indexing
        T& operator[](size_t index) {
            if (index == 0) return x;
            else if (index == 1) return y;
            else throw std::out_of_range("Index out of range");
        }
        const T& operator[](size_t index) const {
            if (index == 0) return x;
            else if (index == 1) return y;
            else throw std::out_of_range("Index out of range");
        }

        // equality
        bool operator==(const Child<T>& other) const {
            return x == other.x && y == other.y;
        }
        bool operator!=(const Child<T>& other) const {
            return !(*this == other);
        }
};

// scalar-left multiplication for Tuple2
template <template <typename> class Child, typename T>
inline Child<T> operator*(const T& scalar, const Tuple2<Child, T>& v) {
    return static_cast<const Child<T>&>(v) * scalar;
}

template <template <typename> class Child, typename T>
class Tuple3 {
    public:
        T x, y, z; // member variables

        // constructors
        Tuple3() : x(0), y(0), z(0) {} // default constructor
        Tuple3(T x, T y, T z) : x(x), y(y), z(z) {} // parameterized constructor

        // operator overloads
        Child<T> operator+(const Child<T>& other) const {
            return Child<T>{x + other.x, y + other.y, z + other.z};
        }
        Child<T> operator-(const Child<T>& other) const {
            return Child<T>{x - other.x, y - other.y, z - other.z};
        }
        Child<T> operator*(const T& scalar) const {
            return Child<T>{x * scalar, y * scalar, z * scalar};
        }
        Child<T> operator/(const T& scalar) const {
            if (scalar == 0) throw std::invalid_argument("Division by zero");
            return Child<T>{x / scalar, y / scalar, z / scalar};
        }

        // compound assignment operators
        Child<T>& operator+=(const Child<T>& other) {
            x += other.x;
            y += other.y;
            z += other.z;
            return static_cast<Child<T>&>(*this);
        }
        Child<T>& operator-=(const Child<T>& other) {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            return static_cast<Child<T>&>(*this);
        }
        Child<T>& operator*=(const T& scalar) {
            x *= scalar;
            y *= scalar;
            z *= scalar;
            return static_cast<Child<T>&>(*this);
        }
        Child<T>& operator/=(const T& scalar) {
            if (scalar == 0) throw std::invalid_argument("Division by zero");
            x /= scalar;
            y /= scalar;
            z /= scalar;
            return static_cast<Child<T>&>(*this);
        }

        // unary operators
        Child<T> operator+() const {
            return Child<T>{x, y, z};
        }
        Child<T> operator-() const {
            return Child<T>{-x, -y, -z};
        }

        // indexing
        T& operator[](size_t index) {
            if (index == 0) return x;
            else if (index == 1) return y;
            else if (index == 2) return z;
            else throw std::out_of_range("Index out of range");
        }
        const T& operator[](size_t index) const {
            if (index == 0) return x;
            else if (index == 1) return y;
            else if (index == 2) return z;
            else throw std::out_of_range("Index out of range");
        }

        // equality
        bool operator==(const Child<T>& other) const {
            return x == other.x && y == other.y && z == other.z;
        }
        bool operator!=(const Child<T>& other) const {
            return !(*this == other);
        }
};

// scalar-left multiplication for Tuple3
template <template <typename> class Child, typename T>
inline Child<T> operator*(const T& scalar, const Tuple3<Child, T>& v) {
    return static_cast<const Child<T>&>(v) * scalar;
}

#endif
