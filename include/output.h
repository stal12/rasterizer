#pragma once

#include <string>
#include <fstream>

#include "framebuffer.h"

void WriteImg(const std::string& filename, const Framebuffer& framebuffer);
