#pragma once

#include <vector>
#include <string>

#include "vector.h"

struct Texture {

    const int w;
    const int h;
    std::vector<Vec3> colors;

    Texture(int w_, int h_) : w(w_), h(h_), colors(w* h) {};

private:
    const Vec3& getColor(int x, int y) const {
        // Repeat the border indefinitely

        if (x < 0) {
            x = 0;
        }
        else if (x >= w) {
            x = w - 1;
        }
        if (y < 0) {
            y = 0;
        }
        else if (y >= h) {
            y = h - 1;
        }

        return colors[y * w + x];
    }

public:
    Vec3 sample(float x, float y) {

        // Convert coords from range [0, 1] to [0, w] and [0, h]
        x *= w;
        y *= h;

        x -= 0.5f;
        y -= 0.5f;

        const int xMin = static_cast<int>(x);
        const int xMax = xMin + 1;
        const int yMin = static_cast<int>(y);
        const int yMax = yMin + 1;

        const float xA = x - xMin;
        const float yA = y - yMin;

        return
            getColor(xMin, yMin) * (1.f - xA) * (1.f - yA) +
            getColor(xMin, yMax) * (1.f - xA) * yA +
            getColor(xMax, yMax) * xA * yA +
            getColor(xMax, yMin) * xA * (1.f - yA);
    }

};

// Read a texture from a ppm image
extern Texture ReadTexture(const std::string& filename, float gamma = 2.2f);