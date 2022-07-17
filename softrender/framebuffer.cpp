#include "framebuffer.h"

FrameBuffer::FrameBuffer(int width,
	int height,
	ColorFormat color_format,
	DepthFormat depth_format)
	: _width(width)
	, _height(height)
	, _color_format(color_format)
	, _depth_format(depth_format)
{
	if (_color_format == ColorFormat::LDR_RGB)
		_ldr_color_buffer.resize(width * height * 3);
	else if(_color_format == ColorFormat::HDR_RGB)
		_hdr_color_buffer.resize(width * height * 3);

	if(_depth_format == DepthFormat::FLOAT32)
		_depth_buffer_32.resize(width * height);
}

void FrameBuffer::clear_color(Color4 color)
{
	if (_color_format == ColorFormat::LDR_RGB)
	{
		int index = 0;
		while(index < _width * _height * 3)
		{
			_ldr_color_buffer[index++] = clamp<int>(color.r * 255.0f, 0, 255);
			_ldr_color_buffer[index++] = clamp<int>(color.g * 255.0f, 0, 255);
			_ldr_color_buffer[index++] = clamp<int>(color.b * 255.0f, 0, 255);
		}
	}
	else if (_color_format == ColorFormat::HDR_RGB)
	{
		int index = 0;
		while (index < _width * _height * 3)
		{
			_hdr_color_buffer[index++] = color.r;
			_hdr_color_buffer[index++] = color.g;
			_hdr_color_buffer[index++] = color.b;
		}
	}
}

void FrameBuffer::clear_depth(float depth)
{
	if (_depth_format == DepthFormat::FLOAT32)
	{
		for (int i = 0; i < _width * _height; i++)
			_depth_buffer_32[i] = depth;
	}
}

int FrameBuffer::width()
{
	return _width;
}

int FrameBuffer::height()
{
	return _height;
}

unsigned char* FrameBuffer::ldr_color_buffer_data()
{
	if (_color_format == ColorFormat::LDR_RGB)
		return _ldr_color_buffer.data();
	else
		return nullptr;
}

float* FrameBuffer::hdr_color_buffer_data()
{
	if (_color_format == ColorFormat::HDR_RGB)
		return _hdr_color_buffer.data();
	else
		return nullptr;
}

float* FrameBuffer::depth_buffer_data()
{
	if (_depth_format == DepthFormat::FLOAT32)
		return _depth_buffer_32.data();
	else
		return nullptr;
}

void FrameBuffer::set_color(int x, int y, const Color4& color)
{
	if (_color_format == ColorFormat::LDR_RGB)
	{
		_ldr_color_buffer[(y * _width + x) * 3 + 0] = clamp<int>(color[0] * 255.0f, 0, 255);
		_ldr_color_buffer[(y * _width + x) * 3 + 1] = clamp<int>(color[1] * 255.0f, 0, 255);
		_ldr_color_buffer[(y * _width + x) * 3 + 2] = clamp<int>(color[2] * 255.0f, 0, 255);
	}
	else if (_color_format == ColorFormat::HDR_RGB)
	{
		_hdr_color_buffer[(y * _width + x) * 3 + 0] = color[0];
		_hdr_color_buffer[(y * _width + x) * 3 + 1] = color[1];
		_hdr_color_buffer[(y * _width + x) * 3 + 2] = color[2];
	}
}

void FrameBuffer::set_depth(int x, int y, float depth)
{
	if (_depth_format == DepthFormat::FLOAT32)
		_depth_buffer_32[x + y * _width] = depth;
}

Color4 FrameBuffer::get_color(int x, int y) const
{
	if (_color_format == ColorFormat::LDR_RGB)
	{
		Color4 color;
		color.r = _ldr_color_buffer[(y * _width + x) * 3 + 0] / 255.0f;
		color.g = _ldr_color_buffer[(y * _width + x) * 3 + 1] / 255.0f;
		color.b = _ldr_color_buffer[(y * _width + x) * 3 + 2] / 255.0f;
		color.a = 1.0f;
		return color;
	}
	else if (_color_format == ColorFormat::HDR_RGB)
	{
		Color4 color;
		color.r = _hdr_color_buffer[(y * _width + x) * 3 + 0];
		color.g = _hdr_color_buffer[(y * _width + x) * 3 + 1];
		color.b = _hdr_color_buffer[(y * _width + x) * 3 + 2];
		color.a = 1.0f;
		return color;
	}
	else
		return Color4(0.0f);
}

float FrameBuffer::get_depth(int x, int y) const
{
	if (_depth_format == DepthFormat::FLOAT32)
		return _depth_buffer_32[x + y * _width];
	else
		return 0.0;
}

FrameBuffer::ColorFormat FrameBuffer::color_format() const
{
	return _color_format;
}

FrameBuffer::DepthFormat FrameBuffer::depth_format() const
{
	return _depth_format;
}
