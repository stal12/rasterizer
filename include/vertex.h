#pragma once

#include <vector>

#include "vec.h"

template <typename... Attr>
struct Vertex {

    Vec4 pos;
    std::tuple<Attr...> attr;

    Vertex(Vec4 pos_, Attr... attr_) : pos(std::move(pos_)), attr(std::move(attr_)...) {}

};

struct BasicVertShader {

    auto operator()(Vec3 pos, Vec4 color, Vec2 tex) {
        return Vertex({ pos.x, pos.y, pos.z, 1.0 }, color, tex);
    }

};
