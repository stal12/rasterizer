#pragma once

#include "vector.h"

struct Vertex {

    Vec3 pos;	// This should be a Vec4
    Vec4 color;
    Vec2 tex;

};

struct BasicVertShader {

    Vertex operator()(const Vertex& vertex) {
        return vertex;
    }

};
