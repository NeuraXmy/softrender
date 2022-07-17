#include "renderwindow.h"
#include <iostream>
#include <Windows.h>

static constexpr size_t WINDOW_TITLE_MAX_LEN = 128;
#ifdef UNICODE
static const wchar_t* const WINDOW_CLASS_NAME = L"Class";
static const wchar_t* const WINDOW_ENTRY_NAME = L"Entry";
#else
static const char* const WINDOW_CLASS_NAME = "Class";
static const char* const WINDOW_ENTRY_NAME = "Entry";
#endif

static bool class_registered = false;

struct RenderWindowData
{
    int width;
    int height;
    int bmp_width;
    int bmp_height;
	bool is_open = false;
    HWND handle = NULL;
    HDC memory_dc = NULL;
    unsigned char* surface_buffer;
};

static LRESULT CALLBACK process_message(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    RenderWindow* window = (RenderWindow*)GetProp(hWnd, WINDOW_ENTRY_NAME);
    if (window == NULL) {
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    else if (uMsg == WM_CLOSE) {
        if (window->window_close_callback)
            window->window_close_callback(window);
        window->close();
        return 0;
    }
    else if (uMsg == WM_KEYDOWN) {
        if (window->key_callback)
            window->key_callback(window, wParam, KeyState::DOWN);
        return 0;
    }
    else if (uMsg == WM_KEYUP) {
        if (window->key_callback)
            window->key_callback(window, wParam, KeyState::UP);
        return 0;
    }
    else if (uMsg == WM_LBUTTONDOWN) {
        if (window->mouse_button_callback)
            window->mouse_button_callback(window, MouseButton::L, ButtonState::DOWN);
        return 0;
    }
    else if (uMsg == WM_RBUTTONDOWN) {
        if (window->mouse_button_callback)
            window->mouse_button_callback(window, MouseButton::R, ButtonState::DOWN);
        return 0;
    }
    else if (uMsg == WM_LBUTTONUP) {
        if (window->mouse_button_callback)
            window->mouse_button_callback(window, MouseButton::L, ButtonState::UP);
        return 0;
    }
    else if (uMsg == WM_RBUTTONUP) {
        if (window->mouse_button_callback)
            window->mouse_button_callback(window, MouseButton::R, ButtonState::UP);
        return 0;
    }
    else if (uMsg == WM_MOUSEWHEEL) {
        float offset = GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
        if (window->scroll_callback)
            window->scroll_callback(window, offset);
        return 0;
    }
    else {
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

static bool register_class() {
    ATOM class_atom;
    WNDCLASS window_class;
    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = process_message;
    window_class.cbClsExtra = 0;
    window_class.cbWndExtra = 0;
    window_class.hInstance = GetModuleHandle(NULL);
    window_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    window_class.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    window_class.lpszMenuName = NULL;
    window_class.lpszClassName = WINDOW_CLASS_NAME;
    class_atom = RegisterClass(&window_class);
    return class_atom;
}

static void blit_buffer(unsigned char* surface, unsigned char* src, int width, int height) {
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
            surface[y * width + x] = src[y * width + x];
		}
	}
}

RenderWindow::RenderWindow()
{
	_data = std::make_unique<RenderWindowData>();
}

RenderWindow::~RenderWindow()
{
    close();
}

bool RenderWindow::open(int width, int height, std::string_view title_sv)
{
    if (!class_registered)
    {
        if (!(class_registered = register_class()))
        {
		    std::cerr << "failed to register window class: " << GetLastError() << std::endl;
			return false;
        }
    }
	
    DWORD style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    RECT rect;

#ifdef UNICODE
    wchar_t title[WINDOW_TITLE_MAX_LEN];
    mbstowcs(title, title_sv.data(), title_sv.size() + 1);
#else
    const char* title = title_;
#endif

    rect.left = 0;
    rect.top = 0;
    rect.right = width;
    rect.bottom = height;
    AdjustWindowRect(&rect, style, 0);

    _data->handle = CreateWindow(WINDOW_CLASS_NAME, title, style,
        CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
        NULL, NULL, GetModuleHandle(NULL), NULL);
	
    if (_data->handle == NULL)
    {
		std::cerr << "failed to create window: " << GetLastError() << std::endl;
        close();
        return false;
    }

    BITMAPINFOHEADER bi_header;
    HBITMAP dib_bitmap;
    HBITMAP old_bitmap;
    HDC window_dc;

    window_dc = GetDC(_data->handle);
    _data->memory_dc = CreateCompatibleDC(window_dc);
    ReleaseDC(_data->handle, window_dc);

    memset(&bi_header, 0, sizeof(BITMAPINFOHEADER));
    bi_header.biSize = sizeof(BITMAPINFOHEADER);
    bi_header.biWidth = width;
    bi_header.biHeight = height;
    bi_header.biPlanes = 1;
    bi_header.biBitCount = 24;
    bi_header.biCompression = BI_RGB;
    dib_bitmap = CreateDIBSection(_data->memory_dc, (BITMAPINFO*)&bi_header,
        DIB_RGB_COLORS, (void**)&_data->surface_buffer,
        NULL, 0);
    if (dib_bitmap == NULL)
    {
        std::cerr << "failed to create bitmap: " << GetLastError() << std::endl;
        close();
        return false;
    }
    old_bitmap = (HBITMAP)SelectObject(_data->memory_dc, dib_bitmap);
    DeleteObject(old_bitmap);

    _data->width = width;
    _data->height = height;
    _framebuffer = std::make_unique<FrameBuffer>(width, height);
    _data->width = width;
    _data->height = height;

    SetProp(_data->handle, WINDOW_ENTRY_NAME, this);
    ShowWindow(_data->handle, SW_SHOW);
	
    return _data->is_open = true;
}

void RenderWindow::set_title(std::string_view title_sv)
{
#ifdef UNICODE
    wchar_t title[WINDOW_TITLE_MAX_LEN];
    mbstowcs(title, title_sv.data(), title_sv.size() + 1);
#else
    const char* title = title_;
#endif

    SetWindowText(_data->handle, title);
}

void RenderWindow::close()
{
	if (_data->is_open)
	{
        if (_data->memory_dc)
        {
            DeleteDC(_data->memory_dc);
            _data->memory_dc = NULL;
        }
        if (_data->handle)
        {
            DestroyWindow(_data->handle);
            _data->handle = NULL;
        }
		_data->is_open = false;
	}
}

bool RenderWindow::is_open() const
{
    return _data->is_open;
}

void RenderWindow::show()
{
    int width  = _data->width;
    int height = _data->height;

    int ind = 0;
    while (ind < width * height * 3)
    {
        _data->surface_buffer[ind + 0] = _framebuffer->ldr_color_buffer_data()[ind + 2];
		_data->surface_buffer[ind + 1] = _framebuffer->ldr_color_buffer_data()[ind + 1];
		_data->surface_buffer[ind + 2] = _framebuffer->ldr_color_buffer_data()[ind + 0];
        ind += 3;
    }

    HDC window_dc = GetDC(_data->handle);
    HDC memory_dc = _data->memory_dc;
    BitBlt(window_dc, 0, 0, width, height, memory_dc, 0, 0, SRCCOPY);
    ReleaseDC(_data->handle, window_dc);
}

void RenderWindow::poll_events()
{
    if (cursor_pos_callback)
    {
        POINT pos;
        GetCursorPos(&pos);
		cursor_pos_callback(this, pos.x, pos.y);
    }

    MSG message;
    while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    if (update_callback)
        update_callback(this);
}