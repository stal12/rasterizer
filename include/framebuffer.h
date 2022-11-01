#pragma once

#include <vector>
#include <algorithm>

#include "vec.h"

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
