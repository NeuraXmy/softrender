#include "phong.h"

namespace Transform
{
	static thread_local Mat4 model;
	static thread_local Mat4 view;
	static thread_local Mat4 projection;
	static thread_local Mat4 modelview;
	static thread_local Mat3 normal_tranform;
};

void Phong::VS::load_uniforms()
{
	Transform::model			= get_uniform<Mat4>("transform.model");
	Transform::view				= get_uniform<Mat4>("transform.view");
	Transform::projection		= get_uniform<Mat4>("transform.projection");
	Transform::modelview		= Transform::model * Transform::view;
	Transform::normal_tranform	= trans::normalTransformMat(Transform::modelview);
}

void Phong::VS::run(const VSIn& in, VSOut& out)
{
	Vec3 in_position	= vec3(in.attributes[ATTR_position]);
	Vec3 in_normal		= vec3(in.attributes[ATTR_normal]);
	Vec2 in_texcoord	= vec2(in.attributes[ATTR_texcoord]);

	auto& out_position	= out.out_varying[VARY_position];
	auto& out_normal	= out.out_varying[VARY_normal];
	auto& out_texcoord	= out.out_varying[VARY_texcoord];

	out.position = vec4(in_position) * Transform::modelview * Transform::projection;

	out_position = vec4(in_position) * Transform::modelview;
	out_texcoord = vec4(in_texcoord);
	out_normal = vec4(Transform::normal_tranform * in_normal);
}

static thread_local Vec3 camera_pos;
static thread_local float gamma;
static thread_local float exposure;
namespace Material
{
	static Color3 color_ambient;
	static Color3 color_diffuse;
	static Color3 color_specular;
	static thread_local TextureSampler texture_ambient0;
	static thread_local TextureSampler texture_diffuse0;
	static thread_local TextureSampler texture_specular0;
};

void Phong::FS::load_uniforms()
{
	camera_pos = get_uniform<Vec3>("camera_pos");
	gamma = get_uniform<float>("gamma", 2.2f);
	exposure = get_uniform<float>("exposure", 1.0f);
	Material::color_ambient		= vec3(get_uniform<Color4>("material.color_ambient",  Color::WHITE));
	Material::color_diffuse     = vec3(get_uniform<Color4>("material.color_diffuse",  Color::WHITE));
	Material::color_specular    = vec3(get_uniform<Color4>("material.color_specular", Color::WHITE));
	Material::texture_ambient0  = get_uniform<TextureSampler>("material.texture_ambient0",  TextureSampler(Color::WHITE));
	Material::texture_diffuse0  = get_uniform<TextureSampler>("material.texture_diffuse0",  TextureSampler(Color::WHITE));
	Material::texture_specular0 = get_uniform<TextureSampler>("material.texture_specular0", TextureSampler(Color::BLACK));
	
	if (Material::texture_ambient0.empty()) Material::texture_ambient0 = Material::texture_diffuse0;
}

void Phong::FS::run(const FSIn& in, FSOut& out)
{
	Vec3 in_position	= vec3(in.in_varying[VARY_position]);
	Vec3 in_normal		= vec3(in.in_varying[VARY_normal]);
	Vec2 in_texcoord	= vec2(in.in_varying[VARY_texcoord]);

	Vec3 light_dir = Vec3(2.0f, 1.0f, 1.0f);
	Vec3 n = glm::normalize(in_normal);
	Vec3 d = glm::normalize(light_dir);
	Vec3 h = glm::normalize((camera_pos - in_position + d) * 0.5f);

	float ambient  = 0.2f;
	float diffuse  = std::max(0.0f, glm::dot(n, d));
	float specular = glm::pow(std::max(0.0f, glm::dot(n, h)), 64.0f);

	Vec3 ambient_color = vec3(Material::texture_ambient0.sample(in_texcoord));
	Vec3 diffuse_color = vec3(Material::texture_diffuse0.sample(in_texcoord));
	Vec3 specular_color = vec3(Material::texture_specular0.sample(in_texcoord));

	Vec3 color = ambient_color  * ambient  * Material::color_ambient
			   + diffuse_color  * diffuse  * Material::color_diffuse
			   + specular_color * specular * Material::color_specular;

	color = glm::pow(color, vec3(1.0f / gamma));
	if(exposure > 0.0f)
		color = vec3(1.0f) - glm::exp(-color * exposure);
	
	out.color = vec4(color, 1.0f);
}
