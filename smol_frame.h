/*
Copyright © 2023 Marko Ranta (Discord: Coderunner#2271)

This software is provided *as-is*, without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/

/*
NOTES: 

If your program doesn't need to refer frames in multiple code 
files you can just include this file into your main source file 
like this:

#define SMOL_FRAME_BACKEND_XXXX  (Optional backend, falls back to X11 also Linux only)
#define SMOL_FRAME_IMPLEMENTATION
#include "smol_frame.h"

...
int main() { ... }

If you need to include this file in multiple source files, add empty .c 
file into your project called 'smol_frame.c' for example, and add the 
code lines from above the 'int main()' code.

Also on Windows you don't have to link anything to use this library.
On linux, if you fall back to X11, you can compile code like this:
gcc main.c -lX11 -o main 

XCB is bit more cumbersome, because you need these packages:
- libxcb-devel
- libxcb-icccm4-dev
- libxcb-keysyms1-dev
If there's one unified package, let me know I'll wipe that list.
Anyway compiling with xcb backend would work like this:
gcc main.c -lxcb -lxcb-icccm -lxcb-keysyms -o main

If you're using WAYLAND you have to link those libraries
accordingly. (Wayland not implemented yet)
*/

/*
TODO: 
- Rudimentary OpenGL setup utility functions, on Windows this should be quite trivial task.
- Icons, Cursors, Mouse hiding
- Key modifiers to the events
- Some sort of generic drag / drop system for items (files at starters)
- Backends (Wayland for Linux, Mac, maybe Web and android)
- High DPI stuff
- Implement actual OpenGL context set up for newly generated window, if the window flag is defined.

Contributions: 
- Me (MaGetzUb) 
- Diego Lopes (https://github.com/DCubix)
*/


#ifndef SMOL_FRAME_H
#define SMOL_FRAME_H

#if defined(_WIN32)
#	ifndef SMOL_PLATFORM_WINDOWS
#	define SMOL_PLATFORM_WINDOWS
#	endif 
#	define WIN32_LEAN_AND_MEAN
#	include <Windows.h>
#	include <windowsx.h>
#	include <wingdi.h>
#	pragma comment(lib, "gdi32.lib")
#elif defined(__linux__) 
#	ifndef SMOL_PLATFORM_LINUX
#	define SMOL_PLATFORM_LINUX
#	endif 
#	if defined(SMOL_FRAME_BACKEND_XCB)
#		include <xcb/xcb.h>
#		include <xcb/xcb_icccm.h>
#		include <xcb/xcb_keysyms.h>
#		include <xcb/xkb.h>
#		include <X11/keysym.h>
#	elif defined(SMOL_FRAME_BACKEND_WAYLAND)
#		include <wayland-server.h>
#		include <wayland-client-core.h>
#		include <wayland-client-protocol.h> 
#	error Wayland backend not implemented yet!
#	else
#		define SMOL_FRAME_BACKEND_X11
#		include <X11/X.h>
#		include <X11/Xlib.h>
#		include <X11/keysym.h>
#		include <X11/XKBlib.h>
#		include <X11/Xutil.h>
#	endif 
#elif defined(__APPLE__)
#	ifndef SMOL_PLATFORM_MAX_OS
#	define SMOL_PLATFORM_MAX_OS
#	endif 
//TODO:
#	error Mac OS backend not implemented yet!
#endif 


//Forward declaration for frame handle
typedef struct _smol_frame_t smol_frame_t;

//Forward declaration for frame event 
typedef struct _smol_frame_event_t smol_frame_event_t;

//smol_frame_create_advanced - Creates advanced window
//Arguments:
// - int width            -- width of the window
// - int height           -- height of the window
// - const char* title    -- title of the window
// - unsigned int flags   -- behavioral hints for the window (is it resizable, does it support OpenGL etc)
// - smol_frame_t* parent -- parent window for this window
//Returns: smol_frame_t*  -- a handle to the newly created window
smol_frame_t* smol_frame_create_advanced(int width, int height, const char* title, unsigned int flags, smol_frame_t* parent);

//smol_frame_create - Creates a window
//Arguments:
// - int width            -- width of the window
// - int height           -- height of the window
// - const char* title    -- title of the window
//Returns: smol_frame_t*  -- a handle to the newly created window
smol_frame_t* smol_frame_create(int width, int height, const char* title);

//smol_frame_is_closed - Returns true if the window has been closed
//Arguments: 
// - smol_frame_t* frame -- A handle to the frame
//Returns: int -- containing 1 if closed, 0 if not
int smol_frame_is_closed(smol_frame_t* frame);

//smol_frame_destroy - Destroys a frame
//Arguments: 
// - smol_frame_t* frame -- A handle to the frame to be destroyed
void smol_frame_destroy(smol_frame_t* frame);

//smol_frame_acquire_event - Acquire an event from a frame's event queue
//Arguments: 
// - smol_frame_t* frame       -- A handle to the frame whose events we're inspecting
// - smol_frame_event_t* event -- A pointer to event structure
//Returns: int -- containing the number of events left in the queue 
int smol_frame_acquire_event(smol_frame_t* frame, smol_frame_event_t* event);

//smol_frame_update - Polls all events for a window.
//Arguments: 
// - smol_frame_t* frame -- A window that's events are being polled
void smol_frame_update(smol_frame_t* frame);

//smol_frame_set_title - Set a window top bar title
//Arguments:
// - smol_frame_t* frame -- A window that's title is being changed
// - const char* title	 -- The window title
void smol_frame_set_title(smol_frame_t* frame, const char* title);

//smol_frame_blit_pixels - Blits pixels to frame 
//Arguments: 
// - smol_frame_t* frame  -- A window that's being drawn to
// - unsigned int* pixBuf -- A pointer to pixel buffer, pixels are in order of XXRRGGBB, at least on windows.
// - int pixBufWidth      -- Width of the pixel buffer
// - int pixBufHeight     -- Height of the pixel buffer
// - int dstX             -- Copy destination location on x-axis
// - int dstY             -- Copy destination location on y-axis
// - int dstW             -- Destination width
// - int dstH             -- Destination height
// - int srcX             -- Source location on x-axis 
// - int srcX             -- Source location on y-axis
// - int srcW             -- Source width
// - int srcH             -- Source height
void smol_frame_blit_pixels(smol_frame_t* frame, unsigned int* pixBuf, int pixBufWidth, int pixBufHeight, int dstX, int dstY, int dstW, int dstH, int srcX, int srcY, int srcW, int srcH);

#if defined(SMOL_PLATFORM_WINDOWS)

//smol_frame_get_win32_window_handle - Gets the HWND window handle on Windows 
//Arguments:
// - smol_frame_t* frame -- A frame we want the HWND handle from
//Returns: HWND - the window handle 
HWND smol_frame_get_win32_window_handle(smol_frame_t* frame);

//smol_frame_get_win32_window_handle - Gets the Window's module handle on Windows 
//Arguments:
// - smol_frame_t* frame -- A frame we want the module handle from
//Returns: HINSTANCE - the module handle 
HINSTANCE smol_frame_get_win32_module_handle(smol_frame_t* frame);

#elif defined(SMOL_PLATFORM_LINUX)
#	if defined(SMOL_FRAME_BACKEND_X11) 

//smol_frame_get_x11_display - Gets the X11 Display server connection from a frame 
//Arguments:
// - smol_frame_t* frame -- A frame we want the Display server connection from
//Returns: Display* - the display server connection 
Display* smol_frame_get_x11_display(smol_frame_t* frame);

//smol_frame_get_x11_window - Gets the X11 Window handle from a frame 
//Arguments:
// - smol_frame_t* frame -- A frame we want the X11 Window handle from
//Returns: Window - the window handle 
Window smol_frame_get_x11_window(smol_frame_t* frame);

#	elif defined(SMOL_FRAME_BACKEND_XCB) 

//smol_frame_get_x11_display - Gets the Xcb Display server connection from a frame 
//Arguments:
// - smol_frame_t* frame -- A frame we want the Display server connection from
//Returns: xcb_connection_t* - the display server connection 
xcb_connection_t* smol_frame_get_xcb_connection(smol_frame_t* frame);

//smol_frame_get_xcb_window - Gets the xcb Window handle from a frame 
//Arguments:
// - smol_frame_t* frame -- A frame we want the X11 Window handle from
//Returns: Window - the window handle 
xcb_window_t smol_frame_get_xcb_window(smol_frame_t* frame);

#	elif defined(SMOL_FRAME_BACKEND_WAYLAND)
//TODO: 
#endif 
#elif defined(SMOL_PLATFORM_MAC_OS)
//TODO:
#endif 

//Poor man's lazy memory management 
#ifndef SMOL_ALLOC
#define SMOL_ALLOC( size ) malloc(size)
#endif 

#ifndef SMOL_FREE
#define SMOL_FREE( ptr ) free(ptr)
#endif 

#ifndef SMOL_REALLOC
#define SMOL_REALLOC( old_ptr, new_size ) realloc(old_ptr, new_size)
#endif 

