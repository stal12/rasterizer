#define _USE_MATH_DEFINES
#include <cmath>

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

	for (int i = 0; i < vertices.size() / 3; ++i) {

		Vertex a = std::apply(vShader, vertices[i * 3]);		// it works with const Vertex a, but how?
		auto b = std::apply(vShader, vertices[i * 3 + 1]);
		auto c = std::apply(vShader, vertices[i * 3 + 2]);

		// TODO Clipping

		// TODO Divide by w
		a.pos = a.pos * (1.f / a.pos.w);
		b.pos = b.pos * (1.f / b.pos.w);
		c.pos = c.pos * (1.f / c.pos.w);

		// Find fragments

		const int w = framebuffer.w;
		const int h = framebuffer.h;

		// Convert coordinates to window space
		const Vec2 aPos{ (a.pos.x - -1.f) / 2.f * w, (a.pos.y - -1.f) / 2.f * h };
		const Vec2 bPos{ (b.pos.x - -1.f) / 2.f * w, (b.pos.y - -1.f) / 2.f * h };
		const Vec2 cPos{ (c.pos.x - -1.f) / 2.f * w, (c.pos.y - -1.f) / 2.f * h };

		// Bounding box
		const float top = std::max({ aPos.y, bPos.y, cPos.y });
		const float bottom = std::min({ aPos.y, bPos.y, cPos.y });
		const float left = std::min({aPos.x, bPos.x, cPos.x	});
		const float right = std::max({ aPos.x, bPos.x, cPos.x });

		// Examine the internal of the triangle line by line
		const float den = (bPos.y - cPos.y) * (aPos.x - cPos.x) + (cPos.x - bPos.x) * (aPos.y - cPos.y);
		
		for (float y = bottom - 0.5f; y < top + 0.5f; ++y) {
			for (float x = left - 0.5f; x < right + 0.5f; ++x) {
		
				const float wa = ((bPos.y - cPos.y) * (x - cPos.x) + (cPos.x - bPos.x) * (y - cPos.y)) / den;
				const float wb = ((cPos.y - aPos.y) * (x - cPos.x) + (aPos.x - cPos.x) * (y - cPos.y)) / den;
				const float wc = 1.f - wa - wb;
				constexpr float tol = 0.00001f;
				if (wa >= -tol && wb >= -tol && wc >= -tol && wa <= 1+tol && wb <= 1+tol && wc <= 1+tol) {
					// New fragment
					
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

	/*std::vector<std::tuple<Vec3, Vec4, Vec2>> vertices{
		{{-1.0f, -1.0f, 0.f}, {1.f, 0.f, 0.f, 1.f}, {0.0f, 1.0f }},
		{{-1.0f, +1.0f, 0.f}, {1.f, 0.f, 0.f, 1.f}, {1.0f, 0.0f}},
		{{ 1.0f, 0.f, 0.f}, {1.f, 1.f, 0.f, 0.8f}, {1.0f, 1.0f}},

		{ {0.8f, 0.3f, -0.1f}, {0.f, 0.f, 1.f, 0.5f}, {1.0f, 0.5f} },
	   { {0.5f, -0.5f, 0.2f }, { 0.f, 1.f, 0.f, 0.5f }, {0.0f, 0.0f} },
	   { { 0.0f, 0.5f, -0.5f }, { 0.f, 1.f, 1.f, 0.5f }, {1.0f, 0.7f} },

	   { {-0.5f, -0.5f, -0.1f}, { 0.f, 1.f, 0.f, 1.0f }, {0.f, 0.f} },
	   { {0.5f, -0.5f, 0.4f }, { 0.f, 1.f, 0.f, 0.7f }, {1.f, 0.f} },
	   { { 0.0f, 0.5f, -0.5f }, { 0.f, 1.f, 0.f, 0.5f }, {0.5f, 1.f} }

   };*/

	std::vector<std::tuple<Vec3, Vec2>> vertices{
		// back face
		{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
		{{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
		{{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}},
		{{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}},
		{{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f}},
		{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
		// front face
		{{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}},
		{{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f}},
		{{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}},
		{{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}},
		{{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f}},
		{{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}},
		// left face
		{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
		{{-0.5f,  0.5f, -0.5f}, {1.0f, 0.0f}},
		{{-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}},
		{{-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}},
		{{-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f}},
		{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
		// right face
		 {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
		 {{0.5f,  0.5f, -0.5f}, {1.0f, 0.0f}},
		 {{0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}},
		 {{0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}},
		 {{0.5f, -0.5f,  0.5f}, {0.0f, 1.0f}},
		 {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
		// bottom face      
		{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
		{{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
		{{ 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f}},
		{{ 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f}},
		{{-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f}},
		{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
		// top face
		{{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f}},
		{{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f}},
		{{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}},
		{{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}},
		{{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f}},
		{{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f}}
	};

	framebuffer.clear({ 0.1f,0.1f,0.2f,1.f });
	CubeVertShader cube_vert;
	cube_vert.model =
		translation({ 0.f, 0.f, -2.5f }) *
		rotation(0.8f * (float)M_PI / 4.f, normalize(Vec3{1.f, 1.f, 1.f}))*
		scaling(Vec3{ 1.f, 1.f, 1.f } * 1.0f);
	cube_vert.view = lookAt(Vec3{ 0.f, 0.f, 0.f }, Vec3{ 0.3f, 0.f, -1.f }, Vec3{ 0.f, 1.f, 0.f });
	cube_vert.projection = projection((float)M_PI / 4.f, (float)w / h, 0.1f, 10.f);
	DrawTriangles(framebuffer, std::span{ vertices.begin(), 36}, cube_vert, TextureFragShader(texture));
	//DrawTriangles(framebuffer, std::span{ vertices.begin() + 3, 3 }, BasicVertShader(), BasicFragShader());
	//DrawTriangles(framebuffer, std::span{ vertices.begin() + 6, 3 }, BasicVertShader(), TextureFragShader(texture));

	WriteImg("img.ppm", framebuffer);

}