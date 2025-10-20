#pragma once
#include <cmath>

struct Vector3 {
    double x, y, z;

    Vector3() : x(0), y(0), z(0) {}
    Vector3(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}

    Vector3 operator+(const Vector3& v) const {
        return Vector3(x + v.x, y + v.y, z + v.z);
    }

    Vector3 operator-(const Vector3& v) const {
        return Vector3(x - v.x, y - v.y, z - v.z);
    }

    Vector3 operator*(double s) const {
        return Vector3(x * s, y * s, z * s);
    }

    Vector3 operator/(double s) const {
        return Vector3(x / s, y / s, z / s);
    }

    Vector3& operator+=(const Vector3& v) {
        x += v.x; y += v.y; z += v.z;
        return *this;
    }

    Vector3& operator-=(const Vector3& v) {
        x -= v.x; y -= v.y; z -= v.z;
        return *this;
    }

    double magnitude() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    Vector3 normalized() const {
        double mag = magnitude();
        if (mag > 0.0) return *this / mag;
        return Vector3(0, 0, 0);
    }

    double dot(const Vector3& v) const {
        return x * v.x + y * v.y + z * v.z;
    }

    Vector3 cross(const Vector3& v) const {
        return Vector3(
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x
        );
    }
};