#define SMOL_ALLOC_INSTANCE( type ) (type*)SMOL_ALLOC(sizeof(type))
#define SMOL_ALLOC_ARRAY( type, count ) (type*)SMOL_ALLOC(sizeof(type)*count)

#endif 


//Window configuration flags, passed to flags argument of 'smol_frame_create_advanced' function
typedef enum {
	SMOL_FRAME_CONFIG_HAS_TITLEBAR        = 0x00000001U,
	SMOL_FRAME_CONFIG_HAS_MAXIMIZE_BUTTON = 0x00000002U,
	SMOL_FRAME_CONFIG_IS_RESIZABLE        = 0x00000004U,
	SMOL_FRAME_CONFIG_SUPPORT_OPENGL	  = 0x00000008U,
	SMOL_FRAME_DEFAULT_CONFIG      		  = (
		SMOL_FRAME_CONFIG_HAS_TITLEBAR
	)
} smol_frame_config;

//Keyboard event
typedef struct {
	unsigned int code;
} smol_frame_key_event;

//Mouse event
typedef struct {
	int  x,  y; //Position 
	int  z,  w; //Wheel orientation (accumulated in a frame)
	int dx, dy; //Position delta
	int dz, dw; //Wheel orientation delta
	int button; //Mouse button 
} smol_frame_mouse_event;

//Resize event
typedef struct {
	int width;
	int height;
} smol_frame_resize_event;

//All the event types
typedef enum {
	SMOL_FRAME_EVENT_CLOSED = 1,
	SMOL_FRAME_EVENT_RESIZE,
	SMOL_FRAME_EVENT_KEY_DOWN,
	SMOL_FRAME_EVENT_KEY_UP,
	SMOL_FRAME_EVENT_MOUSE_MOVE,
	SMOL_FRAME_EVENT_MOUSE_BUTTON_DOWN,
	SMOL_FRAME_EVENT_MOUSE_BUTTON_UP,
	SMOL_FRAME_EVENT_MOUSE_VER_WHEEL,
	SMOL_FRAME_EVENT_MOUSE_HOR_WHEEL,
	SMOL_FRAME_EVENT_FOCUS_LOST,
	SMOL_FRAME_EVENT_FOCUS_GAINED,
	SMOL_FRAME_EVENT_TEXT_INPUT
} smol_frame_event_type;

//Most rudimentary event structure
typedef struct _smol_frame_event_t {
	smol_frame_event_type type;
	union {
		smol_frame_key_event key;
		smol_frame_mouse_event mouse;
		smol_frame_resize_event size;
		unsigned int unicode;
	};
} smol_frame_event_t;

//All the key code
typedef enum {
	SMOLK_UNKNOWN = 0xFFFFFFFFU,
	SMOLK_A = 0, SMOLK_B, SMOLK_C, SMOLK_D, SMOLK_E, SMOLK_F, SMOLK_G, SMOLK_H,
	SMOLK_I, SMOLK_J, SMOLK_K, SMOLK_L, SMOLK_M, SMOLK_N, SMOLK_O, SMOLK_P, SMOLK_Q,
	SMOLK_R, SMOLK_S, SMOLK_T, SMOLK_U, SMOLK_V, SMOLK_W, SMOLK_X, SMOLK_Y, SMOLK_Z,
	SMOLK_NUM0, SMOLK_NUM1, SMOLK_NUM2, SMOLK_NUM3, SMOLK_NUM4,
	SMOLK_NUM5, SMOLK_NUM6, SMOLK_NUM7, SMOLK_NUM8, SMOLK_NUM9,
	SMOLK_ESC, SMOLK_LCONTROL, SMOLK_LSHIFT,
	SMOLK_LALT, SMOLK_LSYSTEM, SMOLK_RCONTROL, SMOLK_RSHIFT,
	SMOLK_RALT, SMOLK_RSYSTEM, SMOLK_MENU,
	SMOLK_LBRACKET, SMOLK_RBRACKET,
	SMOLK_SEMICOLON, SMOLK_COMMA, SMOLK_PERIOD,
	SMOLK_QUOTE, SMOLK_SLASH, SMOLK_BACKSLASH, SMOLK_TILDE,
	SMOLK_EQUAL, SMOLK_DASH, SMOLK_SPACE, SMOLK_RETURN, SMOLK_BACKSPACE,
	SMOLK_TAB, SMOLK_PAGEUP, SMOLK_PAGEDOWN, SMOLK_END, SMOLK_HOME,
	SMOLK_INSERT, SMOLK_DELETE, SMOLK_ADD, SMOLK_SUBSTRACT, SMOLK_MULTIPLY,
	SMOLK_DIVIDE, SMOLK_LEFT, SMOLK_RIGHT, SMOLK_UP, SMOLK_DOWN,
	SMOLK_NUMPAD0, SMOLK_NUMPAD1, SMOLK_NUMPAD2, SMOLK_NUMPAD3,
	SMOLK_NUMPAD4, SMOLK_NUMPAD5, SMOLK_NUMPAD6, SMOLK_NUMPAD7,
	SMOLK_NUMPAD8, SMOLK_NUMPAD9, SMOLK_ENTER, SMOLK_DEL,
	SMOLK_CAPSLOCK, SMOLK_NUMLOCK, SMOLK_PRINTSCREEN, SMOLK_SCROLLLOCK,
	SMOLK_F1, SMOLK_F2, SMOLK_F3, SMOLK_F4, SMOLK_F5, SMOLK_F6, SMOLK_F7, SMOLK_F8, 
	SMOLK_F9, SMOLK_F10, SMOLK_F11, SMOLK_F12, SMOLK_F13, SMOLK_F14, SMOLK_F15,
	SMOLK_PAUSE,
	SMOLK_PLUS, SMOLK_MINUS, SMOLK_ASTERISK,
	SMOLK_KEYCOUNT
} smol_key;


#ifdef SMOL_FRAME_IMPLEMENTATION

#ifdef _MSC_VER
#	ifndef SMOL_BREAKPOINT
#		define SMOL_BREAKPOINT __debugbreak //MSVC uses this
#	endif 
#else 
#	ifndef SMOL_BREAKPOINT
#		define SMOL_BREAKPOINT __builtin_trap //Clang and GCC uses this, AFAIK
#	endif
#endif 

#ifndef SMOL_ASSERT
#define SMOL_ASSERT(condition) \
	if(!(condition)) \
		printf(\
			"SMOL FRAME ASSERTION FAILED!\n" \
			#condition "\n" \
			"IN FILE '" __FILE__ "'\n" \
			"ON LINE %d", __LINE__ \
		), \
		SMOL_BREAKPOINT()
#endif 

typedef struct {
	smol_frame_event_t* events;
	int event_count;
	int front_index;
	int back_index;
} smol_event_queue_t;

#if !defined(SMOL_PLATFORM_WINDOWS)
//This is kind of analog to Window's DC (device context)
typedef struct {
#if	 defined(SMOL_FRAME_BACKEND_XCB)
	xcb_gcontext_t gc;
#elif defined(SMOL_FRAME_BACKEND_X11)
	GC gc;
	XImage* image;
#elif defined(SMOL_FRAME_BACKEND_WAYLAND)
	/* TODO */
#endif 
	unsigned int* pixel_data;
	int width;
	int height;
} smol_software_renderer_t;
#endif 

typedef struct _smol_frame_t {
#if defined(SMOL_PLATFORM_WINDOWS)
	HWND frame_handle_win32;
	HMODULE module_handle_win32;
	unsigned short high_utf16_surrogate;
#elif defined(SMOL_PLATFORM_LINUX) 
#	if defined(SMOL_FRAME_BACKEND_X11)
	Display* display_server_connection;
	Window frame_window;
	XIM im;
	XIC ic;
#	elif defined(SMOL_FRAME_BACKEND_XCB)
	xcb_connection_t* display_server_connection;
	xcb_screen_t* screen;
	xcb_window_t frame_window;
#	endif
	smol_software_renderer_t* renderer;
#endif 
	int width;
	int height;
	
	int should_close;
	
	int old_mouse_x;
	int old_mouse_y;
	
	int mouse_z_accum;
	int mouse_w_accum;

	smol_event_queue_t event_queue;
} smol_frame_t;


#pragma region Event queue implementation

//TODO: sanity check this code that it doesn't have logical flaws, so far it has worked fine. ~MaGetzUb

smol_event_queue_t smol_event_queue_create(int numEvents) {

	smol_event_queue_t result = { 0 };
	result.events = SMOL_ALLOC_ARRAY(smol_frame_event_t, numEvents);
	result.event_count = numEvents;
	result.back_index = 0;
	result.front_index = 0;

	return result;
}

void smol_event_queue_destroy(smol_event_queue_t* queue) {
	queue->back_index = 0;
	queue->front_index = 0;
	queue->event_count = 0;
	SMOL_FREE(queue->events);
}

