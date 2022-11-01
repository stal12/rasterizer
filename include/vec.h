#pragma once

#include <array>

struct Vec2 {

    union {
        // Accessing these randomly can be UB, I'm not sure
        // GLM does the same
        std::array<float, 2> data;
        struct {
            float x, y;
        };
    };

    Vec2() = default;
    Vec2(float v) : x(v), y(v) {};
    Vec2(float x_, float y_) : x(x_), y(y_) {};

    const float& operator[](int pos) const {
        return data[pos];
    }

    float& operator[](int pos) {
        return data[pos];
    }

};


inline Vec2 operator*(const Vec2& v, float f) {
    return Vec2{ v.x * f, v.y * f };
}

inline Vec2 operator*(float f, const Vec2& v) {
    return Vec2{ v.x * f, v.y * f };
}

inline Vec2 operator+(const Vec2& a, const Vec2& b) {
    return Vec2{ a.x + b.x, a.y + b.y };
}

inline float norm(const Vec2& v) {
    return sqrtf(v.x * v.x + v.y * v.y);
}

inline Vec2 normalize(const Vec2& v) {
    return v * (1.f / norm(v));
}

struct Vec3 {
    union {
        std::array<float, 3> data;
        struct {
            float x, y, z;
        };
        struct {
            float r, g, b;
        };
    };

    Vec3() = default;
    Vec3(float v) : x(v), y(v), z(v) {};
    Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {};
    Vec3(const Vec2& v, float z_) : x(v.x), y(v.y), z(z_) {};

    const float& operator[](int pos) const {
        return data[pos];
    }

    float& operator[](int pos) {
        return data[pos];
    }

};

inline Vec3 operator-(const Vec3& v) {
    return Vec3{ -v.x, -v.y, -v.z };
}

inline Vec3 operator*(const Vec3& v, float f) {
    return Vec3{ v.x * f, v.y * f, v.z * f };
}

inline Vec3 operator/(const Vec3& v, float f) {
    return Vec3{ v.x / f, v.y / f, v.z / f };
}

inline Vec3 operator/(float f, const Vec3& v) {
    return Vec3{ f / v.x, f / v.y, f / v.z };
}

inline Vec3 operator+(const Vec3& a, const Vec3& b) {
    return Vec3{ a.x + b.x, a.y + b.y, a.z + b.z };
}

inline Vec3 operator-(const Vec3& a, const Vec3& b) {
    return Vec3{ a.x - b.x, a.y - b.y, a.z - b.z };
}

inline float norm(const Vec3& v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

inline Vec3 normalize(const Vec3& v) {
    return v * (1.f / norm(v));
}

inline float dot(const Vec3& a, const Vec3& b) {
    return { a.x * b.x + a.y * b.y + a.z * b.z };
}

inline Vec3 cross(const Vec3& a, const Vec3& b)
{
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

struct Vec4 {
    union {
        std::array<float, 4> data;
        struct {
            float x, y, z, w;
        };
        struct {
            float r, g, b, a;
        };
    };

    Vec4() = default;
    Vec4(float v) : x(v), y(v), z(v), w(v) {};
    Vec4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {};
    Vec4(Vec3 v, float w_) : x(v.x), y(v.y), z(v.z), w(w_) {}

    const float& operator[](int pos) const {
        return data[pos];
    }

    float& operator[](int pos) {
        return data[pos];
    }

};

inline Vec4 operator*(const Vec4& v, float f) {
    return Vec4{ v.x * f, v.y * f, v.z * f, v.w * f };
}

inline Vec4 operator*(float f, const Vec4& v) {
    return Vec4{ v.x * f, v.y * f, v.z * f, v.w * f };
}

inline Vec4 operator/(const Vec4& v, float f) {
    return Vec4{ v.x / f, v.y / f, v.z / f, v.w / f };
}

inline Vec4 operator+(const Vec4& a, const Vec4& b) {
    return Vec4{ a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
}

inline float norm(const Vec4& v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}

inline Vec4 normalize(const Vec4& v) {
    return v * (1.f / norm(v));
}
