#include "texture.h"

#include <string>
#include <fstream>

// Read a texture from a ppm image
Texture ReadTexture(const std::string& filename, float gamma) {

    std::ifstream is(filename, std::ios::binary);
    if (!is.is_open()) {
        throw std::runtime_error("ReadTexture: can't open file");
    }
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
    std::vector<uint8_t> data(w * h * 3 * dataSize);
    is.read(reinterpret_cast<char*>(data.data()), data.size());

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

