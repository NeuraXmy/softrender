#ifndef RENDER_WINDOW_H
#define RENDER_WINDOW_H

#include "rendertarget.h"

enum class KeyState
{
	UP, DOWN
};
enum class MouseButton
{
	L, R
};
enum class ButtonState
{
	UP, DOWN
};

class RenderWindow;

using KeyCallbackFunc		   = void(*)(RenderWindow* window, int key, KeyState state);
using MouseButtonCallbackFunc  = void(*)(RenderWindow* window, MouseButton button, ButtonState state);
using WindowCloseCallbackFunc  = void(*)(RenderWindow* window);
using CursorPosCallbackFunc    = void(*)(RenderWindow* window, int x, int y);
using ScrollCallbackFunc       = void(*)(RenderWindow* window, float offset);
using UpdateFunc			   = void(*)(RenderWindow* window);

class RenderWindowData;

class RenderWindow : public RenderTarget
{
public:

	RenderWindow();

	~RenderWindow();

	bool open(int width, int height, std::string_view title);

	void set_title(std::string_view title);

	void close();

	bool is_open() const;

	void show();

	void poll_events();
	

	KeyCallbackFunc				key_callback			= nullptr;
	MouseButtonCallbackFunc		mouse_button_callback	= nullptr;
	WindowCloseCallbackFunc		window_close_callback	= nullptr;
	CursorPosCallbackFunc		cursor_pos_callback		= nullptr;
	ScrollCallbackFunc			scroll_callback			= nullptr;
	UpdateFunc					update_callback			= nullptr;

private:

	std::unique_ptr<RenderWindowData> _data;

};

#endif 
