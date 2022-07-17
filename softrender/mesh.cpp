#include "mesh.h"
#include <unordered_map>

void Mesh::draw(std::shared_ptr<RenderDevice> device, FrameBuffer& framebuffer, const Mat4& transform)
{
	std::unordered_map<std::string, size_t> type_num;
	for (auto& [tex, type_name] : textures)
	{
		std::string name = "material." + type_name + std::to_string(type_num[type_name]++);
		device->set_shader_uniform(name, TextureSampler(tex));
	}
	for (auto& [name, color] : material_colors)
		device->set_shader_uniform("material." + name, color);

	device->set_shader_uniform("transform.model", transform);

	device->draw(framebuffer, vertex_array);
}
