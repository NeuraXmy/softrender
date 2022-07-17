#include "renderppm.h"
#include <fstream>

RenderPpm::RenderPpm(int w, int h) : _w(w), _h(h)
{
	_framebuffer = std::make_unique<FrameBuffer>(w, h, 
		FrameBuffer::ColorFormat::LDR_RGB,
		FrameBuffer::DepthFormat::None);
}

void RenderPpm::save(std::string_view filename)
{
	std::ofstream out(filename.data());
	out << "P3\n" << _w << " " << _h << "\n255\n";
	for (int y = 0; y < _h; y++)
	{
		for (int x = 0; x < _w; x++)
		{
			auto c = _framebuffer->get_color(x, y) * 255.0f;
			out << int(c.r) << " " << int(c.g) << " " << int(c.b) << " ";
		}
		out << "\n";
	}
}
