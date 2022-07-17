#ifndef RENDER_DEVICE_H
#define RENDER_DEVICE_H

#include <unordered_map>
#include <vector>
#include <string>
#include <any>
#include <memory>
#include "renderstates.h"
#include "framebuffer.h"

constexpr int MAX_VARYING_NUM = 5;

struct ShaderProgram;

struct VSIn
{
	Vec4 attributes[MAX_VARYING_NUM];
};
struct VSOut
{
	Vec4 position;
	Vec4 out_varying[MAX_VARYING_NUM];
};
struct FSIn
{
	FSIn() = default;
	FSIn(const VSOut& vsout, size_t varying_num) 
	{
		memcpy(in_varying, vsout.out_varying, sizeof(Vec4) * varying_num);
	}
	Vec4 in_varying[MAX_VARYING_NUM];
};
struct FSOut
{
	Vec4 color;
	bool discarded = false;
};

using Vertex = VSIn;
using VertexBuffer = std::vector<Vertex>;
using IndexBuffer = std::vector<size_t>;

struct VertexArray
{
	VertexBuffer vertices;
	IndexBuffer indices;
};

class RenderDevice : public std::enable_shared_from_this<RenderDevice>
{
public:

	RenderDevice();

	~RenderDevice();

	void set_shader_program(const ShaderProgram& program);

	ShaderProgram& shader_program();

	template<class T>
	void set_shader_uniform(std::string_view name, const T& value)
	{
		_shader_uniforms[name.data()] = value;
	}

	template<class T>
	T get_shader_uniform(std::string_view name, const T& default_val = T()) const
	{
		auto it = _shader_uniforms.find(name.data());
		return it != _shader_uniforms.end() ? std::any_cast<T>(it->second) : default_val;
	}

	bool has_shader_uniform(std::string_view name) const
	{
		return _shader_uniforms.find(name.data()) != _shader_uniforms.end();
	}

	void clear_shader_uniform(std::string_view name)
	{
		_shader_uniforms.erase(name.data());
	}

	void clear_shader_uniforms()
	{
		_shader_uniforms.clear();
	}

	void shrink_buffer_size();

	RenderStates& render_states();

	const RenderStates& render_states() const;

	void draw(FrameBuffer& framebuffer, VertexArray vertex_array);

private:

	RenderStates _render_states;

	std::unordered_map<std::string, std::any> _shader_uniforms;

	std::unique_ptr<ShaderProgram> _shader_program = std::make_unique<ShaderProgram>();

	
	struct Point
	{
		VSOut v;
		bool culled = false;
	};

	struct Line
	{
		VSOut v[2];
		bool culled = false;
	};

	struct Triangle
	{
		VSOut v[3];
		bool culled = false;

		void reverse_order()
		{
			std::swap(v[0], v[1]);
		}
	};

	struct Fragment
	{
		Vec4 color;
		int x, y;
		float depth;
		float inv_w;
		bool discarded = false;
	};


	std::vector<VSOut>		_vsout_buffer;
	std::vector<Point>		_point_buffer;
	std::vector<Line>		_line_buffer;
	std::vector<Triangle>	_triangle_buffer;
	std::vector<FSIn>		_fsin_buffer;
	std::vector<Fragment>   _fragment_buffer;
	

	void _run_vertex_shader(const VertexBuffer& vertices);

	void _assemble_points(const IndexBuffer& indices);

	void _assemble_lines(const IndexBuffer& indices);

	void _assemble_triangles(const IndexBuffer& indices);

	void _clipping_points();

	void _clipping_lines();

	void _clipping_triangles();

	void _face_culling();

	void _to_viewport();

	void _rasterize_points();
	
	void _rasterize_lines();
	
	void _rasterize_triangles();

	void _early_z_test(FrameBuffer& framebuffer);
	
	void _run_fragment_shader();

	void _fragment_test(FrameBuffer& framebuffer);

	void _post_processing(FrameBuffer& framebuffer);


	enum class ClipPlane
	{
		LEFT,
		RIGHT,
		BOTTOM,
		TOP,
		NEAR,
		FAR
	};


	void clear_buffers();

	size_t add_vsout(const VSOut& vertex);

	void add_point(size_t i);

	void add_line(size_t i, size_t j);

	void add_triangle(size_t i, size_t j, size_t k);

	void draw_point(const VSOut& v);
	
	void draw_line(const VSOut& s, const VSOut& t);

	void draw_triangle(const VSOut& a, const VSOut& b, const VSOut& c);
	
	void clip_lines_by_plane(ClipPlane plane);

	void clip_triangles_by_plane(ClipPlane plane);

	VSOut interpolation_vsout(const VSOut& a, const VSOut& b, float t) const;

	VSOut interpolation_vsout(const VSOut& a, const VSOut& b, const VSOut& c, float t0, float t1, float t2) const;

	void vsout_to_viewport(VSOut& vsout);

	float get_clip_interpolation_ratio(const Vec4& a, const Vec4& b, ClipPlane plane) const;

	float check_in_clip_plane(const Vec4& p, ClipPlane plane) const;
	
};

#endif 
