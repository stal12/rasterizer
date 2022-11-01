#define _USE_MATH_DEFINES
#include <cmath>

#include "vec.h"
#include "mat.h"
#include "vertex.h"
#include "fragment.h"
#include "texture.h"
#include "framebuffer.h"
#include "pipeline.h"
#include "output.h"


int main(void) {

	constexpr int scale = 1;
	constexpr int w = 1920 / scale;
	constexpr int h = 1080 / scale;

	Framebuffer framebuffer(w, h);

	Texture texture = ReadTexture("../data/greywall.ppm");

	/*std::vector<std::tuple<Vec3, Vec4, Vec2>> vertices{
		{{-1.0f, -1.0f, 0.f}, {1.f, 0.f, 0.f, 1.f}, {0.0f, 1.0f }},
		{{-1.0f, +1.0f, 0.f}, {1.f, 0.f, 0.f, 1.f}, {1.0f, 0.0f}},
		{{ 1.0f, 0.f, 0.f}, {1.f, 1.f, 0.f, 0.8f}, {1.0f, 1.0f}},

		{ {0.8f, 0.3f, -0.1f}, {0.f, 0.f, 1.f, 0.5f}, {1.0f, 0.5f} },
	   { {0.5f, -0.5f, 0.2f }, { 0.f, 1.f, 0.f, 0.5f }, {0.0f, 0.0f} },
	   { { 0.0f, 0.5f, -0.5f }, { 0.f, 1.f, 1.f, 0.5f }, {1.0f, 0.7f} },

	   { {-0.5f, -0.5f, -0.1f}, { 0.f, 1.f, 0.f, 1.0f }, {0.f, 0.f} },
	   { {0.5f, -0.5f, 0.4f }, { 0.f, 1.f, 0.f, 0.7f }, {1.f, 0.f} },
	   { { 0.0f, 0.5f, -0.5f }, { 0.f, 1.f, 0.f, 0.5f }, {0.5f, 1.f} }

   };*/

	std::vector<std::tuple<Vec3, Vec2>> vertices{
		// back face
		{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
		{{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
		{{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}},
		{{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}},
		{{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f}},
		{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
		// front face
		//{{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}},
		//{{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f}},
		//{{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}},
		//{{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}},
		//{{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f}},
		//{{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}},
		// left face
		{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
		{{-0.5f,  0.5f, -0.5f}, {1.0f, 0.0f}},
		{{-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}},
		{{-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}},
		{{-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f}},
		{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
		// right face
		 {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
		 {{0.5f,  0.5f, -0.5f}, {1.0f, 0.0f}},
		 {{0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}},
		 {{0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}},
		 {{0.5f, -0.5f,  0.5f}, {0.0f, 1.0f}},
		 {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
		// bottom face      
		{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
		{{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
		{{ 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f}},
		{{ 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f}},
		{{-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f}},
		{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
		// top face
		{{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f}},
		{{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f}},
		{{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}},
		{{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}},
		{{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f}},
		{{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f}}
	};

	framebuffer.clear({ 0.1f,0.1f,0.2f,1.f });
	CubeVertShader cube_vert;
	cube_vert.model =
		translation({ 0.f, 0.1f, -2.f }) *
		rotation(0.8f * (float)M_PI / 4.f, normalize(Vec3{1.f, 1.f, 1.f}))*
		scaling(1.0f);
	Vec3 eye = { 0.f, 0.0f, 0.f };
	cube_vert.view = lookAt(eye, eye + Vec3{ 0.0f, 0.f, -1.f }, Vec3{ 0.f, 1.f, 0.f });
	cube_vert.projection = projection((float)M_PI / 4.f, (float)w / h, 0.1f, 10.f);
	DrawTriangles(framebuffer, std::span{ vertices.begin(), 30}, cube_vert, TextureFragShader(texture));
	//DrawTriangles(framebuffer, std::span{ vertices.begin() + 3, 3 }, BasicVertShader(), BasicFragShader());
	//DrawTriangles(framebuffer, std::span{ vertices.begin() + 6, 3 }, BasicVertShader(), TextureFragShader(texture));

	WriteImg("img.ppm", framebuffer);

}