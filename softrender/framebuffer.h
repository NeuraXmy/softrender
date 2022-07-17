#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <vector>
#include "maths.h"


class FrameBuffer
{
public:

	enum class ColorFormat
	{
		LDR_RGB,
		HDR_RGB
	};

	enum class DepthFormat
	{
		None,
		FLOAT32
	};


	FrameBuffer(
		int width, 
		int height, 
		ColorFormat color_format = ColorFormat::LDR_RGB, 
		DepthFormat depth_format = DepthFormat::FLOAT32);

	void clear_color(Color4 color);

	void clear_depth(float depth);

	int width();

	int height();

	unsigned char* ldr_color_buffer_data();

	float* hdr_color_buffer_data();

	float* depth_buffer_data();

	void set_color(int x, int y, const Color4& color);

	void set_depth(int x, int y, float depth);

	Color4 get_color(int x, int y) const;

	float get_depth(int x, int y) const;

	ColorFormat color_format() const;

	DepthFormat depth_format() const;
	
private:

	int _width;
	int _height;
	
	std::vector<unsigned char> _ldr_color_buffer;
	std::vector<float>		   _hdr_color_buffer;

	std::vector<float> _depth_buffer_32;

	ColorFormat _color_format;
	DepthFormat _depth_format;
	
};

#endif