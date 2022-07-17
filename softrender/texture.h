#ifndef TEXTURE_H
#define TEXTURE_H

#include <vector>
#include <string>
#include <memory>
#include "maths.h"

class Texture
{
public:
	
	enum class ColorFormat
	{
		LDR_RGBA,
		HDR_RGBA
	};

	enum class SampleMode
	{
		NEAREST,
		BILINEAR,
		BICUBIC
	};

	enum class WarpMode
	{
		REPEAT,
		MIRRORED_REPEAT,
		CLAMP_TO_EDGE,
		CLAMP_TO_BORDER
	};

	
	SampleMode sampleMode = SampleMode::NEAREST;
	WarpMode warpMode = WarpMode::REPEAT;
	
	
	Texture();

	Texture(int w, int h, ColorFormat format = ColorFormat::LDR_RGBA);

	Texture(std::string_view path, bool flip = true, ColorFormat format = ColorFormat::LDR_RGBA);


	void clear();

	bool empty();

	void create(int w, int h, ColorFormat format = ColorFormat::LDR_RGBA);

	bool load(std::string_view path, bool flip = true, ColorFormat format = ColorFormat::LDR_RGBA);

	void save(std::string_view path) const;
	

	int width() const;

	int height() const;
	
	unsigned char* ldr_color_buffer_data();

	float* hdr_color_buffer_data();

	ColorFormat color_format() const;


	void set_color(int x, int y, const Color4& color);

	Color4 get_color(int x, int y) const;

	Color4 sample(float x, float y) const;

	
private:

	int _width = 0;
	int _height = 0;

	std::vector<unsigned char> _ldr_color_buffer;
	std::vector<float>		   _hdr_color_buffer;

	ColorFormat _color_format = ColorFormat::LDR_RGBA;
	
};

struct TextureSampler
{
	TextureSampler(std::shared_ptr<Texture> texture = nullptr) : _texture(texture) {}

	TextureSampler(const Color4& default_color) : _default_color(default_color) {}

	Color4 sample(float x, float y) const;

	Color4 sample(const Vec2& texcoord) const;

	bool empty() const;

private:

	std::shared_ptr<Texture> _texture = nullptr;

	Color4 _default_color = Color::TRANSPARENT;
	
};

#endif