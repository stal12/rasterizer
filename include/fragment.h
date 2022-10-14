#pragma once

#include "vector.h"
#include "texture.h"

struct Fragment {

    Vec3 pos;
    Vec4 color;
    Vec2 tex;

};

extern Fragment FragmentShader(const Fragment& fragment);

struct BasicFragShader {

    const float gamma = 2.2f;

    BasicFragShader() = default;
    BasicFragShader(float gamma_) : gamma(gamma_) {}

    Fragment operator()(const Fragment& fragment) {
        const Vec4 color = {
            powf(fragment.color.r, 1.f / gamma),
            powf(fragment.color.g, 1.f / gamma),
            powf(fragment.color.b, 1.f / gamma),
            fragment.color.a
        };
        return { fragment.pos, color, fragment.tex };
    }

};

struct TextureFragShader {

    Texture tex;
    const float gamma = 2.2f;

    TextureFragShader(const Texture& t) : tex(t) {}
    TextureFragShader(const Texture& t, float gamma_) : tex(t), gamma(gamma_) {}

    Fragment operator()(const Fragment& fragment) {
        
        const Vec3 texColor = tex.sample(fragment.tex.x, fragment.tex.y);
        
        const Vec4 color = {
            powf(texColor.r, 1.f / gamma),
            powf(texColor.g, 1.f / gamma),
            powf(texColor.b, 1.f / gamma),
            fragment.color.a
        };
        return { fragment.pos, color, fragment.tex };
    }

};
