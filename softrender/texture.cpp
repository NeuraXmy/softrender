#include "texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <iostream>

Texture::Texture()
{
}

Texture::Texture(int w, int h, ColorFormat format)
{
	create(w, h, format);
}

Texture::Texture(std::string_view path, bool filp, ColorFormat format)
{
	load(path, filp, format);
}

void Texture::clear()
{
	_width = 0;
	_height = 0;
	_ldr_color_buffer.clear();
	_hdr_color_buffer.clear();
}

bool Texture::empty()
{
	return _ldr_color_buffer.empty() && _hdr_color_buffer.empty();
}

void Texture::create(int w, int h, ColorFormat format)
{
	clear();
	_width = w;
	_height = h;
	_color_format = format;
	if(_color_format == ColorFormat::LDR_RGBA)
		_ldr_color_buffer.resize(_width * _height * 4);
	else if(_color_format == ColorFormat::HDR_RGBA)
		_hdr_color_buffer.resize(_width * _height * 4);
}

bool Texture::load(std::string_view path, bool flip, ColorFormat format)
{
	clear();
	_color_format = format;

	stbi_set_flip_vertically_on_load(flip);

	if (_color_format == ColorFormat::LDR_RGBA)
	{
		int channel = 0;
		unsigned char* data = stbi_load(path.data(), &_width, &_height, &channel, 0);
		if (data == nullptr)
		{
			clear();
			std::cerr << "failed to load texture: " << path << std::endl;
			return false;
		}

		_ldr_color_buffer.resize(_width * _height * 4);
		int i = 0, j = 0;
		for (int k = 0; k < _width * _height; k++)
		{
			_ldr_color_buffer[i + 0] = channel > 0 ? data[j + 0] : 0;
			_ldr_color_buffer[i + 1] = channel > 1 ? data[j + 1] : 0;
			_ldr_color_buffer[i + 2] = channel > 2 ? data[j + 2] : 0;
			_ldr_color_buffer[i + 3] = channel > 3 ? data[j + 3] : 255;
			i += 4;
			j += channel;
		}
		stbi_image_free(data);
	}
	else if (_color_format == ColorFormat::HDR_RGBA)
	{
		int channel = 0;
		float* data = stbi_loadf(path.data(), &_width, &_height, &channel, 0);
		if (data == nullptr)
		{
			clear();
			std::cerr << "failed to load texture: " << path << std::endl;
			return false;
		}
		
		_hdr_color_buffer.resize(_width * _height * 4);
		int i = 0, j = 0;
		for (int k = 0; k < _width * _height; k++)
		{
			_hdr_color_buffer[i + 0] = channel > 0 ? data[j + 0] : 0.0f;
			_hdr_color_buffer[i + 1] = channel > 1 ? data[j + 1] : 0.0f;
			_hdr_color_buffer[i + 2] = channel > 2 ? data[j + 2] : 0.0f;
			_hdr_color_buffer[i + 3] = channel > 3 ? data[j + 3] : 1.0f;
			i += 4;
			j += channel;
		}
		stbi_image_free(data);
	}

	return true;
}

void Texture::save(std::string_view path) const
{
	if (_color_format == ColorFormat::LDR_RGBA)
	{
		stbi_write_png(path.data(), _width, _height, 4, _ldr_color_buffer.data(), _width * 4);
	}
	else if (_color_format == ColorFormat::HDR_RGBA)
	{
		stbi_write_hdr(path.data(), _width, _height, 4, _hdr_color_buffer.data());
	}
}

int Texture::width() const
{
	return _width;
}

int Texture::height() const
{
	return _height;
}

unsigned char* Texture::ldr_color_buffer_data()
{
	return _ldr_color_buffer.data();
}

float* Texture::hdr_color_buffer_data()
{
	return _hdr_color_buffer.data();
}

Texture::ColorFormat Texture::color_format() const
{
	return _color_format;
}

void Texture::set_color(int x, int y, const Color4& color)
{
	if(_color_format == ColorFormat::LDR_RGBA)
	{
		int index = (y * _width + x) * 4;
		_ldr_color_buffer[index + 0] = clamp<int>(color.r * 255.0f, 0, 255);
		_ldr_color_buffer[index + 1] = clamp<int>(color.g * 255.0f, 0, 255);
		_ldr_color_buffer[index + 2] = clamp<int>(color.b * 255.0f, 0, 255);
		_ldr_color_buffer[index + 3] = clamp<int>(color.a * 255.0f, 0, 255);
	}
	else if(_color_format == ColorFormat::HDR_RGBA)
	{
		int index = (y * _width + x) * 4;
		_hdr_color_buffer[index + 0] = color.r;
		_hdr_color_buffer[index + 1] = color.g;
		_hdr_color_buffer[index + 2] = color.b;
		_hdr_color_buffer[index + 3] = color.a;
	}
}

