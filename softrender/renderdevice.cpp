#include "renderdevice.h"
#include "shader.h"
#include "framebuffer.h"
#include <algorithm>
#include <iostream>
#include <thread>
#include <mutex>

RenderDevice::RenderDevice()
{
	
}

RenderDevice::~RenderDevice()
{

}

void RenderDevice::set_shader_program(const ShaderProgram& program)
{
	assert(program.vertex_shader && program.fragment_shader);
	assert(program.varying_num <= MAX_VARYING_NUM);
	*_shader_program = program;
	_shader_program->vertex_shader->set_device(shared_from_this());
	_shader_program->fragment_shader->set_device(shared_from_this());
}

ShaderProgram& RenderDevice::shader_program()
{
	return *_shader_program;
}

void RenderDevice::shrink_buffer_size()
{
	clear_buffers();
	_vsout_buffer.shrink_to_fit();
	_point_buffer.shrink_to_fit();
	_line_buffer.shrink_to_fit();
	_triangle_buffer.shrink_to_fit();
	_fsin_buffer.shrink_to_fit();
	_fragment_buffer.shrink_to_fit();
}

RenderStates& RenderDevice::render_states()
{
	return _render_states;
}

const RenderStates& RenderDevice::render_states() const
{
	return _render_states;
}


void RenderDevice::draw(FrameBuffer& framebuffer, VertexArray vertex_array)
{
	auto& [vertices, indices] = vertex_array;

	if (indices.empty() && vertices.size()) {
		indices.resize(vertices.size());
		int ind = 0;
		for (auto& x : indices) x = ind++;
	}
	if (_render_states.viewport.w == 0)
	{
		_render_states.viewport.w = framebuffer.width();
		_render_states.viewport.h = framebuffer.height();
	}

	clear_buffers();

	_run_vertex_shader(vertices);
	
	switch (_render_states.primitive_mode)
	{
	case PrimitiveMode::POINTS:
		_assemble_points(indices);
		_clipping_points();
		_to_viewport();
		_rasterize_points();
		break;

	case PrimitiveMode::LINES: 
	case PrimitiveMode::LINE_STRIPE:
	case PrimitiveMode::LINE_LOOP:
		 _assemble_lines(indices);
		 _clipping_lines();
		 _to_viewport();
		 _rasterize_lines();
		break;

	case PrimitiveMode::TRIANGLES:
	case PrimitiveMode::TRIANGLE_STRIPE:
	case PrimitiveMode::TRIANGLE_FAN:
	case PrimitiveMode::QUADS:
		 _assemble_triangles(indices);
		 _clipping_triangles();
		 _to_viewport();
		 _face_culling();
		 _rasterize_triangles();
		break;
	}

	_early_z_test(framebuffer);
	 
	_run_fragment_shader();

	_fragment_test(framebuffer);

	_post_processing(framebuffer);
			
}


void RenderDevice::_run_vertex_shader(const VertexBuffer& vertices)
{
	assert(_shader_program->vertex_shader);
	auto vs = _shader_program->vertex_shader;
	vs->load_uniforms();
	for (auto& vsin : vertices)
	{
		VSOut result;
		vs->run(vsin, result);
		_vsout_buffer.push_back(result);
	}
}

void RenderDevice::_assemble_points(const IndexBuffer& indices)
{
	for (size_t i = 0; i < indices.size(); i++)
		add_point(indices[i]);
}

void RenderDevice::_assemble_lines(const IndexBuffer& indices)
{
	if (_render_states.primitive_mode == PrimitiveMode::LINES)
	{
		for(size_t i = 0; i + 1 < indices.size(); i += 2)
			add_line(indices[i], indices[i + 1]);
	}
	else if (_render_states.primitive_mode == PrimitiveMode::LINE_STRIPE)
	{
		if(indices.size() >= 2)
			add_line(indices[0], indices[1]);
		for (size_t i = 2; i < indices.size(); i++)
			add_line(indices[i - 1], indices[i]);
	}
	else if (_render_states.primitive_mode == PrimitiveMode::LINE_LOOP)
	{
		for (size_t i = 0; i < indices.size(); i++)
			add_line(indices[i], indices[(i + 1) % indices.size()]);
	}
}

