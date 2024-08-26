#ifndef VEC3_H
#define VEC3_H

#include "rt.h"
#include <array>

class vec3 {
public:
    vec3() : data{ 0.0, 0.0, 0.0 } {}
    vec3(double x, double y, double z) : data{ x, y, z } {}

    double x() const { return data[0]; }
    double y() const { return data[1]; }
    double z() const { return data[2]; }

    vec3 operator-() const { return vec3{ -data[0], -data[1], -data[2] }; }

    double& operator[](size_t i) { return data[i]; }
    double operator[](size_t i) const { return data[i]; }

    vec3& operator+=(const vec3& v) {
        (data[0] += v.data[0], data[1] += v.data[1], data[2] += v.data[2]);
        return *this;
    }

    vec3& operator*=(double t) {
        (data[0] *= t, data[1] *= t, data[2] *= t);
        return *this;
    }

    vec3& operator/=(double t) {
        return *this *= (1.0 / t);
    }

    double length() const {
        return std::sqrt(length_squared());
    }

    constexpr double length_squared() const {
        return data[0] * data[0] + data[1] * data[1] + data[2] * data[2];
    }

    bool near_zero() const {
        // Return true if the vector is close to zero in all dimensions.
        auto s = 1e-8;
        return (std::fabs(data[0]) < s) && (std::fabs(data[1]) < s) && (std::fabs(data[2]) < s);
    }

    static vec3 random() {
        return vec3(RandomGenerator::instance().random_double(), RandomGenerator::instance().random_double(), RandomGenerator::instance().random_double());
    }

    static vec3 random(double min, double max) {
        return vec3(RandomGenerator::instance().random_double(min, max), RandomGenerator::instance().random_double(min, max), RandomGenerator::instance().random_double(min, max));
    }

private:
    std::array<double, 3> data;
};

using point3 = vec3;

inline std::ostream& operator<<(std::ostream& out, const vec3& v) {
    return out << v[0] << ' ' << v[1] << ' ' << v[2];
}

inline vec3 operator+(const vec3& u, const vec3& v) {
    return vec3{ u[0] + v[0], u[1] + v[1], u[2] + v[2] };
}

inline vec3 operator-(const vec3& u, const vec3& v) {
    return vec3{ u[0] - v[0], u[1] - v[1], u[2] - v[2] };
}

inline vec3 operator*(const vec3& u, const vec3& v) {
    return vec3{ u[0] * v[0], u[1] * v[1], u[2] * v[2] };
}

inline vec3 operator*(double t, const vec3& v) {
    return vec3{ t * v[0], t * v[1], t * v[2] };
}

inline vec3 operator*(const vec3& v, double t) {
    return t * v;
}

inline vec3 operator/(const vec3& v, double t) {
    return (1.0 / t) * v;
}

inline double dot(const vec3& u, const vec3& v) {
    return u[0] * v[0] + u[1] * v[1] + u[2] * v[2];
}

inline vec3 cross(const vec3& u, const vec3& v) {
    return vec3{ u[1] * v[2] - u[2] * v[1],
                u[2] * v[0] - u[0] * v[2],
                u[0] * v[1] - u[1] * v[0] };
}

inline vec3 unit_vector(const vec3& v) {
    return v / v.length();
}

inline vec3 random_in_unit_disk() {
    while (true) {
        auto p = vec3(RandomGenerator::instance().random_double(-1, 1), RandomGenerator::instance().random_double(-1, 1), 0);
        if (p.length_squared() < 1)
            return p;
    }
}

inline vec3 random_in_unit_sphere() {
    while (true) {
        auto p = vec3::random(-1, 1);
        if (p.length_squared() < 1)
            return p;
    }
}

inline vec3 random_unit_vector() {
    return unit_vector(random_in_unit_sphere());
}

inline vec3 random_on_hemisphere(const vec3& normal) {
    vec3 on_unit_sphere = random_unit_vector();
    if (dot(on_unit_sphere, normal) > 0.0)
        return on_unit_sphere;
    else
        return -on_unit_sphere;
}

inline vec3 reflect(const vec3& v, const vec3& n) {
    return v - 2 * dot(v, n) * n;
}

inline vec3 refract(const vec3& uv, const vec3& n, double etai_over_etat) {
    auto cos_theta = std::fmin(dot(-uv, n), 1.0);
    vec3 r_out_perp = etai_over_etat * (uv + cos_theta * n);
    vec3 r_out_parallel = -std::sqrt(std::fabs(1.0 - r_out_perp.length_squared())) * n;
    return r_out_perp + r_out_parallel;
}

#endif // VEC3_H