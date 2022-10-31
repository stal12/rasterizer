#pragma once

#include <array>
#include <span>

#include "vec.h"

template <size_t N>
struct Mat {

	std::array<float, N* N> data;

	Mat() = default;
	Mat(float v) {
		data.fill(0);
		for (int i = 0; i < N; ++i) {
			data[i * N + i] = v;
		}
	};
	Mat(const std::array<float, N* N>& data_) : data(data_) {}
	Mat(std::array<float, N* N>&& data_) : data(std::move(data_)) {}

	const float& operator[](int pos) const {
		return data[pos];
	}

	float& operator[](int pos) {
		return data[pos];
	}

};

template <size_t N>
Mat<N> operator*(const Mat<N>& v, float f) {
	Mat<N> res = v;
	for (int i = 0; i < 16; ++i) {
		res[i] *= f;
	}
	return res;
}

template <size_t N>
Mat<N> operator*(float f, const Mat<N>& v) {
	return v * f;
}

template <size_t N>
Mat<N> operator+(const Mat<N>& a, const Mat<N>& b) {
	Mat<N> res = a;
	for (int i = 0; i < N * N; ++i) {
		res[i] += b[i];
	}
	return res;
}

template <size_t N>
Mat<N> operator*(const Mat<N>& a, const Mat<N>& b) {
	Mat<N> res = 0.f;
	for (int r = 0; r < N; ++r) {
		for (int c = 0; c < N; ++c) {
			for (int j = 0; j < N; ++j) {
				res[r * N + c] += a[r * N + j] * b[j * N + c];
			}
		}
	}
	return res;
}

using Mat4 = Mat<4>;

Vec4 operator*(const Mat4& m, const Vec4& v);

Mat4 translation(const Vec3& v);
Mat4 scaling(const Vec3& v);

// axis MUST be normalized
Mat4 rotation(float angle, const Vec3& axis);
Mat4 projection(float left, float right, float bottom, float top, float near, float far);
Mat4 projection(float fovy, float aspect, float near, float far);
Mat4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& up);