void RenderDevice::_assemble_triangles(const IndexBuffer& indices)
{
	if (_render_states.primitive_mode == PrimitiveMode::TRIANGLES)
	{
		for(size_t i = 0; i + 2 < indices.size(); i += 3)
			add_triangle(indices[i], indices[i + 1], indices[i + 2]);
	}
	else if(_render_states.primitive_mode == PrimitiveMode::TRIANGLE_STRIPE)
	{
		if(indices.size() >= 3)
			add_triangle(indices[0], indices[1], indices[2]);
		for (size_t i = 3; i < indices.size(); i += 2)
		{
			add_triangle(indices[i - 2], indices[i - 1], indices[i]);
			if (i & 1)
				_triangle_buffer.back().reverse_order();
		}
	}
	else if (_render_states.primitive_mode == PrimitiveMode::TRIANGLE_FAN)
	{
		for(size_t i = 0; i + 1 < indices.size(); i++)
			add_triangle(indices[0], indices[i + 1], indices[i + 2]);
	}
	else if (_render_states.primitive_mode == PrimitiveMode::QUADS)
	{
		for (size_t i = 0; i + 3 < indices.size(); i += 4)
		{
			add_triangle(indices[i], indices[i + 1], indices[i + 2]);
			add_triangle(indices[i], indices[i + 2], indices[i + 3]);
		}
	}
}

void RenderDevice::_clipping_points()
{
	for (auto& point : _point_buffer)
	{
		auto& p = point.v;
		float w = p.position.w;
		if (p.position.x <= -w || p.position.x >= w
			|| p.position.y <= -w || p.position.y >= w
			|| p.position.z <= 0 || p.position.z >= w)
			point.culled = true;
	}
}

void RenderDevice::_clipping_lines()
{
	clip_lines_by_plane(ClipPlane::LEFT);
	clip_lines_by_plane(ClipPlane::RIGHT);
	clip_lines_by_plane(ClipPlane::BOTTOM);
	clip_lines_by_plane(ClipPlane::TOP);
	clip_lines_by_plane(ClipPlane::NEAR);
	clip_lines_by_plane(ClipPlane::FAR);
}

void RenderDevice::_clipping_triangles()
{
	clip_triangles_by_plane(ClipPlane::LEFT);
	clip_triangles_by_plane(ClipPlane::RIGHT);
	clip_triangles_by_plane(ClipPlane::BOTTOM);
	clip_triangles_by_plane(ClipPlane::TOP);
	clip_triangles_by_plane(ClipPlane::NEAR);
	clip_triangles_by_plane(ClipPlane::FAR);
}

void RenderDevice::_face_culling()
{
	if (_render_states.cull_face_mode != CullFaceMode::NONE)
	{
		auto order = _render_states.front_vertex_order;
		if (_render_states.cull_face_mode == CullFaceMode::BACK)
			order = order == FrontVertexOrder::CLOCKWISE ? FrontVertexOrder::COUNTER_CLOCKWISE : FrontVertexOrder::CLOCKWISE;
		for (auto& triangle : _triangle_buffer)
		{
			if (triangle.culled) continue;
			auto& v0 = triangle.v[0];
			auto& v1 = triangle.v[1];
			auto& v2 = triangle.v[2];
			auto d1 = vec2(v1.position - v0.position);
			auto d2 = vec2(v2.position - v1.position);
			float s = d1.x * d2.y - d1.y * d2.x;
			if (order == FrontVertexOrder::CLOCKWISE && s < 0.0f
				|| order == FrontVertexOrder::COUNTER_CLOCKWISE && s > 0.0f)
			{
				triangle.culled = true;
			}
		}
	}
}

