# Rasterizer

A small rasterizer developed for learning purpose, based on the OpenGL rendering pipeline.

## Requirements

- C++ 20
- CMake 3.10

## Description

The project is meant to be simple enough to require no external dependency.
At its core is the `DrawTriangles` function, which renders triangles on a framebuffer, taking as input:
- A list of points and other optional data (normals, texture coords, etc)
- A vertex shader
- A fragment shader

Shaders are implemented as template function object, in the intent of (partially) emulating the huge flexibility of GLSL.

### Rendering pipeline

The rendering pipeline is as follows:
- Apply the vertex shader to each input point, obtaining vertices in Normalized Device Coordinates (with possible attributes)
- Group vertices into triangles
- Convert vertices coordinates to screen space
- Find the triangle bounding box, clipping at the screen borders
- Check which pixels in the bb are part of the triangle, and emit a fragment for each of them
- Apply the fragment shader to each fragment
- Perform depth test to check which fragment must be drawn
- Find the final color of the fragment doing alpha-blending with the current value in the framebuffer
- Update the framebuffer with the new color and depth

