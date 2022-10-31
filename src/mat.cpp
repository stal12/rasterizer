#include "mat.h"

Vec4 operator*(const Mat4& m, const Vec4& v) {
    Vec4 res = 0.f;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            res[i] += m[i * 4 + j] * v[j];
        }
    }
    return res;
}

Mat4 translation(const Vec3& v) {
	Mat4 res = 1.f;
	res[3] += v[0];
	res[7] += v[1];
	res[11] += v[2];
	return res;
}

Mat4 scaling(const Vec3& v) {
	Mat4 res = 1.f;
	res[0] = v[0];
	res[5] = v[1];
	res[10] = v[2];
	return res;
}

// axis MUST be normalized
Mat4 rotation(float angle, const Vec3& axis) {
	Mat4 res = 1.f;
	const float cosa = cosf(angle);
	const float sina = sinf(angle);
	const float ux = axis.x;
	const float uy = axis.y;
	const float uz = axis.z;
	res[0] = cosa + ux * ux * (1.f - cosa);
	res[1] = ux * uy * (1.f - cosa) - uz * sina;
	res[2] = ux * uz * (1.f - cosa) + uy * sina;
	res[4] = ux * uy * (1.f - cosa) + uz * sina;
	res[5] = cosa + uy * uy * (1.f - cosa);
	res[6] = uy * uz * (1.f - cosa) - ux * sina;
	res[8] = ux * uz * (1.f - cosa) - uy * sina;
	res[9] = uy * uz * (1.f - cosa) + ux * sina;
	res[10] = cosa + uz * uz * (1.f - cosa);
	return res;
}

Mat4 projection(float left, float right, float bottom, float top, float near, float far) {
	Mat4 res = 0.f;
	res[0] = 2 * near / (right - left);
	res[2] = (right + left) / (right - left);
	res[5] = 2 * near / (top - bottom);
	res[6] = (top + bottom) / (top - bottom);
	res[10] = -(far + near) / (far - near);
	res[11] = -2 * far * near / (far - near);
	res[14] = -1.f;
	return res;
}

Mat4 projection(float fovy, float aspect, float near, float far) {
	const float top = near * tanf(fovy / 2.f);
	const float bottom = -top;
	const float right = top * aspect;
	const float left = -right;
	return projection(left, right, bottom, top, near, far);
}

Mat4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
	
	Mat4 res = 1.f;
	const Vec3 forward = normalize(eye - center);
	const Vec3 right = normalize(cross(up, forward));
	const Vec3 newup = cross(forward, right);
	res[0] = right[0];
	res[1] = right[1];
	res[2] = right[2];
	res[4] = newup[0];
	res[5] = newup[1];
	res[6] = newup[2];
	res[8] = forward[0];
	res[9] = forward[1];
	res[10] = forward[2];
	res[3] = -dot(right, eye[0]);
	res[7] = -dot(newup, eye[1]);
	res[11] = -dot(forward, eye[2]);
	return res;
}