void RenderDevice::_to_viewport()
{
	for (auto& point : _point_buffer)
		vsout_to_viewport(point.v);
	for (auto& line : _line_buffer)
		for (int i = 0; i < 2; i++)
			vsout_to_viewport(line.v[i]);
	for (auto& triangle : _triangle_buffer)
		for (int i = 0; i < 3; i++)
			vsout_to_viewport(triangle.v[i]);
}

void RenderDevice::_rasterize_points()
{
	for (auto& point : _point_buffer)
	{
		if (point.culled) continue;
		draw_point(point.v);
	}
}

void RenderDevice::_rasterize_lines()
{
	for (auto& line : _line_buffer)
	{
		if (line.culled) continue;
		if (_render_states.polygon_mode == PolygonMode::POINTED)
		{
			draw_point(line.v[0]);
			draw_point(line.v[1]);
		}
		else
		{
			draw_line(line.v[0], line.v[1]);
		}
	}
}

void RenderDevice::_rasterize_triangles()
{
	for (auto& triangle : _triangle_buffer)
	{
		if (triangle.culled) continue;

		auto& v0 = triangle.v[0];
		auto& v1 = triangle.v[1];
		auto& v2 = triangle.v[2];
		
		if (_render_states.polygon_mode == PolygonMode::POINTED)
		{
			draw_point(v0);
			draw_point(v1);
			draw_point(v2);
		}
		else if (_render_states.polygon_mode == PolygonMode::WIREFRAME)
		{
			draw_line(v0, v1);
			draw_line(v1, v2);
			draw_line(v2, v0);
		}
		else
		{
			draw_triangle(v0, v1, v2);
		}
	}
}

void RenderDevice::_early_z_test(FrameBuffer& framebuffer)
{
	if (_render_states.depth_test && _render_states.eary_z_test)
	{
		assert(framebuffer.depth_format() != FrameBuffer::DepthFormat::None);
		for(auto& fragment : _fragment_buffer)
		{
			int x = fragment.x;
			int y = fragment.y;
			if (x < 0 || y < 0 || x >= framebuffer.width() || y >= framebuffer.height())
				continue;
			if (fragment.depth <= framebuffer.get_depth(x, y))
			{
				if (!_render_states.depth_mask)
					framebuffer.set_depth(x, y, fragment.depth);
			}
			else
				fragment.discarded = true;
		}
	}
}

void RenderDevice::_run_fragment_shader()
{
	for (int i = 0; i < _fsin_buffer.size(); i++)
		for (int j = 0; j < _shader_program->varying_num; j++)
			_fsin_buffer[i].in_varying[j] /= _fragment_buffer[i].inv_w;
	
	assert(_shader_program->fragment_shader);
	auto fs = _shader_program->fragment_shader;

	auto run_fs = [fs](const FSIn& fsin, Fragment& fragment) {
		FSOut result;
		fs->run(fsin, result);
		fragment.color = result.color;
		fragment.discarded |= result.discarded;
	};

	fs->load_uniforms();
	for (int i = 0; i < _fsin_buffer.size(); i++)
		run_fs(_fsin_buffer[i], _fragment_buffer[i]);
}

void RenderDevice::_fragment_test(FrameBuffer& framebuffer)
{
	for (auto& fragment : _fragment_buffer)
	{
		if (fragment.discarded) continue;
		int x = fragment.x;
		int y = fragment.y;
		if (x < 0 || y < 0 || x >= framebuffer.width() || y >= framebuffer.height())
			continue;

		if (_render_states.alpha_test)
			if (fragment.color.a < _render_states.alpha_test_threshold)
				continue;

		if (_render_states.depth_test && !_render_states.eary_z_test)
		{
			assert(framebuffer.depth_format() != FrameBuffer::DepthFormat::None);
			if (fragment.depth <= framebuffer.get_depth(x, y))
			{
				if (!_render_states.depth_mask)
					framebuffer.set_depth(x, y, fragment.depth);
				if (!_render_states.color_mask)
					framebuffer.set_color(fragment.x, fragment.y, fragment.color);
			}
		}
		else
		{
			if (!_render_states.color_mask)
				framebuffer.set_color(fragment.x, fragment.y, fragment.color);
		}
	}
}

