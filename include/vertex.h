#pragma once

#include <vector>

#include "vec.h"
#include "mat.h"

template <typename... Attr>
struct Vertex {

    Vec4 pos;
    std::tuple<Attr...> attr;

    Vertex(Vec4 pos_, Attr... attr_) : pos(std::move(pos_)), attr(std::move(attr_)...) {}

};

struct BasicVertShader {

    auto operator()(Vec3 pos, Vec2 tex) {
        return Vertex({ pos.x, pos.y, pos.z, 1.0 }, tex);
    }

};

struct CubeVertShader {

    Mat4 model = 1.f;
    Mat4 view = 1.f;
    Mat4 projection = 1.f;

    auto operator()(Vec3 pos, Vec2 tex) {
        Vec4 pos4 = { pos.x, pos.y, pos.z, 1.0f };
        //pos4 = projection * view * model * pos4;
        pos4 = view * model * pos4;
        pos4 = projection * pos4;
        return Vertex(pos4, tex);
    }

};
