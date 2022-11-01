#pragma once

#include <vector>
#include <tuple>

#include "vec.h"
#include "vertex.h"
#include "fragment.h"
#include "framebuffer.h"


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



template <typename VertAttr, typename Vert, typename Frag>
void DrawTriangles(Framebuffer& framebuffer, std::span<VertAttr> vertices, Vert vShader, Frag fShader) {

	std::vector<decltype(Fragment(std::apply(vShader, vertices[0])))> fragments;

	for (int i = 0; i < vertices.size() / 3; ++i) {

		auto a = std::apply(vShader, vertices[i * 3]);		// it works with const Vertex a, but how?
		auto b = std::apply(vShader, vertices[i * 3 + 1]);
		auto c = std::apply(vShader, vertices[i * 3 + 2]);

		// Clipping is prformed afterwards

		a.pos = a.pos / a.pos.w;
		b.pos = b.pos / b.pos.w;
		c.pos = c.pos / c.pos.w;

		// Find fragments

		const int w = framebuffer.w;
		const int h = framebuffer.h;

		// Convert coordinates to window space
		const Vec2 aPos{ (a.pos.x - -1.f) / 2.f * w, (a.pos.y - -1.f) / 2.f * h };
		const Vec2 bPos{ (b.pos.x - -1.f) / 2.f * w, (b.pos.y - -1.f) / 2.f * h };
		const Vec2 cPos{ (c.pos.x - -1.f) / 2.f * w, (c.pos.y - -1.f) / 2.f * h };

		// Bounding box with clipping
		const float top = std::min((float)h, std::ceil(std::max({ aPos.y, bPos.y, cPos.y })));
		const float bottom = std::max(0.f, std::floor(std::min({ aPos.y, bPos.y, cPos.y })));
		const float left = std::max(0.f, std::floor(std::min({ aPos.x, bPos.x, cPos.x })));
		const float right = std::min((float)w, std::ceil(std::max({ aPos.x, bPos.x, cPos.x })));

		// Examine the internal of the triangle line by line
		const float den = (bPos.y - cPos.y) * (aPos.x - cPos.x) + (cPos.x - bPos.x) * (aPos.y - cPos.y);

		for (float y = bottom + 0.5f; y < top; ++y) {
			for (float x = left + 0.5f; x < right; ++x) {

				const float wa = ((bPos.y - cPos.y) * (x - cPos.x) + (cPos.x - bPos.x) * (y - cPos.y)) / den;
				const float wb = ((cPos.y - aPos.y) * (x - cPos.x) + (aPos.x - cPos.x) * (y - cPos.y)) / den;
				const float wc = 1.f - wa - wb;
				constexpr float tol = 0; // 0.00001f;
				if (wa >= -tol && wb >= -tol && wc >= -tol && wa <= 1 + tol && wb <= 1 + tol && wc <= 1 + tol) {
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