void RenderDevice::_post_processing(FrameBuffer& framebuffer)
{
	
}


void RenderDevice::clear_buffers()
{
	_vsout_buffer.clear();
	_point_buffer.clear();
	_line_buffer.clear();
	_triangle_buffer.clear();
	_fsin_buffer.clear();
	_fragment_buffer.clear();
}

size_t RenderDevice::add_vsout(const VSOut& vertex)
{
	_vsout_buffer.push_back(vertex);
	return _vsout_buffer.size() - 1;
}

void RenderDevice::add_point(size_t i)
{
	_point_buffer.push_back(Point{ _vsout_buffer[i] });
}

void RenderDevice::add_line(size_t i, size_t j)
{
	_line_buffer.push_back(Line{ _vsout_buffer[i], _vsout_buffer[j] });
}

void RenderDevice::add_triangle(size_t i, size_t j, size_t k)
{
	_triangle_buffer.push_back(Triangle{ _vsout_buffer[i], _vsout_buffer[j], _vsout_buffer[k] });
}

void RenderDevice::draw_point(const VSOut& v)
{
	int sx = std::ceil(v.position.x - _render_states.point_size * 0.5f);
	int tx = std::floor(v.position.x + _render_states.point_size * 0.5f);
	int sy = std::ceil(v.position.y - _render_states.point_size * 0.5f);
	int ty = std::floor(v.position.y + _render_states.point_size * 0.5f);

	for (int x = sx; x <= tx; x++)
		for (int y = sy; y <= ty; y++)
		{
			if (_render_states.point_style == PointStyle::CIRCLE)
			{
				float dx = x + 0.5f - v.position.x;
				float dy = y + 0.5f - v.position.y;
				if (dx * dx + dy * dy > _render_states.point_size * _render_states.point_size * 0.25)
					continue;
			}

			_fsin_buffer.push_back(FSIn(v, _shader_program->varying_num));

			Fragment fragment;
			fragment.x = x;
			fragment.y = y;
			fragment.depth = v.position.z;
			fragment.inv_w = v.position.w;
			_fragment_buffer.push_back(fragment);
		}
}

void RenderDevice::draw_line(const VSOut& vs, const VSOut& vt)
{
	int sx = floor(vs.position.x);
	int tx = floor(vt.position.x);
	int sy = floor(vs.position.y);
	int ty = floor(vt.position.y);

	bool steep = abs(ty - sy) > abs(tx - sx);
	if (steep)
		std::swap(sx, sy), std::swap(tx, ty);
	bool reverse = sx > tx;
	if (reverse)
		std::swap(sx, tx), std::swap(sy, ty);

	int dx = tx - sx;
	int dy = abs(ty - sy);
	int error = dx / 2;
	int stepy = ty < sy ? -1 : 1;

	int y = sy;
	for (int x = sx; x <= tx; x++)
	{
		float t = float(x - sx) / (tx - sx);
		VSOut v = interpolation_vsout(vs, vt, reverse ? 1.0f - t : t);

		_fsin_buffer.push_back(FSIn(v, _shader_program->varying_num));

		Fragment fragment;
		fragment.x = x;
		fragment.y = y;
		if (steep) std::swap(fragment.x, fragment.y);
		fragment.depth = v.position.z;
		fragment.inv_w = v.position.w;
		_fragment_buffer.push_back(fragment);

		error -= dy;
		if (error < 0)
			y += stepy, error += dx;
	}
}