int smol_event_queue_push_back(smol_event_queue_t* queue, const smol_frame_event_t* event) {

	if(queue->back_index < queue->front_index)
		return 0;

	int index = queue->back_index++;
	index = index % queue->event_count;
	queue->events[index] = *event;

	return 1;
}

int smol_event_queue_push_front(smol_event_queue_t* queue, const smol_frame_event_t* event) {

	if(queue->back_index < queue->front_index)
		return 0;

	int index = queue->front_index--;
	index = (queue->event_count + index) % queue->event_count;
	queue->events[index] = *event;

	return 1;
}

int smol_event_queue_count(smol_event_queue_t* queue) {
	return queue->back_index - queue->front_index;
}

int smol_event_queue_pop_front(smol_event_queue_t* queue, smol_frame_event_t* event) {
	
	if(queue->back_index < queue->front_index)
		return 0;

	int index = queue->front_index++;
	index = index % queue->event_count;
	*event = queue->events[index];

	if(queue->front_index == queue->back_index) {
		queue->front_index = queue->back_index = 0;
	}

	return 1;
}

int smol_event_queue_pop_back(smol_event_queue_t* queue, smol_frame_event_t* event) {
	
	if(queue->back_index < queue->front_index)
		return 0;

	int index = queue->back_index--;
	index = (queue->event_count + index) % queue->event_count;
	*event = queue->events[index];

	if(queue->front_index == queue->back_index) {
		queue->front_index = queue->back_index = 0;
	}

	return 1;
}

#pragma endregion 

#pragma region Platform agnostic functions

smol_frame_t* smol_frame_create(int width, int height, const char* title) {
	return smol_frame_create_advanced(width, height, title, SMOL_FRAME_DEFAULT_CONFIG, NULL);
}

int smol_frame_acquire_event(smol_frame_t* frame, smol_frame_event_t* event) {

	int nEvents = smol_event_queue_count(&frame->event_queue);

	if(nEvents <= 0)
		return 0;
	
	if(smol_event_queue_pop_front(&frame->event_queue, event))
		return nEvents;
	
	return 0;
}

int smol_frame_is_closed(smol_frame_t* frame) {
	return frame->should_close;
}

#pragma endregion 

#pragma region Win32 Implementation
#if defined(SMOL_PLATFORM_WINDOWS)
WNDCLASSEXW wndClass;

LRESULT CALLBACK smol_frame_handle_event(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);

smol_frame_t* smol_frame_create_advanced(int width, int height, const char* title, unsigned int flags, smol_frame_t* parent) {

	RECT winRect = { 0 };
	HWND wnd = NULL;
	POINT point = { 0 };
	DWORD exStyle = WS_VISIBLE | WS_SYSMENU | WS_MINIMIZEBOX;
	smol_frame_t* result = NULL;


	if(wndClass.cbSize == 0) {

		wndClass.cbSize = sizeof(wndClass);
		wndClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
		wndClass.lpfnWndProc = &smol_frame_handle_event;
		wndClass.cbClsExtra = NULL;
		wndClass.cbWndExtra = NULL;
		wndClass.hInstance = GetModuleHandle(0);
		wndClass.hIcon = LoadIcon(NULL, IDI_WINLOGO);
		wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wndClass.lpszMenuName = NULL;
		wndClass.lpszClassName = L"smol_frame";
		wndClass.hIconSm = NULL;
		
		ATOM registerResult = RegisterClassExW(&wndClass);
		
		SMOL_ASSERT("Window class initialization failed!" && registerResult);

		if(registerResult == 0) {
			return NULL;
		}

	}

	if(GetCursorPos(&point) == TRUE) {
		HMONITOR monitor = MonitorFromPoint(point, MONITOR_DEFAULTTONEAREST);
		MONITORINFO monitorInfo = { sizeof(monitorInfo) };
		if(GetMonitorInfoW(monitor, &monitorInfo) != FALSE) {
			winRect.left = ((monitorInfo.rcMonitor.left + monitorInfo.rcMonitor.right) - width) >> 1;
			winRect.top = ((monitorInfo.rcMonitor.top + monitorInfo.rcMonitor.bottom) - height) >> 1;
			winRect.right = winRect.left + width;
			winRect.bottom = winRect.top + height;
		}
	}

	if(flags & SMOL_FRAME_CONFIG_HAS_TITLEBAR) exStyle |= WS_CAPTION;
	if(flags & SMOL_FRAME_CONFIG_HAS_MAXIMIZE_BUTTON) exStyle |= WS_MAXIMIZEBOX;
	if(flags & SMOL_FRAME_CONFIG_IS_RESIZABLE) exStyle |= WS_SIZEBOX;

	AdjustWindowRect(&winRect, exStyle, FALSE);

	
	result = SMOL_ALLOC_INSTANCE(smol_frame_t);
	SMOL_ASSERT("Unable to allocate result!" && result);
	memset(result, 0, sizeof(smol_frame_t));

	wchar_t wide_title[256] = { 0 };
	MultiByteToWideChar(CP_UTF8, MB_COMPOSITE, title, strlen(title), wide_title, 1024);

	wnd = CreateWindowExW(
		0, 
		wndClass.lpszClassName, 
		wide_title, 
		exStyle, 
		winRect.left, 
		winRect.top, 
		winRect.right - winRect.left, 
		winRect.bottom - winRect.top, 
		parent ? (parent)->frame_handle_win32 : NULL, 
		NULL, 
		wndClass.hInstance,
		result
	);

	if(IsWindow(wnd) == TRUE) {

		result->frame_handle_win32 = wnd;
		result->module_handle_win32 = wndClass.hInstance;
		result->event_queue = smol_event_queue_create(2048);

	} else {

		SMOL_ASSERT(!"Window creation failed!");
		SMOL_FREE(result);
		result = NULL;

	}

	return result;


}

void smol_frame_set_title(smol_frame_t* frame, const char* title) {
	wchar_t wide_title[256] = { 0 };
	MultiByteToWideChar(CP_UTF8, MB_COMPOSITE, title, strlen(title), wide_title, 256);	
	SMOL_ASSERT("Failed to set window title!" && SetWindowTextW(frame->frame_handle_win32, wide_title));
}

HWND smol_frame_get_win32_window_handle(smol_frame_t* frame) {
	return frame->frame_handle_win32;
}

HINSTANCE smol_frame_get_win32_module_handle(smol_frame_t* frame) {
	return frame->module_handle_win32;
}

void smol_frame_destroy(smol_frame_t* frame) {
	SMOL_ASSERT(DestroyWindow(frame->frame_handle_win32));
	smol_event_queue_destroy(&frame->event_queue);
	SMOL_FREE(frame);
}

