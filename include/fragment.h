#pragma once

#include "vec.h"
#include "texture.h"
#include "vertex.h"

template <typename... Attr>
struct Fragment {

    Vec3 pos;
    std::tuple<Attr...> attr;

    Fragment(Vec3 pos_, Attr... attr_) : pos(std::move(pos_)), attr(std::move(attr_)...) {}
    Fragment(const Vertex<Attr...>& ver) : pos(ver.pos.x, ver.pos.y, ver.pos.z), attr(ver.attr) {}

};

template <typename... Attr>
auto make_fragment(Vec3 pos, Attr... attr) {
    return Fragment(pos, attr);
}

struct BasicFragShader {

    Vec4 operator() (Vec3 pos, Vec4 color, Vec2 tex) {
        return color;
    }

};

struct TextureFragShader {

    Texture texture;
    const float gamma = 2.2f;

    TextureFragShader(const Texture& t) : texture(t) {}
    TextureFragShader(const Texture& t, float gamma_) : texture(t), gamma(gamma_) {}

    Vec4 operator()(Vec3 pos, Vec4 color, Vec2 tex) {
        
        const Vec3 texColor = texture.sample(tex.x, tex.y);
        
        const Vec4 outColor = {
            powf(texColor.r, 1.f / gamma),
            powf(texColor.g, 1.f / gamma),
            powf(texColor.b, 1.f / gamma),
            color.a
        };
        return outColor;
    }

};
