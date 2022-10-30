#include <vector>
#include <array>
#include <string>
#include <fstream>
#include <algorithm>
#include <span>
#include <tuple>

#include "vec.h"
#include "vertex.h"
#include "fragment.h"
#include "texture.h"

//template <typename Head, typename... Tail>
//constexpr std::tuple<Tail...> tuple_tail(const std::tuple<Head, Tail...>& t) {
//	return std::apply([](auto head, auto... tail) {
//		return std::make_tuple(tail...);
//}, t);
//}

template <size_t Start, typename... Vals>
constexpr void _tuple_interpolate(std::tuple<Vals...>& res,
	const std::tuple<Vals...>& a, const std::tuple<Vals...>& b, const std::tuple<Vals...>& c, 
	float wa, float wb, float wc) {
	if constexpr (Start < std::tuple_size_v<std::remove_reference_t<decltype(res)>>) {
		std::get<Start>(res) = std::get<Start>(a) * wa + std::get<Start>(b) * wb + std::get<Start>(c) * wc;
		_tuple_interpolate<Start + 1>(res, a, b, c, wa, wb, wc);
	}
}

template <typename... Vals>
constexpr std::tuple<Vals...> tuple_interpolate(
	const std::tuple<Vals...>& a, const std::tuple<Vals...>& b, const std::tuple<Vals...>& c,
	float wa, float wb, float wc) {

	std::tuple<Vals...> res;
	_tuple_interpolate<0>(res, a, b, c, wa, wb, wc);
	return res;
}

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

	float getDepth(int x, int y) const {
		return depths[y * w + x];
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

template <typename VertAttr, typename Vert, typename Frag>
void DrawTriangles(Framebuffer& framebuffer, std::span<VertAttr> vertices, Vert vShader, Frag fShader) {

	std::vector<decltype(Fragment(std::apply(vShader, vertices[0])))> fragments;

	// std::apply(vShader, vertices[0]);

	for (int i = 0; i < vertices.size() / 3; ++i) {

		const auto a = std::apply(vShader, vertices[i * 3]);		// it works with const Vertex a, but how?
		const auto b = std::apply(vShader, vertices[i * 3 + 1]);
		const auto c = std::apply(vShader, vertices[i * 3 + 2]);

		// TODO Clipping

		// Find fragments

		const int w = framebuffer.w;
		const int h = framebuffer.h;

		// Convert coordinates to window space
		const Vec2 aPos{ (a.pos.x - -1.f) / 2.f * w, (a.pos.y - -1.f) / 2.f * h };
		const Vec2 bPos{ (b.pos.x - -1.f) / 2.f * w, (b.pos.y - -1.f) / 2.f * h };
		const Vec2 cPos{ (c.pos.x - -1.f) / 2.f * w, (c.pos.y - -1.f) / 2.f * h };

		// Find the upper, leftmost, and rightmost vertices
		Vec2 high, left, right;
		if (aPos.y >= bPos.y && aPos.y >= cPos.y) {
			high = aPos;
			left = bPos;
			right = cPos;
		}
		else if (bPos.y >= aPos.y && bPos.y >= cPos.y) {
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
					const float z = (wa * a.pos.z + wb * b.pos.z + wc * c.pos.z) / 2.f + 0.5f;
					const Vec3 fragPos{ x, y, z };

					const auto fragAttr = tuple_interpolate(a.attr, b.attr, c.attr, wa, wb, wc);

					auto fragment = std::apply([&fragPos](auto&&... attrs) {
						return Fragment(fragPos, attrs...);
						}, fragAttr);

					fragments.push_back(fragment);
				}
			}
		}
	}

	// Now draw fragments
	for (int i = 0; i < fragments.size(); ++i) {
	
		const auto& frag = fragments[i];
			
		Vec4 color = std::apply([&frag, &fShader](auto&&... attrs) {
			return fShader(frag.pos, attrs...);
			}, frag.attr);
	 	
		const int x = static_cast<int>(frag.pos.x);
		const int y = static_cast<int>(frag.pos.y);
	
		// Z-test
		const float depth = framebuffer.getDepth(x, y);
		if (frag.pos.z < depth) {
			framebuffer.setDepth(x, y, frag.pos.z);
	
			// Alpha blending
			const Vec4& src = framebuffer.getColor(x, y);
			const Vec4 res = src * (1 - color.a) + color * color.a;
	
			framebuffer.setColor(x, y, res);
		}
	}
}

int main(void) {

	constexpr int scale = 1;
	constexpr int w = 1920 / scale;
	constexpr int h = 1080 / scale;

	Framebuffer framebuffer(w, h);

	Texture texture = ReadTexture("../data/greywall.ppm");

	std::vector<std::tuple<Vec3, Vec4, Vec2>> vertices{
		{{-1.0f, -1.0f, 0.f}, {1.f, 0.f, 0.f, 1.f}, {0.0f, 1.0f }},
		{{-1.0f, +1.0f, 0.f}, {1.f, 0.f, 0.f, 1.f}, {1.0f, 0.0f}},
		{{ 1.0f, 0.f, 0.f}, {1.f, 1.f, 0.f, 0.8f}, {1.0f, 1.0f}},
	
		{ {0.8f, 0.3f, -0.1f}, {0.f, 0.f, 1.f, 0.5f}, {1.0f, 0.5f} },
	   { {0.5f, -0.5f, 0.2f }, { 0.f, 1.f, 0.f, 0.5f }, {0.0f, 0.0f} },
	   { { 0.0f, 0.5f, -0.5f }, { 0.f, 1.f, 1.f, 0.5f }, {1.0f, 0.7f} },

	   { {-0.5f, -0.5f, -0.1f}, { 0.f, 1.f, 0.f, 1.0f }, {0.f, 0.f} },
	   { {0.5f, -0.5f, 0.4f }, { 0.f, 1.f, 0.f, 0.7f }, {1.f, 0.f} },
	   { { 0.0f, 0.5f, -0.5f }, { 0.f, 1.f, 0.f, 0.5f }, {0.5f, 1.f} }

   };

	framebuffer.clear({ 0.1f,0.1f,0.2f,1.f });
	DrawTriangles(framebuffer, std::span{ vertices.begin(), 3 }, BasicVertShader(), BasicFragShader());
	DrawTriangles(framebuffer, std::span{ vertices.begin() + 3, 3 }, BasicVertShader(), BasicFragShader());
	DrawTriangles(framebuffer, std::span{ vertices.begin() + 6, 3 }, BasicVertShader(), TextureFragShader(texture));

	WriteImg("img.ppm", framebuffer);

}