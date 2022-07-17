#include "model.h"
#include "texture.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/texture.h>
#include <iostream>


Model::Model()
{
	
}

Model::Model(std::string_view path)
{
	load(path);
}

bool Model::load(std::string_view path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path.data(), aiProcess_Triangulate | aiProcess_FlipUVs);
	
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cerr << "error when loading model " << path << " : " << importer.GetErrorString() << std::endl;
		return false;
	}
	
	int directory_len = path.find_last_of('/') + 1;
	if (!directory_len)
		directory_len = path.find_last_of('\\') + 1;
	_directory = path.substr(0, directory_len);

	_centroid_position = Vec3(0.0);
	_position_count = 0;
	_aabb_start = Vec3( std::numeric_limits<float>::max());
	_aabb_end   = Vec3(-std::numeric_limits<float>::max());

	_process_node(scene->mRootNode, scene);
	
	_centroid_position /= float(_position_count);
	
	return true;
}

void Model::draw(std::shared_ptr<RenderDevice> device, FrameBuffer& framebuffer)
{
	Mat4 global_transform = get_global_transform();
	for (auto& mesh : _meshes)
		mesh.draw(device, framebuffer, global_transform);
}

void Model::clear()
{
	_meshes.clear();
	_directory.clear();
	_textures.clear();
	transform = trans::identity();
}

bool Model::empty() const
{
	return _meshes.empty();
}

Mat4 Model::get_global_transform() const
{
	if (parent)
		return parent->get_global_transform() * transform;
	else
		return transform;
}

Vec3 Model::get_centroid_position() const
{
	return _centroid_position;
}

Vec3 Model::get_aabb_start_position() const
{
	return _aabb_start;
}

Vec3 Model::get_aabb_end_position() const
{
	return _aabb_end;
}

void Model::_process_node(aiNode* node, const aiScene* scene)
{
	for (size_t i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		_process_mesh(mesh, scene);
	}
	for (size_t i = 0; i < node->mNumChildren; i++)
	{
		_process_node(node->mChildren[i], scene);
	}
}

void Model::_process_mesh(aiMesh* mesh, const aiScene* scene)
{
	_meshes.emplace_back();
	auto& m = _meshes.back();

	for (size_t i = 0; i < mesh->mNumVertices; i++)
	{
		Vec4 pos;
		pos.x = mesh->mVertices[i].x;
		pos.y = mesh->mVertices[i].y;
		pos.z = mesh->mVertices[i].z;
		pos.w = 1.0f;

		++_position_count;
		_centroid_position += vec3(pos);
		_aabb_start = glm::min(_aabb_start, vec3(pos));
		_aabb_end	= glm::max(_aabb_end, vec3(pos));

		Vec4 texcoord;
		if (mesh->mTextureCoords[0])
		{
			texcoord.x = mesh->mTextureCoords[0][i].x;
			texcoord.y = mesh->mTextureCoords[0][i].y;
		}

		Vec4 normal;
		normal.x = mesh->mNormals[i].x;
		normal.y = mesh->mNormals[i].y;
		normal.z = mesh->mNormals[i].z;
		normal.w = 1.0f;
	
		Vertex v;
		v.attributes[0] = pos;
		v.attributes[1] = texcoord;
		v.attributes[2] = normal;
		m.vertex_array.vertices.push_back(v);
	}
	
	for (size_t i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (size_t j = 0; j < face.mNumIndices; j++)
			m.vertex_array.indices.push_back(face.mIndices[j]);
	}
	
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		// _load_material_color(m, material, AI_MATKEY_COLOR_AMBIENT, "color_ambient");
		// _load_material_color(m, material, AI_MATKEY_COLOR_DIFFUSE, "color_diffuse");
		// _load_material_color(m, material, AI_MATKEY_COLOR_SPECULAR, "color_specular");
		
		_load_textures(m, material, aiTextureType_AMBIENT, "texture_ambient");
		_load_textures(m, material, aiTextureType_DIFFUSE, "texture_diffuse");
		_load_textures(m, material, aiTextureType_SPECULAR, "texture_specular");
		_load_textures(m, material, aiTextureType_NORMALS, "texture_normal");
		_load_textures(m, material, aiTextureType_OPACITY, "texture_opacity");
	}
}

void Model::_load_textures(Mesh& mesh, aiMaterial* mat, aiTextureType type, std::string_view type_name)
{
	for (size_t i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString path;
		mat->GetTexture(type, i, &path);
		
		auto it = _textures.find(path.C_Str());
		if (it != _textures.end())
		{
			mesh.textures.push_back(it->second);
			continue;
		}
		
		ModelTexture tex;
		tex.tex = std::make_shared<Texture>(_directory + path.C_Str(), false);
		tex.type_name = type_name;

		mesh.textures.push_back(tex);
		_textures[path.C_Str()] = tex;
	}
}

void Model::_load_material_color(Mesh& mesh, aiMaterial* mat, const char* pKey, unsigned int type, unsigned int idx, std::string_view name)
{
	aiColor4D color;
	auto ret = mat->Get(pKey, type, idx, color);
	if(ret != aiReturn_SUCCESS)
		return;
	mesh.material_colors[name.data()] = Vec4(color.r, color.g, color.b, color.a);
}