//Can be passed null, this will pump messages to every window (on windows at least)
void smol_frame_update(smol_frame_t* frame) {

	MSG msg;
	while(PeekMessageW(&msg, frame ? frame->frame_handle_win32 : NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

}

int smol_frame_mapkey(WPARAM key, LPARAM ext);

LRESULT CALLBACK smol_frame_handle_event(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	if(msg == WM_CREATE) {
		CREATESTRUCT* createStruct = (CREATESTRUCT*)lParam;
		SetWindowLongPtrW(wnd, GWLP_USERDATA, (LONG_PTR)createStruct->lpCreateParams);
		return 1;
	}

	smol_frame_t* frame = (smol_frame_t*)GetWindowLongPtrW(wnd, GWLP_USERDATA);
	if(!frame) 
		goto def_proc;
	if(frame->frame_handle_win32 == NULL) 
		goto def_proc;

	switch(msg) {
		case WM_CLOSE: {
			smol_frame_event_t event = { 0 };
			event.type = SMOL_FRAME_EVENT_CLOSED;
			smol_event_queue_push_back(&frame->event_queue, &event);
			frame->should_close = 1;
			return 1;
		} break;
		case WM_SIZE: {
			smol_frame_event_t event = { 0 };
			event.type = SMOL_FRAME_EVENT_RESIZE;
			event.size.width = LOWORD(lParam);
			event.size.height = HIWORD(lParam);
			frame->width = LOWORD(lParam);
			frame->height = HIWORD(lParam);
			smol_event_queue_push_back(&frame->event_queue, &event);
			return 1;
		} break;
		case WM_MOUSEMOVE: {

			smol_frame_event_t event = { 0 };

			event.type = SMOL_FRAME_EVENT_MOUSE_MOVE;
			event.mouse.x = GET_X_LPARAM(lParam);
			event.mouse.y = GET_Y_LPARAM(lParam);
			event.mouse.dx = event.mouse.x - frame->old_mouse_x;
			event.mouse.dy = event.mouse.y - frame->old_mouse_y;
			smol_event_queue_push_back(&frame->event_queue, &event);

			frame->old_mouse_x = event.mouse.x;
			frame->old_mouse_y = event.mouse.y;

			return 1;
		} break;
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_XBUTTONDOWN:
		{
			smol_frame_event_t event = { 0 };
			event.type = SMOL_FRAME_EVENT_MOUSE_BUTTON_DOWN;
			
			switch(msg) {
				case WM_LBUTTONDOWN: event.mouse.button = 1; break;
				case WM_RBUTTONDOWN: event.mouse.button = 2; break;
				case WM_MBUTTONDOWN: event.mouse.button = 3; break;
				case WM_XBUTTONDOWN: event.mouse.button = 4 + GET_XBUTTON_WPARAM(wParam); break;
			}

			event.mouse.x = GET_X_LPARAM(lParam);
			event.mouse.y = GET_Y_LPARAM(lParam);
			event.mouse.dx = event.mouse.x - frame->old_mouse_x;
			event.mouse.dy = event.mouse.y - frame->old_mouse_y;

			smol_event_queue_push_back(&frame->event_queue, &event);
			frame->old_mouse_x = event.mouse.x;
			frame->old_mouse_y = event.mouse.y;

			return 1;
		} break;
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_XBUTTONUP:
		{
			smol_frame_event_t event = { 0 };
			event.type = SMOL_FRAME_EVENT_MOUSE_BUTTON_UP;
			
			switch(msg) {
				case WM_LBUTTONUP: event.mouse.button = 1; break;
				case WM_RBUTTONUP: event.mouse.button = 2; break;
				case WM_MBUTTONUP: event.mouse.button = 3; break;
				case WM_XBUTTONUP: event.mouse.button = 4 + GET_XBUTTON_WPARAM(wParam); break;
			}

			event.mouse.x = GET_X_LPARAM(lParam);
			event.mouse.y = GET_Y_LPARAM(lParam);
			smol_event_queue_push_back(&frame->event_queue, &event);
			
			frame->old_mouse_x = event.mouse.x;
			frame->old_mouse_y = event.mouse.y;

			return 1;
		} break;
		case WM_MOUSEWHEEL: {
			smol_frame_event_t event = { 0 };

			event.type = SMOL_FRAME_EVENT_MOUSE_VER_WHEEL;
			event.mouse.x = GET_X_LPARAM(lParam);
			event.mouse.y = GET_Y_LPARAM(lParam);
			event.mouse.dx = event.mouse.x - frame->old_mouse_x;
			event.mouse.dy = event.mouse.y - frame->old_mouse_y;

			int delta = GET_WHEEL_DELTA_WPARAM(wParam);
			 
			event.mouse.dz = (delta > 0) ? 1 : -1;
			frame->mouse_z_accum += event.mouse.dz;
			event.mouse.z = frame->mouse_z_accum;

			smol_event_queue_push_back(&frame->event_queue, &event);

			frame->old_mouse_x = event.mouse.x;
			frame->old_mouse_y = event.mouse.y;
			return 1;
		} break;
		case WM_MOUSEHWHEEL: {
			smol_frame_event_t event = { 0 };

			event.type = SMOL_FRAME_EVENT_MOUSE_HOR_WHEEL;
			event.mouse.x = GET_X_LPARAM(lParam);
			event.mouse.y = GET_Y_LPARAM(lParam);
			event.mouse.dx = event.mouse.x - frame->old_mouse_x;
			event.mouse.dy = event.mouse.y - frame->old_mouse_y;

			int delta = GET_WHEEL_DELTA_WPARAM(wParam);
			 
			event.mouse.dw = (delta > 0) ? 1 : -1;
			frame->mouse_w_accum += event.mouse.dw;
			event.mouse.w = frame->mouse_w_accum;

			smol_event_queue_push_back(&frame->event_queue, &event);

			frame->old_mouse_x = event.mouse.x;
			frame->old_mouse_y = event.mouse.y;
			
			return 1;
		} break;
		case WM_KEYDOWN:
		case WM_KEYUP: {

			WORD flags = HIWORD(wParam);
			int released = (msg == WM_KEYUP);

			smol_frame_event_t event = { 0 };
			event.type = released ? SMOL_FRAME_EVENT_KEY_UP : SMOL_FRAME_EVENT_KEY_DOWN;
			event.key.code = smol_frame_mapkey(wParam, lParam);

			smol_event_queue_push_back(&frame->event_queue, &event);

			return 0;
		} break;
		case WM_CHAR: {

			unsigned int chr = (unsigned int)wParam;
			unsigned int surrogate = (unsigned int)(wParam >> 10);

			if(surrogate == 0xD800) {
				frame->high_utf16_surrogate = (wParam & 0x3FF);
				return 0;
			} else if(surrogate == 0xDC00) {
				chr = ((unsigned int)frame->high_utf16_surrogate  << 10) | ((unsigned int)(wParam) & 0x3FF);
				frame->high_utf16_surrogate = 0;
			}
			
			smol_frame_event_t event = { 0 };
			event.type = SMOL_FRAME_EVENT_TEXT_INPUT;
			event.unicode = chr;

			smol_event_queue_push_back(&frame->event_queue, &event);

			return 0;
		} break;
		case WM_SETFOCUS: {

			smol_frame_event_t event = { 0 };
			event.type = SMOL_FRAME_EVENT_FOCUS_GAINED;
			smol_event_queue_push_back(&frame->event_queue, &event);

			return 0;
		} break;
		case WM_KILLFOCUS: {

			smol_frame_event_t event = { 0 };
			event.type = SMOL_FRAME_EVENT_FOCUS_LOST;
			smol_event_queue_push_back(&frame->event_queue, &event);

			return 0;
		} break;
	}

def_proc:

	return DefWindowProc(wnd, msg, wParam, lParam);
}

int smol_frame_mapkey(WPARAM key, LPARAM ext) {
	switch(key) {
		case 'A': return SMOLK_A;
		case 'B': return SMOLK_B;
		case 'C': return SMOLK_C;
		case 'D': return SMOLK_D;
		case 'E': return SMOLK_E;
		case 'F': return SMOLK_F;
		case 'G': return SMOLK_G;
		case 'H': return SMOLK_H;
		case 'I': return SMOLK_I;
		case 'J': return SMOLK_J;
		case 'K': return SMOLK_K;
		case 'L': return SMOLK_L;
		case 'M': return SMOLK_M;
		case 'N': return SMOLK_N;
		case 'O': return SMOLK_O;
		case 'P': return SMOLK_P;
		case 'Q': return SMOLK_Q;
		case 'R': return SMOLK_R;
		case 'S': return SMOLK_S;
		case 'T': return SMOLK_T;
		case 'U': return SMOLK_U;
		case 'V': return SMOLK_V;
		case 'W': return SMOLK_W;
		case 'X': return SMOLK_X;
		case 'Y': return SMOLK_Y;
		case 'Z': return SMOLK_Z;
		case VK_ESCAPE: return SMOLK_ESC;
		case VK_RETURN: if((ext >> 6) & 1) return SMOLK_ENTER; else return SMOLK_RETURN;
		case VK_BACK: return SMOLK_BACKSPACE;
		case VK_SPACE: return SMOLK_SPACE;
		case VK_CONTROL: if((ext >> 6) & 1) return SMOLK_RCONTROL; else return SMOLK_LCONTROL;
		case VK_LCONTROL: return SMOLK_LCONTROL;
		case VK_RCONTROL: return SMOLK_RCONTROL;
		case VK_MENU: if((ext >> 6) & 1) return SMOLK_RALT; else return SMOLK_LALT;
		case VK_LMENU: return SMOLK_LALT;
		case VK_RMENU: return SMOLK_RALT;
		case VK_SHIFT: if((ext >> 6) & 1) return SMOLK_RSHIFT; else return SMOLK_LSHIFT;
		case VK_RSHIFT: return SMOLK_RSHIFT;
		case VK_LSHIFT: return SMOLK_LSHIFT;
		case VK_UP: return SMOLK_UP;
		case VK_DOWN: return SMOLK_DOWN;
		case VK_LEFT: return SMOLK_LEFT;
		case VK_RIGHT: return SMOLK_RIGHT;
		case VK_TAB: return SMOLK_TAB;
		case VK_CAPITAL: return SMOLK_CAPSLOCK;
		case VK_NUMLOCK: return SMOLK_NUMLOCK;
		case VK_SCROLL: return SMOLK_SCROLLLOCK;
		case VK_SNAPSHOT: return SMOLK_PRINTSCREEN;
		case VK_PAUSE: return SMOLK_PAUSE;
		case VK_INSERT: return SMOLK_INSERT;
		case VK_HOME: return SMOLK_HOME;
		case VK_PRIOR: return SMOLK_PAGEUP;
		case VK_NEXT: return SMOLK_PAGEDOWN;
		case VK_DELETE: return SMOLK_DELETE;
		case VK_END: return SMOLK_END;
		case 0X30: return SMOLK_NUM0;
		case 0X31: return SMOLK_NUM1;
		case 0X32: return SMOLK_NUM2;
		case 0X33: return SMOLK_NUM3;
		case 0X34: return SMOLK_NUM4;
		case 0X35: return SMOLK_NUM5;
		case 0X36: return SMOLK_NUM6;
		case 0X37: return SMOLK_NUM7;
		case 0X38: return SMOLK_NUM8;
		case 0X39: return SMOLK_NUM9;
		case VK_NUMPAD0: return SMOLK_NUMPAD0;
		case VK_NUMPAD1: return SMOLK_NUMPAD1;
		case VK_NUMPAD2: return SMOLK_NUMPAD2;
		case VK_NUMPAD3: return SMOLK_NUMPAD3;
		case VK_NUMPAD4: return SMOLK_NUMPAD4;
		case VK_NUMPAD5: return SMOLK_NUMPAD5;
		case VK_NUMPAD6: return SMOLK_NUMPAD6;
		case VK_NUMPAD7: return SMOLK_NUMPAD7;
		case VK_NUMPAD8: return SMOLK_NUMPAD8;
		case VK_NUMPAD9: return SMOLK_NUMPAD9;
		case VK_F1: return SMOLK_F1;
		case VK_F2: return SMOLK_F2;
		case VK_F3: return SMOLK_F3;
		case VK_F4: return SMOLK_F4;
		case VK_F5: return SMOLK_F5;
		case VK_F6: return SMOLK_F6;
		case VK_F7: return SMOLK_F7;
		case VK_F8: return SMOLK_F8;
		case VK_F9: return SMOLK_F9;
		case VK_F10:return SMOLK_F10;
		case VK_F11: return SMOLK_F11;
		case VK_F12: return SMOLK_F12;
		case VK_ADD: return SMOLK_ADD;
		case VK_SUBTRACT: return SMOLK_SUBSTRACT;
		case VK_MULTIPLY: return SMOLK_ASTERISK;
		case VK_DIVIDE: return SMOLK_DIVIDE;
		case VK_OEM_PLUS: return SMOLK_PLUS;
		case VK_OEM_MINUS: return SMOLK_MINUS;
		case VK_OEM_COMMA: return SMOLK_COMMA;
		case VK_OEM_PERIOD: return SMOLK_PERIOD;
	}
	return SMOLK_UNKNOWN;
}


void smol_frame_blit_pixels(
	smol_frame_t* frame,
	unsigned int* pixels,
	int width,
	int height,
	int dstX,
	int dstY,
	int dstW,
	int dstH,
	int srcX,
	int srcY,
	int srcW,
	int srcH
) {

	BITMAPINFO bmi = { 0 };
	bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biWidth = width;
	bmi.bmiHeader.biHeight = -height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	StretchDIBits(GetDC(frame->frame_handle_win32), dstX, dstY, dstW, dstH, srcX, srcY, srcW, srcH, pixels, &bmi, DIB_RGB_COLORS, SRCCOPY);

}

#endif 
#pragma endregion 

#pragma region Linux/X11 Implementation
#if defined(SMOL_PLATFORM_LINUX)

int smol_frame_mapkey(int key);

#	if defined(SMOL_FRAME_BACKEND_X11)

Atom smol__wm_delete_window_atom;

void smol_renderer_destroy(smol_software_renderer_t* renderer) {
	XDestroyImage(renderer->image);
	SMOL_FREE(renderer);
}

//Creates and recreates renderer
smol_software_renderer_t* smol_renderer_create(smol_frame_t* frame) {

	smol_software_renderer_t* renderer = SMOL_ALLOC_INSTANCE(smol_software_renderer_t);
	renderer->pixel_data = SMOL_ALLOC_ARRAY(unsigned int, frame->width * frame->height);
	renderer->width = frame->width;
	renderer->height = frame->height;
	renderer->image = XCreateImage(
		frame->display_server_connection,
		DefaultVisual(frame->display_server_connection, 0), 
		DefaultDepth(frame->display_server_connection, 0), 
		ZPixmap, 
		0, 
		(char*)renderer->pixel_data, 
		frame->width, 
		frame->height, 
		32, 
		0
	);

	if(frame->renderer) {
		
		renderer->gc = frame->renderer->gc;

		for(int y = 0; y < renderer->height; y++) {
			int dstY = y;
			int srcY = (frame->renderer->height * y) / renderer->height;	
			for(int x = 0; x < renderer->width; x++) 
			{
				int dstX = x;
				int srcX = (frame->renderer->width * x) / renderer->width;
				renderer->pixel_data[dstX + dstY * renderer->width] = frame->renderer->pixel_data[dstX + dstY * frame->renderer->width];
			}
		} 

		XDestroyImage(frame->renderer->image);
	

	} else {
		renderer->gc = XCreateGC(frame->display_server_connection, frame->frame_window, 0, NULL);
	}

	return renderer;
}

smol_frame_t* smol_frame_create_advanced(int width, int height, const char* title, unsigned int flags, smol_frame_t* parent) {

	//TODO: Window parenting
	
	Display* display = XOpenDisplay(NULL);
	Window parentWindow = None;
	Window result_window = None;
	XWindowAttributes attributes = { 0 };
	XSetWindowAttributes setAttributes = { 0 };
	XIM im;
	XIC ic;
    XIMStyles *styles;
    XIMStyle requested_style;
	smol_frame_t* result = NULL;
	Status status;

	SMOL_ASSERT("Failed to open dispaly server connection!" && display);

	if(display == NULL) 
		return NULL;  

	parentWindow = DefaultRootWindow(display);

	SMOL_ASSERT("Failed to acquire root window!" && parentWindow);
	if(parentWindow == None)
		return NULL;

	if(XGetWindowAttributes(display, parentWindow, &attributes) == 0)
		return NULL;
	
	setAttributes.event_mask = (
		ExposureMask        | KeyPressMask      | 
		KeyReleaseMask      | ButtonPressMask   | 
		ButtonReleaseMask   | PointerMotionMask | 
		ButtonMotionMask    | FocusChangeMask
	);

	result_window = XCreateWindow(
		display, 
		parentWindow, 
		(attributes.width + width) >> 1, 
		(attributes.height + height) >> 1, 
		width, 
		height, 
		0,
		CopyFromParent, 
		CopyFromParent,
		CopyFromParent, 
		CWEventMask, 
		&setAttributes
	);

	if(result_window == None)
		return NULL;


	if(!(flags & SMOL_FRAME_CONFIG_HAS_MAXIMIZE_BUTTON)) {

		XSizeHints* sizeHints = XAllocSizeHints();
		sizeHints->flags = PMinSize | PMaxSize;
		sizeHints->min_width = sizeHints->max_width = width;
		sizeHints->min_height = sizeHints->max_height = height;
		XSetWMNormalHints(display, result_window, sizeHints);

		setAttributes.override_redirect = True;
	}
	

	if(smol__wm_delete_window_atom == None) { 
		smol__wm_delete_window_atom = XInternAtom(display, "WM_DELETE_WINDOW", False);
	}

	status = XSetWMProtocols(display, result_window, &smol__wm_delete_window_atom, 1);

	SMOL_ASSERT("Failed to set Window protocols!" && (status != 0));

	XStoreName(display, result_window, title);
	XMapWindow(display, result_window);

	im = XOpenIM(display, NULL, NULL, NULL);
	XGetIMValues(im, XNQueryInputStyle, &styles, NULL);
	ic = XCreateIC(im, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, result_window, NULL);

 	XSetICFocus(ic);
	XFlush(display);

	result = SMOL_ALLOC_INSTANCE(smol_frame_t);
	result->display_server_connection = display;
	result->frame_window = result_window;
	result->event_queue = smol_event_queue_create(2048);
	result->renderer = NULL;
	result->width = width;
	result->height = height;
	result->ic = ic;
	result->im = im;

	if(!(flags & SMOL_FRAME_CONFIG_SUPPORT_OPENGL)) {
		result->renderer = smol_renderer_create(result);
	}

	//result->window_attributes = setAttributes;
	//result->root_window = rootWindow;


	return result;
}

void smol_frame_set_title(smol_frame_t* frame, const char* title) {
	XStoreName(frame->display_server_connection, frame->frame_window, title);
	XFlush(frame->display_server_connection);
}

void smol_frame_destroy(smol_frame_t* frame) {

	XDestroyWindow(frame->display_server_connection, frame->frame_window);
	XDestroyIC(frame->ic);
	XCloseIM(frame->im);
	XCloseDisplay(frame->display_server_connection);

	smol_event_queue_destroy(&frame->event_queue);
	if(frame->renderer) smol_renderer_destroy(frame->renderer);
	SMOL_FREE(frame);

}

//Can be passed null, this will pump messages to every window (on windows at least)
void smol_frame_update(smol_frame_t* frame) {

	int button_indices[] = {0, 1, 3, 2, 4, 5};

	XEvent xevent;
	while(XPending(frame->display_server_connection)) {
		XNextEvent(frame->display_server_connection, &xevent);
		XFilterEvent(&xevent, frame->frame_window);
		switch(xevent.type) {
			case Expose: {
				XWindowAttributes attribs;
				XGetWindowAttributes(frame->display_server_connection, frame->frame_window, &attribs);
				//mWidth = attribs.width;
				//mHeight = attribs.height;
				smol_frame_event_t event = { 0 };
				event.type = SMOL_FRAME_EVENT_RESIZE;
				event.size.width = attribs.width;
				event.size.height = attribs.height;

				if(frame->width != attribs.width && frame->height != attribs.height) {
					frame->renderer = smol_renderer_create(frame);
				} 

				frame->width = attribs.width;
				frame->height = attribs.height;

				smol_event_queue_push_back(&frame->event_queue, &event);

			} break;
			case ClientMessage: {
				//if(xevent.xclient.type == smol__wm_protocols_atom) {
					//puts("Should quit.");
				if(xevent.xclient.data.l[0] == smol__wm_delete_window_atom) {
					smol_frame_event_t event = { 0 };
					event.type = SMOL_FRAME_EVENT_CLOSED;
					frame->should_close = 1;
					smol_event_queue_push_back(&frame->event_queue, &event);
				}
				//}
				
			} break;
			case KeyPress:
			case KeyRelease: 
			{

				int physical = 1;
				if(xevent.type == KeyRelease) {
					XEvent next;
					if (XPending(frame->display_server_connection)) {
						XPeekEvent(frame->display_server_connection, &next);
						if(next.type == KeyPress && next.xkey.time == xevent.xkey.time && next.xkey.keycode == xevent.xkey.keycode) 
							physical = 0;
					}
				}

				//This break could be within the code block above.
				if(!physical)
					break;

				smol_frame_event_t event = { 0 };
				event.type = ((xevent.type == KeyRelease) && physical) ? SMOL_FRAME_EVENT_KEY_UP: SMOL_FRAME_EVENT_KEY_DOWN;

				KeySym keysym = XLookupKeysym(&xevent.xkey, 0);
				event.key.code = smol_frame_mapkey(keysym);
				smol_event_queue_push_back(&frame->event_queue, &event);

				if(event.type == SMOL_FRAME_EVENT_KEY_DOWN) {
					char buffer[6] = {0};
					Status status = 0;
					int count = Xutf8LookupString(frame->ic, &xevent.xkey, buffer, 6, &keysym, &status);
					unsigned int unicode = 0;
					switch(count) {
						case 1:
							unicode = (buffer[0] & 0x7F);
						break;
						case 2:
							unicode = (buffer[0] & 0x1F) << 6 | (buffer[1] & 0x3F);
						break;
						case 3:
							unicode = (buffer[0] & 0x0F) << 12 | (buffer[1] & 0x3F) << 6 | (buffer[2] & 0x3F);
						break;
						case 4:
							unicode = (buffer[0] & 0x07) << 18 | (buffer[1] & 0x3F) << 12 | (buffer[2] & 0x3F) << 6  | (buffer[3] & 0x3F);
						break;
						case 5:
							unicode = (buffer[0] & 0x03) << 24 | (buffer[1] & 0x3F) << 18 | (buffer[2] & 0x3F) << 12 | (buffer[3] & 0x3F) << 6  | (buffer[4] & 0x3F);
						break;
						case 6:
							unicode = (buffer[0] & 0x01) << 30 | (buffer[1] & 0x3F) << 24 | (buffer[2] & 0x3F) << 18 | (buffer[3] & 0x3F) << 12  | (buffer[4] & 0x3F) << 6 | (buffer[5] & 0x3F);
						break;
					}

					if(count) {
						event.type = SMOL_FRAME_EVENT_TEXT_INPUT;
						event.unicode = unicode;
						smol_event_queue_push_back(&frame->event_queue, &event);
					}
				}

			} 
			break;
			case MotionNotify: {
				smol_frame_event_t event = { 0 };
				event.type = SMOL_FRAME_EVENT_MOUSE_MOVE;
				event.mouse.dx = xevent.xmotion.x - frame->old_mouse_x;
				event.mouse.dy = xevent.xmotion.y - frame->old_mouse_y;

				event.mouse.x = xevent.xmotion.x;
				event.mouse.y = xevent.xmotion.y;
				smol_event_queue_push_back(&frame->event_queue, &event);
			} break;
			case ButtonPress:
			case ButtonRelease: 
			{
				smol_frame_event_t event = { 0 };
				if(xevent.xbutton.button < 4) {
					event.type = xevent.type == ButtonPress ? SMOL_FRAME_EVENT_MOUSE_BUTTON_DOWN : SMOL_FRAME_EVENT_MOUSE_BUTTON_UP;
					event.mouse.button = button_indices[xevent.xbutton.button];

					event.mouse.dx = xevent.xmotion.x - frame->old_mouse_x;
					event.mouse.dy = xevent.xmotion.y - frame->old_mouse_y;

					event.mouse.x = xevent.xmotion.x;
					event.mouse.y = xevent.xmotion.y;

					smol_event_queue_push_back(&frame->event_queue, &event);
				} else {

					if(xevent.type != ButtonPress)
						break;

					int wheel = 0;
					if(xevent.xbutton.button == 4) wheel = 1;
					if(xevent.xbutton.button == 5) wheel = -1;

					event.type = SMOL_FRAME_EVENT_MOUSE_VER_WHEEL;

					event.mouse.dx = xevent.xmotion.x - frame->old_mouse_x;
					event.mouse.dy = xevent.xmotion.y - frame->old_mouse_y;

					event.mouse.x = xevent.xmotion.x;
					event.mouse.y = xevent.xmotion.y;

					frame->mouse_w_accum+=wheel;
					event.mouse.w = frame->mouse_w_accum;
					event.mouse.dw = wheel;

					smol_event_queue_push_back(&frame->event_queue, &event);

				}
			} 
			break;
			case FocusIn: {
				smol_frame_event_t event = { 0 };
				event.type = SMOL_FRAME_EVENT_FOCUS_GAINED;
				smol_event_queue_push_back(&frame->event_queue, &event);
			} break;
			case FocusOut: {
				smol_frame_event_t event = { 0 };
				event.type = SMOL_FRAME_EVENT_FOCUS_LOST;
				smol_event_queue_push_back(&frame->event_queue, &event);
			} break;
		}
	}

}

void smol_frame_blit_pixels(
	smol_frame_t* frame, 
	unsigned int* pixels, 
	int width, 
	int height,
	int dstX, 
	int dstY, 
	int dstW, 
	int dstH,
	int srcX, 
	int srcY, 
	int srcW, 
	int srcH
) {

	smol_software_renderer_t* renderer = frame->renderer;
	SMOL_ASSERT("Frame has no renderer! Frame was probably initialized with OpenGL config!" && frame->renderer);

	//Precalculate amount how many pixels are skipped per axis, to opimize drawing
	int startX = dstX < 0 ? -dstX : 0;
	int startY = dstY < 0 ? -dstY : 0;

	//Precalculate amount of how many pixels are shaved off to optimize drawing
	int endX = (dstX + dstW) > renderer->width ? (dstX + dstW - renderer->width) : dstW; 
	int endY = (dstY + dstH) > renderer->height ? (dstY + dstH - renderer->height) : dstH;

	for(int y = startY; y < endY; y++) {
		int sY = srcY + ((srcH * y) / dstH);
		int dY = dstY + y;
		for(int x = startX; x < endX; x++) {
			int sX = srcX + ((srcW * x) / dstW);
			int dX = dstX + x;
			renderer->pixel_data[dX + dY * renderer->width] = pixels[sX + sY * frame->renderer->width];
		}
	}

	XPutImage(frame->display_server_connection, frame->frame_window, renderer->gc, renderer->image, 0, 0, 0, 0, renderer->width, renderer->height);
}

Display* smol_frame_get_x11_display(smol_frame_t* frame) {
	return (frame)->display_server_connection;
}

Window smol_frame_get_x11_window(smol_frame_t* frame) {
	return (frame)->frame_window;
}

#	endif 
#endif 
#pragma endregion

#pragma region XCB implementation
#if defined(SMOL_FRAME_BACKEND_XCB) && defined(SMOL_PLATFORM_LINUX)

//These globals are bit of nasty busines
xcb_atom_t smol__wm_delete_window_atom;
xcb_atom_t smol__wm_protocols_atom;
int smol__num_frames;
xcb_key_symbols_t* smol__keysyms;

void smol_renderer_destroy(smol_software_renderer_t* renderer) {
	SMOL_FREE(renderer->pixel_data);
	SMOL_FREE(renderer);
}

//Creates and recreates renderer
smol_software_renderer_t* smol_renderer_create(smol_frame_t* frame) {

	smol_software_renderer_t* renderer = SMOL_ALLOC_INSTANCE(smol_software_renderer_t);
	renderer->pixel_data = SMOL_ALLOC_ARRAY(unsigned int, frame->width * frame->height);
	renderer->width = frame->width;
	renderer->height = frame->height;


	if(frame->renderer) {
	
		//xcb_free_pixmap(frame->display_server_connection, renderer->pixmap);
		renderer->gc = frame->renderer->gc;

		for(int y = 0; y < renderer->height; y++) {
			int dstY = y;
			int srcY = (frame->renderer->height * y) / renderer->height;	
			for(int x = 0; x < renderer->width; x++) 
			{
				int dstX = x;
				int srcX = (frame->renderer->width * x) / renderer->width;
				renderer->pixel_data[dstX + dstY * renderer->width] = frame->renderer->pixel_data[srcX + srcY * frame->renderer->width];
			}
		} 

		SMOL_FREE(frame->renderer->pixel_data);

	} else {
		renderer->gc = xcb_generate_id(frame->display_server_connection);
		xcb_create_gc(frame->display_server_connection, renderer->gc, frame->frame_window, 0, NULL);
	}

	return renderer;
}

smol_frame_t* smol_frame_create_advanced(int width, int height, const char* title, unsigned int flags, smol_frame_t* parent) {

	xcb_connection_t* connection = xcb_connect(NULL, NULL);

	int error = xcb_connection_has_error(connection);
	SMOL_ASSERT("Couldn't form a connection to X-server!" && (error == 0));
	if(error) {
		return NULL;
	}

	const xcb_setup_t* setup = xcb_get_setup(connection);
	xcb_screen_t* screen = xcb_setup_roots_iterator(setup).data;

	xcb_window_t window = xcb_generate_id(connection);

	uint32_t event_mask = (
		XCB_EVENT_MASK_EXPOSURE | 
		XCB_EVENT_MASK_KEY_PRESS |
		XCB_EVENT_MASK_KEY_RELEASE |
		XCB_EVENT_MASK_BUTTON_PRESS |
		XCB_EVENT_MASK_BUTTON_RELEASE |
		XCB_EVENT_MASK_POINTER_MOTION |
		XCB_EVENT_MASK_FOCUS_CHANGE | 
		XCB_EVENT_MASK_STRUCTURE_NOTIFY
	);

	xcb_create_window(
		connection, 
		XCB_COPY_FROM_PARENT, 
		window, 
		screen->root, 
		(screen->width_in_pixels + width) >> 1, 
		(screen->height_in_pixels + height) >> 1, 
		width, 
		height, 
		0, 
		XCB_WINDOW_CLASS_INPUT_OUTPUT, 
		screen->root_visual, 
		XCB_CW_EVENT_MASK, 
		&event_mask
	);


	if(!(flags & SMOL_FRAME_CONFIG_HAS_MAXIMIZE_BUTTON)) {
		xcb_size_hints_t size_hints = {0};
		xcb_icccm_size_hints_set_min_size(&size_hints, width, height);
		xcb_icccm_size_hints_set_max_size(&size_hints, width, height);

		xcb_void_cookie_t cookie = xcb_change_property(
			connection,                 
			XCB_PROP_MODE_REPLACE,      
			window,                     
			XCB_ATOM_WM_NORMAL_HINTS,   
			XCB_ATOM_WM_SIZE_HINTS,     
			32,                         
			1,     						
			&size_hints                 
		);

	}

	if(smol__wm_delete_window_atom == 0 && smol__wm_protocols_atom == 0) {

		//Window protocols atom acquisition 
		xcb_intern_atom_cookie_t protocolsCookie = xcb_intern_atom(
			connection, 0, strlen("WM_PROTOCOLS"), "WM_PROTOCOLS"
		);
		xcb_intern_atom_reply_t* protocolsReply = xcb_intern_atom_reply(
			connection, protocolsCookie, NULL
		);
		smol__wm_protocols_atom = protocolsReply->atom;
		free(protocolsReply);
		
		//Delete window atom acquisition
		xcb_intern_atom_cookie_t deleteWindowCookie = xcb_intern_atom(
			connection, 0, strlen("WM_DELETE_WINDOW"), "WM_DELETE_WINDOW"
		);
		xcb_intern_atom_reply_t* deleteWindowReply = xcb_intern_atom_reply(
			connection, deleteWindowCookie, NULL
		);
		
		
		smol__wm_delete_window_atom = deleteWindowReply->atom;
		
		xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, smol__wm_protocols_atom, 4, 32, 1, &smol__wm_delete_window_atom);
		free(deleteWindowReply);
	}

	xcb_change_property(
		connection, 
		XCB_PROP_MODE_REPLACE, 
		window, 
		XCB_ATOM_WM_NAME, 
		XCB_ATOM_STRING, 
		8, 
		strlen(title), 
		(const void*)title
	);

	xcb_map_window(connection, window);
	xcb_flush(connection);


	smol_frame_t* result = SMOL_ALLOC_INSTANCE(smol_frame_t);
	memset(result, 0, sizeof(smol_frame_t));
	result->display_server_connection = connection;
	result->event_queue = smol_event_queue_create(2048);
	result->width = width;
	result->height = height;
	result->screen = screen;
	result->frame_window = window;
	result->renderer = smol_renderer_create(result);

	if(smol__num_frames == 0) {
		smol__keysyms = xcb_key_symbols_alloc(connection);
	}
	smol__num_frames++;

	return result;

}

void smol_frame_set_title(smol_frame_t* frame, const char* title) {
	xcb_change_property(
		frame->display_server_connection, 
		XCB_PROP_MODE_REPLACE, 
		frame->frame_window, 
		XCB_ATOM_WM_NAME, 
		XCB_ATOM_STRING, 
		8, 
		strlen(title), 
		(const void*)title
	);
	xcb_flush(frame->display_server_connection);
}

void smol_frame_destroy(smol_frame_t* frame) {

	xcb_disconnect(frame->display_server_connection);

	smol_event_queue_destroy(&frame->event_queue);
	if(frame->renderer) {
		xcb_free_gc(frame->display_server_connection, frame->renderer->gc);
		smol_renderer_destroy(frame->renderer);
	}

	smol__num_frames--;
	if(smol__num_frames == 0) {
		xcb_key_symbols_free(smol__keysyms);
	}

	SMOL_FREE(frame);

}

//Can be passed null, this will pump messages to every window (on windows at least)
void smol_frame_update(smol_frame_t* frame) {

	int button_indices[] = {0, 1, 3, 2, 4, 5};

	for(xcb_generic_event_t* xevent; (xevent = xcb_poll_for_event(frame->display_server_connection));) {
		switch(xevent->response_type & 0x7F) {
			case XCB_EXPOSE: {

				xcb_expose_event_t* ev = (xcb_expose_event_t*)xevent;

				if(frame->width != ev->width && frame->height != ev->height) {
					frame->renderer = smol_renderer_create(frame);
					frame->width = ev->width;
					frame->height = ev->height;

					smol_frame_event_t event = {0};
					event.type = SMOL_FRAME_EVENT_RESIZE;
					event.size.width = ev->width;
					event.size.height = ev->height;
					smol_event_queue_push_back(&frame->event_queue, &event);
				} 

				xcb_flush(frame->display_server_connection);

			} break;
			case XCB_CLIENT_MESSAGE: {
				xcb_client_message_event_t* ev = (xcb_client_message_event_t*)xevent; 
				if(ev->data.data32[0] == smol__wm_delete_window_atom) {
					
					frame->should_close = 1;

					smol_frame_event_t event = {0};
					event.type = SMOL_FRAME_EVENT_CLOSED;
					smol_event_queue_push_back(&frame->event_queue, &event);
				}
			} break;
			case XCB_KEY_PRESS: 
			case XCB_KEY_RELEASE: 
			{
				xcb_key_press_event_t* ev = (xcb_key_press_event_t*)xevent;
				smol_frame_event_t event = {0};
				event.type = xevent->response_type == XCB_KEY_PRESS ? SMOL_FRAME_EVENT_KEY_DOWN : SMOL_FRAME_EVENT_KEY_UP;
				xcb_keysym_t keysym = xcb_key_symbols_get_keysym(smol__keysyms, ev->detail, 0);
				event.key.code = smol_frame_mapkey(keysym);
				smol_event_queue_push_back(&frame->event_queue, &event);

			} break;
			case XCB_MOTION_NOTIFY: {
				xcb_motion_notify_event_t* ev = (xcb_motion_notify_event_t*)xevent;
				smol_frame_event_t event = {0};
				event.type = SMOL_FRAME_EVENT_MOUSE_MOVE;

				event.mouse.x = ev->event_x;
				event.mouse.y = ev->event_y;
				
				event.mouse.dx = ev->event_x - frame->old_mouse_x;
				event.mouse.dy = ev->event_y - frame->old_mouse_y;
				
				frame->old_mouse_x = ev->event_x;
				frame->old_mouse_y = ev->event_y;

				smol_event_queue_push_back(&frame->event_queue, &event);
			} break;
			case XCB_BUTTON_PRESS: 
			case XCB_BUTTON_RELEASE:
			{
				xcb_button_press_event_t* ev = (xcb_button_press_event_t*)xevent;
				smol_frame_event_t event = {0};

				if(ev->detail < XCB_BUTTON_INDEX_4) {
					event.type = xevent->response_type == XCB_BUTTON_PRESS ? SMOL_FRAME_EVENT_MOUSE_BUTTON_DOWN : SMOL_FRAME_EVENT_MOUSE_BUTTON_UP;
					event.mouse.button = button_indices[ev->detail];

					event.mouse.x = ev->event_x;
					event.mouse.y = ev->event_y;
					
					event.mouse.dx = ev->event_x - frame->old_mouse_x;
					event.mouse.dy = ev->event_y - frame->old_mouse_y;
					
					frame->old_mouse_x = ev->event_x;
					frame->old_mouse_y = ev->event_y;
				} else {

					if(xevent->response_type == XCB_BUTTON_RELEASE)
						break;

					event.type = SMOL_FRAME_EVENT_MOUSE_VER_WHEEL;
					
					int delta = 0;
					if(ev->detail == XCB_BUTTON_INDEX_4) delta = +1;
					if(ev->detail == XCB_BUTTON_INDEX_5) delta = -1;

					frame->mouse_z_accum += delta;
					event.mouse.dz = delta;
					event.mouse.z = frame->mouse_z_accum;
				}


				smol_event_queue_push_back(&frame->event_queue, &event);

			} break;
			case XCB_FOCUS_IN: {
				smol_frame_event_t event = {0};
				event.type = SMOL_FRAME_EVENT_FOCUS_GAINED;
				smol_event_queue_push_back(&frame->event_queue, &event);
			} break;
			case XCB_FOCUS_OUT: {
				smol_frame_event_t event = {0};
				event.type = SMOL_FRAME_EVENT_FOCUS_LOST;
				smol_event_queue_push_back(&frame->event_queue, &event);
			} break;
		}
		free(xevent);
	}
}


void smol_frame_blit_pixels(
	smol_frame_t* frame, 
	unsigned int* pixels, 
	int width, 
	int height,
	int dstX, 
	int dstY, 
	int dstW, 
	int dstH,
	int srcX, 
	int srcY, 
	int srcW, 
	int srcH
) {

	smol_software_renderer_t* renderer = frame->renderer;
	SMOL_ASSERT("Frame has no renderer! Frame was probably initialized with OpenGL config!" && frame->renderer);

	//Precalculate amount how many pixels are skipped per axis, to opimize drawing
	int startX = dstX < 0 ? -dstX : 0;
	int startY = dstY < 0 ? -dstY : 0;

	//Precalculate amount of how many pixels are shaved off to optimize drawing
	int endX = (dstX + dstW) > renderer->width ? (dstX + dstW - renderer->width) : dstW; 
	int endY = (dstY + dstH) > renderer->height ? (dstY + dstH - renderer->height) : dstH;

	for(int y = startY; y < endY; y++) {
		int sY = srcY + ((srcH * y) / dstH);
		int dY = dstY + y;
		for(int x = startX; x < endX; x++) {
			int sX = srcX + ((srcW * x) / dstW);
			int dX = dstX + x;
			renderer->pixel_data[dX + dY * renderer->width] = pixels[sX + sY * width];
		}
	}

	uint32_t bytesPerRow = 4 * renderer->width;
	xcb_put_image(
		frame->display_server_connection,
		XCB_IMAGE_FORMAT_Z_PIXMAP,     
		frame->frame_window,                 
		renderer->gc,                     
		renderer->width, renderer->height,
		0,                             
		0,                             
		0,                             
		frame->screen->root_depth,                            
		bytesPerRow * renderer->height,
		(uint8_t*)renderer->pixel_data
	);
	
	xcb_flush(frame->display_server_connection);


}


#endif 
#pragma endregion 

#pragma region X11 XCB common
#if (defined(SMOL_FRAME_BACKEND_XCB) || defined(SMOL_FRAME_BACKEND_X11)) && defined(SMOL_PLATFORM_LINUX)
int smol_frame_mapkey(int key) {
	switch(key) {
		case XK_A: case XK_a: return SMOLK_A;
		case XK_B: case XK_b: return SMOLK_B;
		case XK_C: case XK_c: return SMOLK_C;
		case XK_D: case XK_d: return SMOLK_D;
		case XK_E: case XK_e: return SMOLK_E;
		case XK_F: case XK_f: return SMOLK_F;
		case XK_G: case XK_g: return SMOLK_G;
		case XK_H: case XK_h: return SMOLK_H;
		case XK_I: case XK_i: return SMOLK_I;
		case XK_J: case XK_j: return SMOLK_J;
		case XK_K: case XK_k: return SMOLK_K;
		case XK_L: case XK_l: return SMOLK_L;
		case XK_M: case XK_m: return SMOLK_M;
		case XK_N: case XK_n: return SMOLK_N;
		case XK_O: case XK_o: return SMOLK_O;
		case XK_P: case XK_p: return SMOLK_P;
		case XK_Q: case XK_q: return SMOLK_Q;
		case XK_R: case XK_r: return SMOLK_R;
		case XK_S: case XK_s: return SMOLK_S;
		case XK_T: case XK_t: return SMOLK_T;
		case XK_U: case XK_u: return SMOLK_U;
		case XK_V: case XK_v: return SMOLK_V;
		case XK_W: case XK_w: return SMOLK_W;
		case XK_X: case XK_x: return SMOLK_X;
		case XK_Y: case XK_y: return SMOLK_Y;
		case XK_Z: case XK_z: return SMOLK_Z;
		case XK_Escape: return SMOLK_ESC; 
		case XK_Return: return SMOLK_RETURN;
		case XK_BackSpace: return SMOLK_BACKSPACE;
		case XK_space: return SMOLK_SPACE;
		case XK_Control_L: return SMOLK_LCONTROL;
		case XK_Control_R: return SMOLK_RCONTROL; 
		case XK_Alt_L: return SMOLK_LALT; 
		case XK_Alt_R: return SMOLK_RALT;
		case XK_Shift_L: return SMOLK_LSHIFT;
		case XK_Shift_R: return SMOLK_RSHIFT; 
		case XK_Up: return SMOLK_UP;
		case XK_Down: return SMOLK_DOWN;
		case XK_Left: return SMOLK_LEFT;
		case XK_Right: return SMOLK_RIGHT; 
		case XK_Tab: return SMOLK_TAB;
		case XK_Caps_Lock: return SMOLK_CAPSLOCK;
		case XK_Num_Lock: return SMOLK_NUMLOCK;
		case XK_Scroll_Lock: return SMOLK_SCROLLLOCK;
		case XK_Print: return SMOLK_PRINTSCREEN;
		case XK_Pause: return SMOLK_PAUSE;
		case XK_Insert: return SMOLK_INSERT;
		case XK_Home: return SMOLK_HOME;
		case XK_Page_Up: return SMOLK_PAGEUP; 
		case XK_Page_Down: return SMOLK_PAGEDOWN;
		case XK_Delete: return SMOLK_DELETE;
		case XK_End: return SMOLK_END; 
		case XK_0: return SMOLK_NUM0; 
		case XK_1: return SMOLK_NUM1; 
		case XK_2: return SMOLK_NUM2;
		case XK_3: return SMOLK_NUM3;
		case XK_4: return SMOLK_NUM4;
		case XK_5: return SMOLK_NUM5;
		case XK_6: return SMOLK_NUM6;
		case XK_7: return SMOLK_NUM7;
		case XK_8: return SMOLK_NUM8;
		case XK_9: return SMOLK_NUM9;
		case XK_KP_0: return SMOLK_NUMPAD0;
		case XK_KP_1: return SMOLK_NUMPAD1;
		case XK_KP_2: return SMOLK_NUMPAD2;
		case XK_KP_3: return SMOLK_NUMPAD3;
		case XK_KP_4: return SMOLK_NUMPAD4;
		case XK_KP_5: return SMOLK_NUMPAD5;
		case XK_KP_6: return SMOLK_NUMPAD6;
		case XK_KP_7: return SMOLK_NUMPAD7;
		case XK_KP_8: return SMOLK_NUMPAD8;
		case XK_KP_9: return SMOLK_NUMPAD9;
		case XK_F1: return SMOLK_F1;
		case XK_F2: return SMOLK_F2;
		case XK_F3: return SMOLK_F3;
		case XK_F4: return SMOLK_F4;
		case XK_F5: return SMOLK_F5;
		case XK_F6: return SMOLK_F6;
		case XK_F7: return SMOLK_F7;
		case XK_F8: return SMOLK_F8;
		case XK_F9: return SMOLK_F9;
		case XK_F10: return SMOLK_F10;
		case XK_F11: return SMOLK_F11;
		case XK_F12: return SMOLK_F12;
		case XK_plus: return SMOLK_PLUS;
		case XK_minus : return SMOLK_MINUS;
		case XK_KP_Add: return SMOLK_ADD;
		case XK_KP_Subtract: return SMOLK_SUBSTRACT;
		case XK_KP_Multiply: return SMOLK_ASTERISK;
		case XK_comma: return SMOLK_COMMA;
		case XK_period: return SMOLK_PERIOD;
	}
	return SMOLK_UNKNOWN;
}
#endif 
#pragma endregion

#endif 
