#include <vector>
#include <array>
#include <string>
#include <fstream>
#include <algorithm>


struct Vec2 {
	float x;
	float y;

	friend Vec2 operator*(const Vec2& v, float f) {
		return Vec2{ v.x * f, v.y * f };
	}

	friend Vec2 operator*(float f, const Vec2& v) {
		return Vec2{ v.x * f, v.y * f };
	}

	friend Vec2 operator+(const Vec2& a, const Vec2& b) {
		return Vec2{ a.x + b.x, a.y + b.y };
	}
};

struct Vec3 {
	union {
		float r;
		float x;
	};
	union {
		float g;
		float y;
	};
	union {
		float b;
		float z;
	};

	friend Vec3 operator*(const Vec3& v, float f) {
		return Vec3{ v.x * f, v.y * f, v.z * f };
	}

	friend Vec3 operator*(float f, const Vec3& v) {
		return Vec3{ v.x * f, v.y * f, v.z * f };
	}

	friend Vec3 operator+(const Vec3& a, const Vec3& b) {
		return Vec3{ a.x + b.x, a.y + b.y, a.z + b.z };
	}
};

struct Vec4 {
	union {
		float r;
		float x;
	};
	union {
		float g;
		float y;
	};
	union {
		float b;
		float z;
	};
	union {
		float a;
		float w;
	};

	friend Vec4 operator*(const Vec4& v, float f) {
		return Vec4{ v.x * f, v.y * f, v.z * f, v.w * f };
	}

	friend Vec4 operator*(float f, const Vec4& v) {
		return Vec4{ v.x * f, v.y * f, v.z * f, v.w * f };
	}

	friend Vec4 operator+(const Vec4& a, const Vec4& b) {
		return Vec4{ a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
	}
};

struct Framebuffer {

	const int w;
	const int h;

	std::vector<Vec4> colors;
	std::vector<float> depths;

	Framebuffer(int w_, int h_) : w(w_), h(h_), colors(w* h), depths(w* h) {}

	Vec4 getColor(int x, int y) const {
		return colors[y * w + x];
	}

	void setColor(int x, int y, const Vec4& color) {
		colors[y * w + x] = color;
	}

	void setDepth(int x, int y, float depth) {
		depths[y * w + x] = depth;
	}

	void clear(const Vec4& color = { 0.f, 0.f, 0.f, 0.f }, float depth = 1.) {
		std::ranges::fill(colors, color);
		std::ranges::fill(depths, depth);
	}

};

void WriteImg(const std::string& filename, const Framebuffer& framebuffer) {

	std::ofstream os(filename, std::ios::binary);
	const int w = framebuffer.w;
	const int h = framebuffer.h;

	os << "P6\n" << framebuffer.w << " " << framebuffer.h << " " << 255 << "\n";
	for (int y = h - 1; y >= 0; --y) {
		for (int x = 0; x < w; ++x) {

			const auto& color = framebuffer.getColor(x, y);
			os.put(static_cast<uint8_t>(color.r * 255));
			os.put(static_cast<uint8_t>(color.g * 255));
			os.put(static_cast<uint8_t>(color.b * 255));

		}
	}
}

struct Vertex {

	Vec3 pos;
	Vec4 color;

};

struct Fragment {

