#include <vector>
#include <array>
#include <string>
#include <fstream>
#include <algorithm>
#include <span>

#include "vector.h"
#include "vertex.h"
#include "fragment.h"
#include "texture.h"

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

template <typename V, typename F>
void DrawTriangles(Framebuffer& framebuffer, std::span<Vertex> vertices, V vShader, F fShader) {

    std::vector<Fragment> fragments;

    for (int i = 0; i < vertices.size() / 3; ++i) {

        Vertex a = vShader(vertices[i * 3]);
        Vertex b = vShader(vertices[i * 3 + 1]);
        Vertex c = vShader(vertices[i * 3 + 2]);

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

                    Fragment frag{
                        {x, y, z},
                        wa * a.color + wb * b.color + wc * c.color,
                        wa * a.tex + wb * b.tex + wc * c.tex
                    };
                    fragments.push_back(frag);
                }

            }
        }
    }

    // Now draw fragments
    for (int i = 0; i < fragments.size(); ++i) {

        Fragment frag = fShader(fragments[i]);

        const int x = static_cast<int>(frag.pos.x);
        const int y = static_cast<int>(frag.pos.y);

        // Z-test
        const float depth = framebuffer.getDepth(x, y);
        if (frag.pos.z < depth) {
            framebuffer.setDepth(x, y, frag.pos.z);

            // Alpha blending
            const Vec4& src = framebuffer.getColor(x, y);
            const Vec4 res = src * (1 - frag.color.a) + frag.color * frag.color.a;

            framebuffer.setColor(x, y, res);
        }
    }
}

int main(void) {

    constexpr int scale = 1;
    constexpr int w = 1920 / scale;
    constexpr int h = 1080 / scale;

    Framebuffer framebuffer(w, h);

    Texture tex = ReadTexture("container.ppm");

    std::vector<Vertex> vertices{
        { {-1.0f, -1.0f, 0.f}, { 1.f, 0.f, 0.f, 1.f }, {0.0f, 1.0f} },
        { {-1.0f, +1.0f, 0.f }, { 1.f, 0.f, 0.f, 1.f }, {1.0f, 0.0f} },
        { { 1.0f, 0.f, 0.f }, { 1.f, 1.f, 0.f, 0.8f }, {1.0f, 1.0f} },
        
        { {0.8f, 0.3f, -0.1f}, { 0.f, 0.f, 1.f, 0.5f }, {1.0f, 0.5f} },
        { {0.5f, -0.5f, 0.2f }, { 0.f, 1.f, 0.f, 0.5f }, {0.0f, 0.0f} },
        { { 0.0f, 0.5f, -0.5f }, { 0.f, 1.f, 1.f, 0.5f }, {1.0f, 0.7f} },

        { {-0.5f, -0.5f, -0.1f}, { 0.f, 1.f, 0.f, 1.0f }, {0.f, 0.f} },
        { {0.5f, -0.5f, 0.4f }, { 0.f, 1.f, 0.f, 0.7f }, {1.f, 0.f} },
        { { 0.0f, 0.5f, -0.5f }, { 0.f, 1.f, 0.f, 0.5f }, {0.5f, 1.f} }

    };

    framebuffer.clear({ 0.1f,0.1f,0.2f,1.f });
    DrawTriangles(framebuffer, { vertices.begin(), 6 }, BasicVertShader(), BasicFragShader());
    DrawTriangles(framebuffer, { vertices.begin() + 6, 3 }, BasicVertShader(), TextureFragShader(tex));

    WriteImg("img2.ppm", framebuffer);

}