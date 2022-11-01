#include "fragment.h"

//Fragment FragmentShader(const Fragment& fragment) {
//    constexpr float gamma = 2.2f;
//    //constexpr float gamma = 1.f;
//    const Vec4 color = {
//        powf(fragment.color.r, 1.f / gamma),
//        powf(fragment.color.g, 1.f / gamma),
//        powf(fragment.color.b, 1.f / gamma),
//        fragment.color.a
//    };
//    return { fragment.pos, color, fragment.tex };
//}