void RenderDevice::draw_triangle(const VSOut& v0, const VSOut& v1, const VSOut& v2)
{
	Vec2 p[3] = {
			Vec2(v0.position.x, v0.position.y),
			Vec2(v1.position.x, v1.position.y),
			Vec2(v2.position.x, v2.position.y) 
	};
	int sx = std::floor(std::min({ p[0].x, p[1].x, p[2].x }));
	int tx = std::floor(std::max({ p[0].x, p[1].x, p[2].x }));
	int sy = std::floor(std::min({ p[0].y, p[1].y, p[2].y }));
	int ty = std::floor(std::max({ p[0].y, p[1].y, p[2].y }));

	for (int y = sy; y <= ty; y++)
		for (int x = sx; x <= tx; x++)
		{
			Vec2 pt = Vec2(x + 0.5, y + 0.5);

			float s = area(p[0], p[1], p[2]);
			float t0 = area(pt, p[1], p[2]) / s;
			float t1 = area(pt, p[2], p[0]) / s;
			float t2 = area(pt, p[0], p[1]) / s;
			if (t0 < 0.0f || t1 < 0.0f || t2 < 0.0f)
				continue;

			VSOut v = interpolation_vsout(v0, v1, v2, t0, t1, t2);

			_fsin_buffer.push_back(FSIn(v, _shader_program->varying_num));

			Fragment fragment;
			fragment.x = x;
			fragment.y = y;
			fragment.depth = v.position.z;
			fragment.inv_w = v.position.w;
			_fragment_buffer.push_back(fragment);
		}
}

void RenderDevice::clip_triangles_by_plane(RenderDevice::ClipPlane plane)
{	
	int n = _triangle_buffer.size();
	for (int i = 0; i < n; i++)
	{
		auto& triangle = _triangle_buffer[i];
		if (triangle.culled) continue;
		auto& v = triangle.v;

		size_t incnt  = 0, in[3]  = {};
		size_t outcnt = 0, out[3] = {};
		for (int j = 0; j < 3; j++) 
			(check_in_clip_plane(v[j].position, plane) ? in[incnt++] : out[outcnt++]) = j;

		if (outcnt == 3)
		{
			triangle.culled = true;
		}
		else if (outcnt == 2)
		{
			triangle.culled = true;
			
			float t0 = get_clip_interpolation_ratio(v[in[0]].position, v[out[0]].position, plane);
			float t1 = get_clip_interpolation_ratio(v[in[0]].position, v[out[1]].position, plane);
			
			VSOut v0 = interpolation_vsout(v[in[0]], v[out[0]], t0);
			VSOut v1 = interpolation_vsout(v[in[0]], v[out[1]], t1);

			_triangle_buffer.push_back(Triangle{ _triangle_buffer[i].v[in[0]], v0, v1 });
			if (in[0] == 1)
				_triangle_buffer.back().reverse_order();
		}
		else if (outcnt == 1)
		{
			triangle.culled = true;

			float t0 = get_clip_interpolation_ratio(v[in[0]].position, v[out[0]].position, plane);
			float t1 = get_clip_interpolation_ratio(v[in[1]].position, v[out[0]].position, plane);

			VSOut v0 = interpolation_vsout(v[in[0]], v[out[0]], t0);
			VSOut v1 = interpolation_vsout(v[in[1]], v[out[0]], t1);
			
			_triangle_buffer.push_back(Triangle{ _triangle_buffer[i].v[in[0]], _triangle_buffer[i].v[in[1]], v0 });
			_triangle_buffer.push_back(Triangle{ _triangle_buffer[i].v[in[1]], v1, v0 });

			if (out[0] == 1)
			{
				_triangle_buffer[_triangle_buffer.size() - 1].reverse_order();
				_triangle_buffer[_triangle_buffer.size() - 2].reverse_order();
			}
		}
	}
}

