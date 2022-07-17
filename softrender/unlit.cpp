#include "unlit.h"

namespace Transform
{
	static thread_local Mat4 model;
	static thread_local Mat4 view;
	static thread_local Mat4 projection;
	static thread_local Mat4 modelview;
};

void Unlit::VS::load_uniforms()
{
	Transform::model			= get_uniform<Mat4>("transform.model");
	Transform::view				= get_uniform<Mat4>("transform.view");
	Transform::projection		= get_uniform<Mat4>("transform.projection");
	Transform::modelview		= Transform::model * Transform::view;
}

void Unlit::VS::run(const VSIn& in, VSOut& out)
{
	Vec3 in_position	= vec3(in.attributes[ATTR_position]);
	Vec2 in_texcoord	= vec2(in.attributes[ATTR_texcoord]);

	auto& out_position	= out.out_varying[VARY_position];
	auto& out_texcoord	= out.out_varying[VARY_texcoord];

	out.position = vec4(in_position) * Transform::modelview * Transform::projection;

	out_position = vec4(in_position) * Transform::modelview;
	out_texcoord = vec4(in_texcoord);
}

static thread_local Vec3 camera_pos;
static thread_local float gamma;
static thread_local float exposure;
namespace Material
{
	static Color3 color_ambient;
	static Color3 color_diffuse;
	static thread_local TextureSampler texture_ambient0;
	static thread_local TextureSampler texture_diffuse0;
};

void Unlit::FS::load_uniforms()
{
	camera_pos = get_uniform<Vec3>("camera_pos");
	gamma = get_uniform<float>("gamma", 2.2f);
	exposure = get_uniform<float>("exposure", 1.0f);
	Material::color_ambient		= vec3(get_uniform<Color4>("material.color_ambient", Color::BLACK));
	Material::color_diffuse		= vec3(get_uniform<Color4>("material.color_diffuse", Color::BLACK));
	Material::texture_ambient0	= get_uniform<TextureSampler>("material.texture_ambient0", TextureSampler(Color::BLACK));
	Material::texture_diffuse0	= get_uniform<TextureSampler>("material.texture_diffuse0", TextureSampler(Color::BLACK));
}

void Unlit::FS::run(const FSIn& in, FSOut& out)
{
	Vec3 in_position = vec3(in.in_varying[VARY_position]);
	Vec2 in_texcoord = vec2(in.in_varying[VARY_texcoord]);

	Vec3 ambient_color = vec3(Material::texture_ambient0.sample(in_texcoord));
	Vec3 diffuse_color = vec3(Material::texture_diffuse0.sample(in_texcoord));
	
	Vec3 color = vec3(Color::BLACK);
	color = glm::max(color, ambient_color);
	color = glm::max(color, diffuse_color);
	color = glm::max(color, Material::color_ambient);
	color = glm::max(color, Material::color_diffuse);

	color = glm::pow(color, vec3(1.0f / gamma));
	if (exposure > 0.0f)
		color = vec3(1.0f) - glm::exp(-color * exposure);

	out.color = vec4(color, 1.0f);
}
