#include <vector>
#include <array>
#include <string>
#include <fstream>
#include <algorithm>


struct Vec2 {

    union {
        // Accessing these randomly can be UB, I'm not sure
        // GLM does the same
        std::array<float, 2> data;
        struct {
            float x, y;
        };
    };

    const float& operator[](int pos) const {
        return data[pos];
    }

    float& operator[](int pos) {
        return data[pos];
    }

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
        std::array<float, 3> data;
        struct {
            float x, y, z;
        };
        struct {
            float r, g, b;
        };
    };
    
    Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {};
    Vec3(const Vec2& v, float z_) : x(v.x), y(v.y), z(z_) {};

    const float& operator[](int pos) const {
        return data[pos];
    }

    float& operator[](int pos) {
        return data[pos];
    }

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
        std::array<float, 4> data;
        struct {
            float x, y, z, w;
        };
        struct {
            float r, g, b, a;
        };
    };

    Vec4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {};
    Vec4(Vec3 v, float w_) : x(v.x), y(v.y), z(v.z), w(w_) {}

    const float& operator[](int pos) const {
        return data[pos];
    }

    float& operator[](int pos) {
        return data[pos];
    }

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

struct Vertex {

    Vec3 pos;	// This should be a Vec4
    Vec4 color;
    Vec2 tex;

};

struct Fragment {

    Vec3 pos;
    Vec4 color;
    Vec2 tex;

};

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
Texture ReadTexture(const std::string& filename, float gamma = 2.2f) {

    std::ifstream is(filename, std::ios::binary);    
    std::string buf;
    is >> buf;
    if (buf != "P6") {
        throw std::runtime_error("ReadTexture: wrong magic number");
    }
    int w, h, maxV;
    is >> w;
    is >> h;
    is >> maxV;
    is.get();

    Texture tex(w, h);

    const uint8_t dataSize = maxV < 256 ? 1 : 2;
    std::vector<char> data(w * h * 3 * dataSize);
    is.read(data.data(), data.size());

    for (int y = h - 1; y >= 0; --y) {
        for (int x = 0; x < w; ++x) {
            for (int c = 0; c < 3; ++c) {

                uint16_t cVal = data[(y * w + x) * dataSize * 3 + dataSize * c];
                if (dataSize == 2) {
                    const uint16_t leastSig = data[(y * w + x) * dataSize * 3 + dataSize * c + 1];
                    cVal = cVal * 256 + leastSig;
                }
                tex.colors[y * w + x][c] = powf(static_cast<float>(cVal) / maxV, gamma);
            }
        }
    }

    return tex;
}

Vertex VertexShader(const Vertex& vertex) {
    return vertex;
}

Fragment FragmentShader(const Fragment& fragment) {
    constexpr float gamma = 2.2f;
    //constexpr float gamma = 1.f;
    const Vec4 color = {
        powf(fragment.color.r, 1.f / gamma),
        powf(fragment.color.g, 1.f / gamma),
        powf(fragment.color.b, 1.f / gamma),
        fragment.color.a
    };
    return { fragment.pos, color, fragment.tex };
}

void DrawTriangles(Framebuffer& framebuffer, const std::vector<Vertex>& vertices) {

    std::vector<Fragment> fragments;

    for (int i = 0; i < vertices.size() / 3; ++i) {

        Vertex a = VertexShader(vertices[i * 3]);
        Vertex b = VertexShader(vertices[i * 3 + 1]);
        Vertex c = VertexShader(vertices[i * 3 + 2]);

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

        Fragment frag = FragmentShader(fragments[i]);

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

    Vec2 d = {2, 3};

    constexpr int scale = 2;
    constexpr int w = 1920 / scale;
    constexpr int h = 1080 / scale;

    Framebuffer framebuffer(w, h);

    std::vector<Vertex> vertices{
        { {-1.0f, -1.0f, 0.f}, { 1.f, 0.f, 0.f, 1.f }, {0.0f, 1.0f} },
        { {-1.0f, +1.0f, 0.f }, { 1.f, 0.f, 0.f, 1.f }, {1.0f, 0.0f} },
        { { 1.0f, 0.f, 0.f }, { 1.f, 1.f, 0.f, 0.8f }, {1.0f, 1.0f} },

        { {-0.5f, -0.5f, -0.1f}, { 0.f, 1.f, 0.f, 0.5f }, {1.0f, 0.0f} },
        { {0.5f, -0.5f, 0.4f }, { 0.f, 1.f, 0.f, 0.5f }, {1.0f, 1.0f} },
        { { 0.0f, 0.5f, -0.5f }, { 0.f, 1.f, 0.f, 0.5f }, {0.0f, 1.0f} },

        { {0.8f, 0.3f, -0.1f}, { 0.f, 0.f, 1.f, 0.5f }, {1.0f, 0.5f} },
        { {0.5f, -0.5f, 0.2f }, { 0.f, 1.f, 0.f, 0.5f }, {0.0f, 0.0f} },
        { { 0.0f, 0.5f, -0.5f }, { 0.f, 1.f, 1.f, 0.5f }, {1.0f, 0.7f} }
    };

    framebuffer.clear({ 0.1f,0.1f,0.2f,1.f });
    DrawTriangles(framebuffer, vertices);

    WriteImg("img.ppm", framebuffer);

}