void RenderDevice::clip_lines_by_plane(RenderDevice::ClipPlane plane)
{
	int n = _line_buffer.size();
	for (int i = 0; i < n; i++)
	{
		auto& line = _line_buffer[i];
		if (line.culled) continue;

		bool in[2];
		for (int j = 0; j < 2; j++)
			in[j] = check_in_clip_plane(line.v[j].position, plane);

		if (!in[0] && !in[1])
		{
			line.culled = true;
		}
		else if (in[0] ^ in[1])
		{
			line.culled = true;
			float t = get_clip_interpolation_ratio(line.v[0].position, line.v[1].position, plane);
			VSOut v = interpolation_vsout(line.v[0], line.v[1], t);
			_line_buffer.push_back(in[0] ? Line{ line.v[0], v } : Line{ v, line.v[1] });
		}
	}
}

VSOut RenderDevice::interpolation_vsout(const VSOut& a, const VSOut& b, float t) const
{
	VSOut c;
	c.position = lerp(a.position, b.position, t);
	for (int i = 0; i < _shader_program->varying_num; i++)
		c.out_varying[i] = lerp(a.out_varying[i], b.out_varying[i], t);
	return c;
}

VSOut RenderDevice::interpolation_vsout(const VSOut& a, const VSOut& b, const VSOut& c, float t0, float t1, float t2) const
{
	VSOut d;
	d.position = a.position * t0 + b.position * t1 + c.position * t2;
	for (int i = 0; i < _shader_program->varying_num; i++)
		d.out_varying[i] = a.out_varying[i] * t0
						 + b.out_varying[i] * t1
						 + c.out_varying[i] * t2;
	return d;
}

void RenderDevice::vsout_to_viewport(VSOut& v)
{
	for (int i = 0; i < _shader_program->varying_num; i++)
		v.out_varying[i] /= v.position.w;
	v.position.x /= v.position.w;
	v.position.y /= v.position.w;
	v.position.z /= v.position.w;
	v.position.w = 1.0f / v.position.w;
	
	v.position.x = (v.position.x + 1.0f) * _render_states.viewport.w * 0.5f;
	v.position.y = (v.position.y + 1.0f) * _render_states.viewport.h * 0.5f;
	
}

float RenderDevice::get_clip_interpolation_ratio(const Vec4& a, const Vec4& b, RenderDevice::ClipPlane plane) const
{
	switch (plane)
	{
	case RenderDevice::ClipPlane::LEFT:	
		return (a.x + a.w) / (a.x + a.w - b.x - b.w);
	case RenderDevice::ClipPlane::RIGHT:
		return (a.x - a.w) / (a.x - a.w - b.x + b.w);
	case RenderDevice::ClipPlane::BOTTOM:
		return (a.y + a.w) / (a.y + a.w - b.y - b.w);
	case RenderDevice::ClipPlane::TOP:
		return (a.y - a.w) / (a.y - a.w - b.y + b.w);
	case RenderDevice::ClipPlane::NEAR:	
		return (a.z - EPS) / (a.z - b.z);
	case RenderDevice::ClipPlane::FAR:
		return (a.z - a.w) / (a.z - a.w - b.z + b.w);
	default:
		return 0.0f;
	}
}

float RenderDevice::check_in_clip_plane(const Vec4& p, ClipPlane plane) const
{
	switch (plane)
	{
	case RenderDevice::ClipPlane::LEFT:
		return p.x >= -p.w;
	case RenderDevice::ClipPlane::RIGHT:
		return p.x <=  p.w;
	case RenderDevice::ClipPlane::BOTTOM:
		return p.y >= -p.w;
	case RenderDevice::ClipPlane::TOP:
		return p.y <=  p.w;
	case RenderDevice::ClipPlane::NEAR:
		return p.w > 0.0f ? p.z >= EPS : p.z <= EPS;
	case RenderDevice::ClipPlane::FAR:
		return p.w > 0.0f ? p.z <= p.w : p.z >= p.w;
	default:
		return false;
	}
}
