#ifndef MESH_H
#define MESH_H

#include "renderdevice.h"
#include "texture.h"
#include <unordered_map>

struct ModelTexture
{
	std::shared_ptr<Texture> tex;
	std::string type_name;
};

class Mesh
{
public:

	VertexArray vertex_array;
	
	std::vector<ModelTexture> textures;

	std::unordered_map<std::string, Vec4> material_colors;

	void draw(std::shared_ptr<RenderDevice> device, FrameBuffer& frame_buffer, const Mat4& transform);
	
};

#endif