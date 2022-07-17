#ifndef RENDER_STATES_H
#define RENDER_STATES_H

enum class PrimitiveMode
{
	POINTS,
	LINES,
	LINE_STRIPE,
	LINE_LOOP,
	TRIANGLES,
	TRIANGLE_STRIPE,
	TRIANGLE_FAN,
	QUADS
};

enum class PolygonMode
{
	POINTED,
	WIREFRAME,
	FILL
};

struct Viewport
{
	int x, y;
	int w, h;
};

enum class PointStyle
{
	RECT,
	CIRCLE
};

enum class CullFaceMode
{
	NONE,
	FRONT,
	BACK
};

enum class FrontVertexOrder
{
	COUNTER_CLOCKWISE,
	CLOCKWISE
};

struct RenderStates
{
	PrimitiveMode primitive_mode = PrimitiveMode::TRIANGLES;
	PolygonMode polygon_mode = PolygonMode::FILL;
	Viewport viewport = { 0, 0, 0, 0 };
	float point_size = 1.0f;
	PointStyle point_style = PointStyle::RECT;
	bool color_mask = false;
	bool depth_test = false;
	bool alpha_test = true;
	float alpha_test_threshold = 0.5f;
	bool eary_z_test = false;
	bool depth_mask = false;
	CullFaceMode cull_face_mode = CullFaceMode::NONE;
	FrontVertexOrder front_vertex_order = FrontVertexOrder::COUNTER_CLOCKWISE;
};

#endif