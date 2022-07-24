#include <iostream>
#include <chrono>
#include <random>
#include <Windows.h>
#include <fstream>
#include "renderdevice.h"
#include "renderppm.h"
#include "renderwindow.h"
#include "utils.h"
#include "phong.h"
#include "unlit.h"
#include "texture.h"
#include "model.h"
#include "profiler.h"

static const int WIN_W = 500;
static const int WIN_H = 500;
static const int fps_limit = 1000000;
static const double min_frame_time = 1.0 / fps_limit;

int main()
{
	Profiler::set_mode(Profiler::Mode::SUM);

	auto device = std::make_shared<RenderDevice>();
	auto window = std::make_shared<RenderWindow>();
		
	if (!window->open(WIN_W, WIN_H, "Software Renderer"))
	{
		std::cerr << "fuck" << std::endl;
		return -1;
	}

	static bool lbtndown = false;
	static bool rbtndown = false;
	static int last_x = 0;
	static int last_y = 0;
	static int cur_x = 0;
	static int cur_y = 0;
	static Mat4 camera_rotate_mat = trans::identity();
	static float camera_dist = 5.0f;
	static bool keydown[512];
	static float deltatime = 0.0f;

	{
		std::ifstream in("profile.json");
		if (in)
		{
			in >> camera_dist;
			for (int i = 0; i < 4; i++)
				for (int j = 0; j < 4; j++)
					in >> camera_rotate_mat[i][j];
			in.close();
		}
	}
	
	static auto rotate_x = [](float r) { camera_rotate_mat *= trans::rotate(r, Vec3(0.0, 1.0, 0.0));  };
	static auto rotate_y = [](float r) { camera_rotate_mat *= trans::rotate(r, Vec3(1.0, 0.0, 0.0));  };
	window->scroll_callback       = [](RenderWindow*, float offset) {
		camera_dist = clamp(camera_dist - offset * sqrt(camera_dist) * 0.1f, 0.001f, 1000.0f);
	};
	window->cursor_pos_callback   = [](RenderWindow*, int x, int y) {
		cur_x = x;
		cur_y = y;
		if (lbtndown)
		{
			float xrot = (x - last_x) * 0.01f;
			float yrot = (y - last_y) * 0.01f;
			rotate_x(xrot);
			rotate_y(yrot);
			last_x = x;
			last_y = y;
		}
	};
	window->mouse_button_callback = [](RenderWindow*, MouseButton btn, ButtonState state) {
		if (btn == MouseButton::L)
		{
			if (lbtndown = state == ButtonState::DOWN)
			{
				last_x = cur_x;
				last_y = cur_y;
			}
		}
		if (btn == MouseButton::R)
			rbtndown = state == ButtonState::DOWN;
	};
	window->key_callback          = [](RenderWindow* window, int key, KeyState state) {
		if (state == KeyState::DOWN)
		{
			keydown[key] = true;
			if (key == VK_ESCAPE)
				window->close();
			if (key == 'O')
				camera_rotate_mat = trans::identity(), camera_dist = 5.0;
			if (key == 'C')
				Profiler::clear();
		}
		if (state == KeyState::UP)
		{
			keydown[key] = false;
		}
	};
	window->update_callback       = [](RenderWindow* window) {
		if (keydown['W']) rotate_y(-0.5f * deltatime);
		if (keydown['S']) rotate_y( 0.5f * deltatime);
		if (keydown['A']) rotate_x(-0.5f * deltatime);
		if (keydown['D']) rotate_x( 0.5f * deltatime);
	};

	device->render_states().viewport = { 0, 0, WIN_W, WIN_H };
	device->render_states().polygon_mode = PolygonMode::FILL;
	device->render_states().depth_test = true;
	device->render_states().eary_z_test = true;
	device->render_states().alpha_test = false;
	device->render_states().point_size = 3.0f;
	device->render_states().point_style = PointStyle::RECT;
	device->render_states().cull_face_mode = CullFaceMode::BACK;

	device->set_shader_program(Phong::program);


	
	VertexArray cube;
	{
		Vec4 p[8];
		for(int z = 0; z < 2; z++)
			for(int y = 0; y < 2; y++)
				for (int x = 0; x < 2; x++)
				{
					p[z * 4 + y * 2 + x] = vec4(x - 0.5f, y - 0.5f, z - 0.5f);
				}
		Vec4 n[6] = {
			vec4( 1,  0,  0,  0),
			vec4(-1,  0,  0,  0),
			vec4( 0,  1,  0,  0),
			vec4( 0, -1,  0,  0),
			vec4( 0,  0,  1,  0),
			vec4( 0,  0, -1,  0),
		};
		size_t ind[6][4] = {
			{ 1, 3, 7, 5 },
			{ 0, 4, 6, 2 },
			{ 2, 6, 7, 3 },
			{ 0, 1, 5, 4 },
			{ 4, 5, 7, 6 },
			{ 0, 2, 3, 1 }
		};
		Vec4 texcoords[4] = {
			vec4(0, 1),
			vec4(1, 1),
			vec4(1, 0),
			vec4(0, 0)
		};
		for (int i = 0; i < 6; i++)
			for(int j = 0; j < 4; j++)
				cube.vertices.push_back(Vertex{ p[ind[i][j]], texcoords[j], n[i] });
	}
	

	auto grid_tex = std::make_shared<Texture>(128, 128);
	{
		for (int y = 0; y < 128; y++)
			for (int x = 0; x < 128; x++)
				grid_tex->set_color(x, y, Color4(((x / 16) ^ (y / 16)) & 1));
		grid_tex->warpMode = Texture::WarpMode::CLAMP_TO_EDGE;
		grid_tex->sampleMode = Texture::SampleMode::NEAREST;
	}

	auto test_tex = std::make_shared<Texture>("res\\tex\\test.png");
	test_tex->sampleMode = Texture::SampleMode::BILINEAR;
	device->set_shader_uniform("material.texture_diffuse0", TextureSampler(test_tex));
	
	Model model;
	model.load("res\\model\\nanosuit\\nanosuit.obj");
	model.transform *= trans::translate(-model.get_centroid_position());

	
	double st = get_time();
	double last_time = st;
	double last_print_fps_time = st;

	device->set_shader_uniform("gamma",    2.2f);
	device->set_shader_uniform("exposure", 1.0f);

	static Mat4 projection;
	projection = trans::perspective(PI * 0.25f, 1.0f, 0.1f, 500.0f);
	// projection = trans::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.5f, 50.0f);
	device->set_shader_uniform("transform.projection", projection);
	
	while (window->is_open())
	{
		deltatime = get_time() - last_time;
		last_time = get_time();
		if (get_time() - last_print_fps_time > 0.5)
		{
			window->set_title("Software Renderer FPS:" + std::to_string(int(1.0 / deltatime + 0.5)));
			last_print_fps_time = get_time();
		}

		static float t = 0.0;
		t += deltatime;

		device->set_shader_uniform("camera_pos", vec3(0.0f, 0.0f, 0.0f));
		
		static Mat4 view;
		view = trans::identity();
		view *= camera_rotate_mat;
		view *= trans::translate(0.0, 0.0, -camera_dist);
		device->set_shader_uniform("transform.view", view);

		{
			window->clear(Color4(0.08, 0.08, 0.1, 1.0));

			for (int i = 0; i < 1; i++) 
			{
				device->render_states().primitive_mode = PrimitiveMode::TRIANGLES;
				window->draw(device, model);
			}
		}

		//system("cls");
		//std::cout << Profiler::str(true) << std::endl;
		//Profiler::clear();

		window->show();
		do window->poll_events();
		while (get_time() - last_time < min_frame_time);
	}

	{
		std::ofstream out("profile.json");
		if (out)
		{
			out << camera_dist << ' ';
			for (int i = 0; i < 4; i++)
				for (int j = 0; j < 4; j++)
					out << camera_rotate_mat[i][j] << ' ';
			out.close();
		}
	}
	
}