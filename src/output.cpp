#include "output.h"

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
