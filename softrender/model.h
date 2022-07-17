#ifndef MODEL_H
#define MODEL_H

#include "mesh.h"
#include <unordered_map>

struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;
enum aiTextureType;

class Model
{
public:
    std::shared_ptr<Model> parent = nullptr;

    Mat4 transform = trans::identity();
	

    Model();
    
    Model(std::string_view path);

    bool load(std::string_view path);
	
    void draw(std::shared_ptr<RenderDevice> device, FrameBuffer& framebuffer);

    void clear();

    bool empty() const;

    Mat4 get_global_transform() const;

    Vec3 get_centroid_position() const;

    Vec3 get_aabb_start_position() const;

    Vec3 get_aabb_end_position() const;
	
private:

    std::vector<Mesh> _meshes;
	
    std::string _directory;

    std::unordered_map<std::string, ModelTexture> _textures;

    Vec3 _centroid_position;
    Vec3 _aabb_start;
    Vec3 _aabb_end;

    int _position_count;


    void _process_node(aiNode* node, const aiScene* scene);
	
    void _process_mesh(aiMesh* mesh, const aiScene* scene);
	
    void _load_textures(Mesh& mesh, aiMaterial* mat, aiTextureType type, std::string_view type_name);

	
    void _load_material_color(Mesh& mesh, aiMaterial* mat, const char* pKey, unsigned int type, unsigned int idx, std::string_view name);
	
};

#endif