Color4 Texture::get_color(int x, int y) const
{
	if (!_width || !_height)
		return Color::TRANSPARENT;
	int tx = x;
	int ty = y;
	if (x < 0 || y < 0 || x >= _width || y >= _height)
	{
		int w = _width;
		int h = _height;
		if (warpMode == WarpMode::REPEAT)
		{
			x = (x % w + w) % w;
			y = (y % h + h) % h;
		}
		else if (warpMode == WarpMode::MIRRORED_REPEAT)
		{
			x = (x % (w * 2) + w * 2) % (w * 2);
			y = (y % (h * 2) + h * 2) % (h * 2);
			if (x >= w) x = 2 * w - x - 1;
			if (y >= h) y = 2 * h - y - 1;
		}
		else if (warpMode == WarpMode::CLAMP_TO_EDGE)
		{
			x = clamp(x, 0, w - 1);
			y = clamp(y, 0, h - 1);
		}
		else if (warpMode == WarpMode::CLAMP_TO_BORDER)
		{
			return Color4(0.0f);
		}
	}
	
	if(_color_format == ColorFormat::LDR_RGBA)
	{
		int index = (y * _width + x) * 4;
		return Color4(
			_ldr_color_buffer[index + 0] / 255.0f, 
			_ldr_color_buffer[index + 1] / 255.0f, 
			_ldr_color_buffer[index + 2] / 255.0f, 
			_ldr_color_buffer[index + 3] / 255.0f);
	}
	else if(_color_format == ColorFormat::HDR_RGBA)
	{
		int index = (y * _width + x) * 4;
		return Color4(
			_hdr_color_buffer[index + 0], 
			_hdr_color_buffer[index + 1], 
			_hdr_color_buffer[index + 2], 
			_hdr_color_buffer[index + 3]);
	}
	return Color4();
}

Color4 Texture::sample(float x, float y) const
{
	x *= _width;
	y *= _height;
	
	if (sampleMode == SampleMode::NEAREST)
	{
		return get_color(floor(x), floor(y));
	}
	else if (sampleMode == SampleMode::BILINEAR)
	{
		int lbx = floor(x - 0.5f);
		int lby = floor(y - 0.5f);
		float tx = x - (lbx + 0.5f);
		float ty = y - (lby + 0.5f);
		Color4 c0 = lerp(get_color(lbx, lby    ), get_color(lbx + 1, lby    ), tx);
		Color4 c1 = lerp(get_color(lbx, lby + 1), get_color(lbx + 1, lby + 1), tx);
		return lerp(c0, c1, ty);
	}
	else if (sampleMode == SampleMode::BICUBIC)
	{
		int lbx = floor(x - 0.5f);
		int lby = floor(y - 0.5f);
		float tx = x - (lbx + 0.5f);
		float ty = y - (lby + 0.5f);

		float wx[] =
		{
			0.5f * (-tx + 2.0f * tx * tx + -tx * tx * tx),
			0.5f * (2.0f - 5.0f * tx * tx + 3.0f * tx * tx * tx),
			0.5f * (tx + 4.0f * tx * tx - 3.0f * tx * tx * tx),
			0.5f * (-tx * tx + tx * tx * tx)
		};
		float wy[] = 
		{
			0.5f * (-ty + 2.0f * ty * ty + -ty * ty * ty),
			0.5f * (2.0f - 5.0f * ty * ty + 3.0f * ty * ty * ty),
			0.5f * (ty + 4.0f * ty * ty - 3.0f * ty * ty * ty),
			0.5f * (-ty * ty + ty * ty * ty)
		};

		Color4 cx[4];
		for (int i = 0; i < 4; i++)
		{
			cx[i] = vec4(0.0f, 0.0f, 0.0f, 0.0f);
			for (int j = 0; j < 4; j++)
				cx[i] += get_color(lbx + j - 1, lby + i - 1) * wx[j];
		}
		Color4 cy = vec4(0.0f, 0.0f, 0.0f, 0.0f);
		for (int i = 0; i < 4; i++)
			cy += cx[i] * wy[i];
		return cy;
	}
}

Color4 TextureSampler::sample(float x, float y) const
{
	return _texture ? _texture->sample(x, y) : _default_color;
}

Color4 TextureSampler::sample(const Vec2& texcoord) const
{
	return _texture ? sample(texcoord.x, texcoord.y) : _default_color;
}

bool TextureSampler::empty() const
{
	return !_texture;
}