	Vec3 pos;
	Vec4 color;

};

Vertex VertexShader(const Vertex& vertex) {
	return vertex;
}

void DrawTriangles(Framebuffer& framebuffer, const std::vector<Vertex>& vertices) {

	std::vector<Fragment> fragments;

	for (int i = 0; i < vertices.size() / 3; ++i) {

		Vertex a = VertexShader(vertices[i * 3]);
		Vertex b = VertexShader(vertices[i * 3 + 1]);
		Vertex c = VertexShader(vertices[i * 3 + 2]);

		// Find fragments
		const int w = framebuffer.w;
		const int h = framebuffer.h;

		// Convert coordinates to window space
		const Vec2 aPos{ (a.pos.x - -1.f) / 2.f * w, (a.pos.y - -1.f) / 2.f * h };
		const Vec2 bPos{ (b.pos.x - -1.f) / 2.f * w, (b.pos.y - -1.f) / 2.f * h };
		const Vec2 cPos{ (c.pos.x - -1.f) / 2.f * w, (c.pos.y - -1.f) / 2.f * h };

		// Find the upper, leftmost, and rightmost vertices
		Vec2 high, left, right;
		if (aPos.y > bPos.y && aPos.y > cPos.y) {
			high = aPos;
			left = bPos;
			right = cPos;
		}
		else if (bPos.y > aPos.y && bPos.y > cPos.y) {
			high = bPos;
			left = aPos;
			right = cPos;
		}
		else {
			high = cPos;
			left = bPos;
			right = aPos;
		}
		if (left.x > right.x) {
			std::swap(left, right);
		}

		// Consider the left and right lines coming down from the upper vertex / \ 
		// m and q are parameters of the equation x = my + q
		float mLeft = (high.x - left.x) / (high.y - left.y);	// TODO What happens if the line is horizontal?
		float qLeft = -left.y * mLeft + left.x;

		float mRight = (high.x - right.x) / (high.y - right.y);
		float qRight = -right.y * mRight + right.x;

		float mBottom = (right.x - left.x) / (right.y - left.y);
		float qBottom = -left.y * mBottom + left.x;

		// Examine the internal of the triangle line by line
		const float den = (bPos.y - cPos.y) * (aPos.x - cPos.x) + (cPos.x - bPos.x) * (aPos.y - cPos.y);
		bool changedLine = false;
		for (int r = static_cast<int>(high.y + 0.5f); ; --r) {

			// Find leftmost and rightmost pixels in this line
			const float y = r + 0.5f;
			if (y < high.y) {

				if (y < left.y) {
					if (!changedLine) {
						// Change the line / with the line _
						mLeft = mBottom;
						qLeft = qBottom;
						changedLine = true;
						left = right;	// From this point they are treated as one
					}
					else {
						break;
					}
				}
				if (y < right.y) {
					// Change the line \ with the line _
					mRight = mBottom;
					qRight = qBottom;
					changedLine = true;
					right = left;
				}

				const float xLeft = mLeft * y + qLeft;
				const float xRight = mRight * y + qRight;

				// Take all pixels with center between these two extremes
				for (float x = ceilf(xLeft - 0.5f) + 0.5f; x < xRight; ++x) {
					// New fragment
					// Interpolate the values of the vertices
					const float wa = ((bPos.y - cPos.y) * (x - cPos.x) + (cPos.x - bPos.x) * (y - cPos.y)) / den;
					const float wb = ((cPos.y - aPos.y) * (x - cPos.x) + (aPos.x - cPos.x) * (y - cPos.y)) / den;
					const float wc = 1.f - wa - wb;
					const float depth = wa * a.pos.z + wb * b.pos.z + wc * c.pos.z;	// TODO Transform this
					Fragment frag{
						{x, y, depth},
						wa * a.color + wb * b.color + wc * c.color
					};
					fragments.push_back(frag);
				}

			}
		}
	}

	// Now draw fragments
	for (int i = 0; i < fragments.size(); ++i) {

		const auto& frag = fragments[i];
		// TODO Run the fragment shader
		// TODO z-test
		framebuffer.setColor(static_cast<int>(frag.pos.x), static_cast<int>(frag.pos.y), frag.color);
	}
}

int main(void) {

	constexpr int w = 1920;
	constexpr int h = 1080;

	Framebuffer framebuffer(w, h);

	std::vector<Vertex> vertices{
		{ {-1.0f, -1.0f, 0.f}, { 1.f, 0.f, 0.f, 1.f } },
		{ {-1.0f, +1.0f, 0.f }, { 0.f, 1.f, 0.f, 1.f } },
		{ { 1.0f, 0.f, 0.f }, { 0.f, 0.f, 1.f, 1.f } }
	};

	framebuffer.clear();
	DrawTriangles(framebuffer, vertices);

	WriteImg("img.ppm", framebuffer);

}