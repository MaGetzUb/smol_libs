/*
Copyright © 2023 Marko Ranta (Discord: coderunner)

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

#define SMOL_FRAME_BACKEND_XXXX  (Optional backend, falls back to X11, also Linux only)
#define SMOL_FRAME_IMPLEMENTATION
#include "smol_frame.h"

...
int main() { ... }

If you need to include this file in multiple source files, add empty .c 
file into your project called 'smol_frame.c' for example, and add the 
code lines from above the 'int main()' code.

On Windows and linux, if you include this header, you don't have to link anything
additional to the gcc arguments. You can just do this:
gcc main.c -o main 

And run:
./main

NOTE: Linux does need the libx11-dev package for the X11 related headers though.

NOTE: If you want your application code not to change for Web, you need to add 
-s ASYNCIFY=1 to the linker options. The smol_frame_update then will yield from
the main loop with emscripten_sleep(0).

XCB is bit more cumbersome, because you need these packages:
- libxcb-devel
- libxcb-icccm4-dev
- libxcb-keysyms1-dev
- libxkbcommon-dev
- libxkbcommon-x11-dev
- libegl1-mesa-dev
If there's one unified package, let me know I'll wipe that list.
Anyway compiling with xcb backend would work like this:
gcc main.c -lxcb -lxcb-icccm -lxcb-keysyms -lxkbcommon -lxkbcommon-x11 -lEGL -o main

Currently, XCB backend is statically linked, and there's no function loading at all, 
so you need all those extra arguments for the gcc command line.
 
I'm also considering to drop the XCB backend, because although it's cleaner API, and 
supports better threading etc. It just seems that all the features seem to be bit scattered 
around into separate libraries, and documentation seem to be bit lacking. X11 on the other 
hand is a nice monolith. The problems I've faced with XCB is keysym mapping to utf-32, for
example typing ^ on nordic keyboard requires double tap of ^ key, but with the xkb xcb 
utils it doesn't seem to tirgger, all other keys seem to work fine. This is not problem on X11.
Because GLX with XCB seemed bit tricky I decided to go with EGL, 
but that feature is still not working, after Making context curret, I get EGL_BAD_ACCESS error. 
- MaGetzUb


If you're using WAYLAND you have to link those libraries
accordingly. (Wayland not implemented yet)
*/

/*
TODO: 
- Icons, Cursors, Mouse hiding
- Key modifiers to the events
- Some sort of generic drag / drop system for items (files at starters)
- Backends (Wayland for Linux, Mac, maybe Web and android)
- High DPI stuff

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
#	include <WinUser.h>
#	include <wingdi.h>
#	ifdef _MSC_VER
#		pragma comment(lib, "kernel32.lib")
#		pragma comment(lib, "user32.lib")
#		pragma comment(lib, "ole32.lib")
#		pragma comment(lib, "shell32.lib")
#		ifndef APIENTRY
#			define APIENTRY __stdcall
#		endif 
#	else 
#		ifndef APIENTRY
#			define APIENTRY __attribute__((stdcall))
#		endif 
#	endif 
#elif defined(__linux__) 
#	ifndef SMOL_PLATFORM_LINUX
#	define SMOL_PLATFORM_LINUX
#	endif 
#	include <unistd.h>
#	include <dlfcn.h>
#	include <signal.h>
#	if defined(SMOL_FRAME_BACKEND_XCB)
#		include <xcb/xcb.h>
#		include <xcb/xcb_icccm.h>
#		include <xcb/xcb_keysyms.h>
#		include <xcb/xkb.h>
#		include <X11/keysym.h>
#		include <xkbcommon/xkbcommon.h>
#		include <xkbcommon/xkbcommon-x11.h>
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
#		include <X11/Xatom.h>
#		ifndef GLX_VERSION_1_0
typedef struct __GLXcontextRec *GLXContext;
#		endif 
#	endif 
#	if defined(SMOL_FRAME_BACKEND_XCB) || defined(SMOL_FRAME_BACKEND_WAYLAND) 
#		include <EGL/egl.h>
#		include <EGL/eglext.h>
#	endif 
#elif defined(__APPLE__)
#	ifndef SMOL_PLATFORM_MAX_OS
#	define SMOL_PLATFORM_MAX_OS
#	endif 
//TODO:
#	error Mac OS backend not implemented yet!
#elif defined(__EMSCRIPTEN__)
#	define SMOL_PLATFORM_WEB
#	include <emscripten.h>
#	include <emscripten/html5.h>
#	include <emscripten/key_codes.h>
#	include <EGL/egl.h>
#	include <EGL/eglext.h>
#endif 
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#define SMOL_TRUE 1
#define SMOL_FALSE 0

#define SMOL_FRAME_EVENT_LOOP(frame, it) for(smol_frame_event_t it; smol_frame_acquire_event(frame, &it);) 

//Forward declare smol frame gl specification structure
typedef struct _smol_frame_gl_spec_t smol_frame_gl_spec_t;

//Forward declare smol frame configuration structure
typedef struct _smol_frame_config_t smol_frame_config_t;

//Forward declaration for frame handle
typedef struct _smol_frame_t smol_frame_t;

//Forward declare smol frame event queue
typedef struct _smol_event_queue_t smol_event_queue_t;

//Forward declare smol frame opengl context structure
typedef struct _smol_gl_context_t smol_gl_context_t;

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
smol_frame_t* smol_frame_create_advanced(const smol_frame_config_t* config);

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

//smol_frame_width - Returns the width of the frame
//Arguments:
// - smol_frame_t* frame - Whose width h is qureried
//Returns: int -- containing the frame width
int smol_frame_width(smol_frame_t* frame);


//smol_frame_height - Returns the width of the frame
//Arguments:
// - smol_frame_t* frame - Whose width h is qureried
//Returns: int -- containing the frame width
int smol_frame_height(smol_frame_t* frame);

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

//smol_frame_set_cursor_visiblity - Sets the mouse cursor hidden or visible
//Arguments:
// - smol_frame_t* -- A frame that's the cursor hiding is applied
// - int is_visible -- If not true, cursor is hidden
void smol_frame_set_cursor_visibility(smol_frame_t* frame, int hidden);

//smol_frame_blit_pixels - Blits pixels to frame 
//Arguments: 
// - smol_frame_t* frame  -- A window that's being drawn to
// - unsigned int* pixBuf -- A pointer to pixel buffer, pixels are in LSB order of R,G,B,A.
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

//smol_frame_get_event_queue - Get the pointer to event queue.
//Arguments: 
// - smol_frame_t* frame -- A window that's event queue is requested
//Returns: smol_event_queue_t* - the pointer to event queue
smol_event_queue_t* smol_frame_get_event_queue(smol_frame_t* frame);


//smol_frame_get_event_queue - Set the frame's event queue.
//Arguments: 
// - smol_frame_t* frame -- A window that's event queue is being set
void smol_frame_set_event_queue(smol_frame_t* frame, smol_event_queue_t* eventQueue);


//smol_init_gl_spec - Initializes smol_frame_gl_spec_t structure (R,G,B,A,Stencil bits wil lbe defaulted to 8, and depth to 24)
//Arguments: 
// - int major_version          -- Major OpenGL version eg. 4
// - int minor_version          -- Minor OpenGL version eg. 6
// - int is_backward_compatible -- Is the created context backward compatible?
// - int is_forward_compatible  -- Is the created context forward compatible?
// - int multisamples           -- Is the context capable of multisampling in the window frame buffer? Values equal or over 4 will enable multisampling
// - int is_debug               -- Does the context have support for debug callbacks
//Returns: smol_frame_gl_spec_t - containing the prefilled gl spec strucutre.
smol_frame_gl_spec_t smol_init_gl_spec(int major_version, int minor_version, int is_backward_compatible, int is_forward_compatible, int multisamples, int is_debug);

//smol_frame_gl_swap_buffers - Swaps opengl buffers
//Arguments:
// - smol_frame_t* frame frame -- A window that's opengl buffers are being swapped
//Returns: int - containing 1 if succeeded, 0 otherwise
int smol_frame_gl_swap_buffers(smol_frame_t* frame);

//smol_frame_get_gl_context - Gets the Window's module handle on Windows 
//Arguments:
// - smol_frame_t* frame -- A frame we want the opengl context structure from
//Returns: smol_gl_context_t - an OpenGl context structure
smol_gl_context_t smol_frame_get_gl_context(smol_frame_t* frame);

//smol_gl_get_proc_address - Gets an OpenGL function pointer 
//Arguments:
// - const char* func  -- A name of the function to load.  
//Returns: void* containing the OpenGL function handle.
void* smol_gl_get_proc_address(const char* func);

//smol_gl_set_vsync - Sets VSync with SwapIntervalEXT
//Arguments 
// - int enabled -- A boolean for setting VSync on / off
void smol_gl_set_vsync(int enabled);

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
	SMOL_FRAME_CONFIG_OWNS_EVENT_QUEUE    = 0x00000008U,
	SMOL_FRAME_DEFAULT_CONFIG             = (
		SMOL_FRAME_CONFIG_HAS_TITLEBAR | SMOL_FRAME_CONFIG_OWNS_EVENT_QUEUE
	)
} smol_frame_config_flags;

//All the event types
typedef enum smol_frame_event_type_ {
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

typedef struct _smol_frame_gl_spec_t {
	int major_version;
	int minor_version;
	int is_backward_compatible;
	int is_forward_compatible;
	int is_debug;
	int red_bits; //Usually 8
	int green_bits; //Usually 8
	int blue_bits; //Usually 8
	int alpha_bits; //Usually 8
	int depth_bits; //Usually 16, 24 or 32
	int stencil_bits; //Usually 8
	int has_multi_sampling;
	int num_multi_samples;
} smol_frame_gl_spec_t;

typedef struct _smol_frame_config_t {
	int width;
	int height;
	const char* title;
	unsigned int flags;
	union {
		smol_frame_t* parent;
		const char* web_element;
	};
	const smol_frame_gl_spec_t* gl_spec;
} smol_frame_config_t;

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

typedef struct {
	unsigned int codepoint;
} smol_frame_text_input;


//Most rudimentary event structure
typedef struct _smol_frame_event_t {
	smol_frame_event_type type;
	smol_frame_t* frame;
	union {
		smol_frame_key_event key;
		smol_frame_mouse_event mouse;
		smol_frame_resize_event size;
		smol_frame_text_input input;
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

#if defined(SMOL_PLATFORM_WINDOWS)
#	ifndef SMOL_BREAKPOINT
#		ifdef _MSC_VER
#			define SMOL_BREAKPOINT() __debugbreak() //MSVC uses this
#		else
#			define SMOL_BREAKPOINT() DebugBreak()
#		endif 
#	endif 
#elif defined(SMOL_PLATFORM_WEB)
#	define SMOL_BREAKPOINT() EM_ASM({ debugger; })
#elif defined(SMOL_PLATFORM_LINUX)
#	ifndef SMOL_BREAKPOINT
#		define SMOL_BREAKPOINT() raise(SIGTRAP)
#	endif
#endif 

#ifndef SMOL_SYMBOLIFY
#define SMOL_SYMBOLIFY(x) #x
#endif 

#ifndef SMOL_STRINGIFY
#define SMOL_STRINGIFY(x) SMOL_SYMBOLIFY(x)
#endif 

#ifndef SMOL_ASSERT
#define SMOL_ASSERT(condition) \
	if(!(condition)) \
		puts(\
			"SMOL FRAME ASSERTION FAILED!\n" \
			"CONDITION: " #condition "\n" \
			"IN FILE: '" __FILE__ "'\n" \
			"ON LINE: " SMOL_STRINGIFY(__LINE__) "\n" \
		), \
		SMOL_BREAKPOINT()
#endif 

typedef struct _smol_event_queue_t {
	smol_frame_event_t* events;
	int event_count;
	int front_index;
	int back_index;
	int mid_point; //Used to forr offset calculation.. (MAX_INT/2)-(MAX_INT/2 % event_count)
	int refs;
} smol_event_queue_t;

#if !(defined(SMOL_PLATFORM_WINDOWS) || defined(SMOL_PLATFORM_WEB))
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

#if defined(SMOL_PLATFORM_WINDOWS)
typedef struct _smol_gl_context_t {
	HGLRC context;
} smol_gl_context_t;
#elif defined(SMOL_PLATFORM_LINUX)
#	if defined(SMOL_FRAME_BACKEND_XCB) || defined(SMOL_FRAME_BACKEND_WAYLAND) 
typedef struct _smol_gl_context_t {
	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;
};
#else 
typedef struct _smol_gl_context_t {
	GLXContext context;
} smol_gl_context_t;
#endif 
#elif defined(SMOL_PLATFORM_WEB)
typedef struct _smol_gl_context_t {
	EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context;
};
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
	xcb_visualtype_t *visual;
#	endif
#	if defined(SMOL_FRAME_BACKEND_XCB) || defined(SMOL_FRAME_BACKEND_WAYLAND) 
	struct xkb_context* kbcontext;
	struct xkb_keymap* kbkeymap;
	struct xkb_state* kbstate;
#endif 
	smol_software_renderer_t* renderer;
#elif defined(SMOL_PLATFORM_WEB)
	const char* element;
#endif 
	int width;
	int height;
	
	int should_close;
	
	int old_mouse_x;
	int old_mouse_y;
	
	int mouse_z_accum;
	int mouse_w_accum;

	smol_event_queue_t* event_queue;
	smol_gl_context_t gl;
} smol_frame_t;


#pragma region Event queue implementation

//TODO: sanity check this code that it doesn't have logical flaws, so far it has worked fine. ~MaGetzUb

smol_event_queue_t* smol_event_queue_create(int numEvents) {

	smol_event_queue_t* result = SMOL_ALLOC_INSTANCE(smol_event_queue_t);
	result->events = SMOL_ALLOC_ARRAY(smol_frame_event_t, numEvents);
	result->event_count = numEvents;
	result->back_index = 0;
	result->front_index = 0;
	result->mid_point = 0x7FFFFFFF;
	result->mid_point -= (result->mid_point % result->event_count);
	result->refs = 1;

	return result;
}

void smol_event_queue_destroy(smol_event_queue_t** queue) {
	if(--(*queue)->refs <= 0) {
		(*queue)->back_index = 0;
		(*queue)->front_index = 0;
		(*queue)->event_count = 0;
		SMOL_FREE((*queue)->events);
		SMOL_FREE((*queue));
	}
	*queue = NULL;
}

int smol_event_queue_push_back(smol_event_queue_t* queue, const smol_frame_event_t* event) {

	if(queue->back_index < queue->front_index)
		return 0;

	int index = queue->back_index++;
	index = (queue->mid_point + index) % queue->event_count;
	queue->events[index] = *event;

	return 1;
}

int smol_event_queue_push_front(smol_event_queue_t* queue, const smol_frame_event_t* event) {

	if(queue->back_index < queue->front_index)
		return 0;

	int index = queue->front_index--;
	index = (queue->mid_point + index) % queue->event_count;
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
	index =  (queue->mid_point + index) % queue->event_count;
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
	index = (queue->mid_point + index) % queue->event_count;
	*event = queue->events[index];

	if(queue->front_index == queue->back_index) {
		queue->front_index = queue->back_index = 0;
	}

	return 1;
}

#pragma endregion 

#pragma region Platform agnostic functions

smol_frame_gl_spec_t smol_init_gl_spec(int major_version, int minor_version, int is_backward_compatible, int is_forward_compatible, int multisamples, int is_debug) {
	smol_frame_gl_spec_t result = {0};
	result.major_version = major_version;
	result.minor_version = minor_version;
	result.is_backward_compatible = is_backward_compatible;
	result.is_forward_compatible = is_forward_compatible;
	result.is_debug = is_debug;
	result.red_bits = 8;
	result.green_bits = 8;
	result.blue_bits = 8;
	result.alpha_bits = 8;
	result.depth_bits = 24;
	result.stencil_bits = 8;
	result.has_multi_sampling = multisamples >= 4;
	result.num_multi_samples = multisamples;
	return result;
}

smol_frame_t* smol_frame_create(int width, int height, const char* title) {

	smol_frame_config_t config = { 0 };
	config.width = width;
	config.height = height;
	config.title = title;
	config.flags = SMOL_FRAME_DEFAULT_CONFIG;

	return smol_frame_create_advanced(&config);

}

int smol_frame_acquire_event(smol_frame_t* frame, smol_frame_event_t* event) {

	int nEvents = smol_event_queue_count(frame->event_queue);

	if(nEvents <= 0)
		return 0;
	
	if(smol_event_queue_pop_front(frame->event_queue, event))
		return nEvents;
	
	return 0;
}

int smol_frame_is_closed(smol_frame_t* frame) {
	return frame->should_close;
}

int smol_frame_width(smol_frame_t* frame) {
	return frame->width;
}

int smol_frame_height(smol_frame_t* frame) {
	return frame->height;
}


void smol_frame_set_event_queue(smol_frame_t* frame, smol_event_queue_t* evq) {
	if(frame->event_queue) {
		smol_event_queue_destroy(&frame->event_queue);
	}
	frame->event_queue = evq;
	evq->refs++;
}

smol_event_queue_t* smol_frame_get_event_queue(smol_frame_t* frame) {
	return frame->event_queue;
}

smol_gl_context_t smol_frame_get_gl_context(smol_frame_t* frame) {
	return frame->gl;
}

#pragma endregion 

#pragma region Win32 Implementation
#if defined(SMOL_PLATFORM_WINDOWS)

static WNDCLASSEX smol__wnd_class;
static HMODULE smol__wingdi;

typedef HGDIOBJ smol_GetStockObject_proc(int i);
typedef int smol_ChoosePixelFormat_proc(HDC hdc, const PIXELFORMATDESCRIPTOR *ppfd);
typedef BOOL smol_SetPixelFormat_proc(HDC hdc,int format, const PIXELFORMATDESCRIPTOR *ppfd);
typedef BOOL smol_SwapBuffers_proc(HDC unnamedParam1);
typedef int smol_StretchDIBits_proc(HDC hdc, int xDest, int yDest, int DestWidth, int DestHeight, int xSrc, int ySrc, int SrcWidth, int SrcHeight, const VOID* lpBits, const BITMAPINFO* lpbmi, UINT iUsage, DWORD rop);

static smol_GetStockObject_proc* smol_GetStockObject;
static smol_ChoosePixelFormat_proc* smol_ChoosePixelFormat;
static smol_SetPixelFormat_proc* smol_SetPixelFormat;
static smol_SwapBuffers_proc* smol_SwapBuffers;
static smol_StretchDIBits_proc* smol_StretchDIBits;

//Some OpenGL Stuff for windows:
typedef HGLRC APIENTRY smol_wglCreateContext_proc(HDC);
typedef BOOL APIENTRY smol_wglDeleteContext_proc(HGLRC);
typedef BOOL APIENTRY smol_wglMakeCurrent_proc(HDC, HGLRC);
typedef PROC APIENTRY smol_wglGetProcAddress_proc(LPCSTR);
typedef PROC APIENTRY smol_wglSwapIntervalEXT_proc(int);

typedef HGLRC APIENTRY smol_wglCreateContextAttribsARB_proc(HDC hDC, HGLRC hshareContext, const int *attribList);

typedef BOOL APIENTRY smol_wglChoosePixelFormatARB_proc(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
typedef BOOL APIENTRY smol_wglGetPixelFormatAttribivARB_proc(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, int *piValues);
typedef BOOL APIENTRY smol_wglGetPixelFormatAttribfvARB_proc(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, FLOAT *pfValues);



//https://registry.khronos.org/OpenGL/extensions/ARB/WGL_ARB_create_context.txt
//Accepted as an attribute name in <*attribList>:
#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB             0x2093
#define WGL_CONTEXT_FLAGS_ARB                   0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB            0x9126

//Accepted as bits in the attribute value for WGL_CONTEXT_FLAGS in
//<*attribList>:
#define WGL_CONTEXT_DEBUG_BIT_ARB               0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB  0x0002

//Accepted as bits in the attribute value for
//WGL_CONTEXT_PROFILE_MASK_ARB //in <*attribList>:
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB        0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

//New errors returned by GetLastError:
#define ERROR_INVALID_VERSION_ARB               0x2095
#define ERROR_INVALID_PROFILE_ARB               0x2096

//https://registry.khronos.org/OpenGL/extensions/ARB/WGL_ARB_pixel_format.txt
//Accepted in the <piAttributes> parameter array of
//wglGetPixelFormatAttribivARB, and wglGetPixelFormatAttribfvARB, and
//as a type in the <piAttribIList> and <pfAttribFList> parameter
//arrays of wglChoosePixelFormatARB
#define WGL_NUMBER_PIXEL_FORMATS_ARB            0x2000
#define WGL_DRAW_TO_WINDOW_ARB                  0x2001
#define WGL_DRAW_TO_BITMAP_ARB                  0x2002
#define WGL_ACCELERATION_ARB                    0x2003
#define WGL_NEED_PALETTE_ARB                    0x2004
#define WGL_NEED_SYSTEM_PALETTE_ARB             0x2005
#define WGL_SWAP_LAYER_BUFFERS_ARB              0x2006
#define WGL_SWAP_METHOD_ARB                     0x2007
#define WGL_NUMBER_OVERLAYS_ARB                 0x2008
#define WGL_NUMBER_UNDERLAYS_ARB                0x2009
#define WGL_TRANSPARENT_ARB                     0x200A
#define WGL_TRANSPARENT_RED_VALUE_ARB           0x2037
#define WGL_TRANSPARENT_GREEN_VALUE_ARB         0x2038
#define WGL_TRANSPARENT_BLUE_VALUE_ARB          0x2039
#define WGL_TRANSPARENT_ALPHA_VALUE_ARB         0x203A
#define WGL_TRANSPARENT_INDEX_VALUE_ARB         0x203B
#define WGL_SHARE_DEPTH_ARB                     0x200C
#define WGL_SHARE_STENCIL_ARB                   0x200D
#define WGL_SHARE_ACCUM_ARB                     0x200E
#define WGL_SUPPORT_GDI_ARB                     0x200F
#define WGL_SUPPORT_OPENGL_ARB                  0x2010
#define WGL_DOUBLE_BUFFER_ARB                   0x2011
#define WGL_STEREO_ARB                          0x2012
#define WGL_PIXEL_TYPE_ARB                      0x2013
#define WGL_COLOR_BITS_ARB                      0x2014
#define WGL_RED_BITS_ARB                        0x2015
#define WGL_RED_SHIFT_ARB                       0x2016
#define WGL_GREEN_BITS_ARB                      0x2017
#define WGL_GREEN_SHIFT_ARB                     0x2018
#define WGL_BLUE_BITS_ARB                       0x2019
#define WGL_BLUE_SHIFT_ARB                      0x201A
#define WGL_ALPHA_BITS_ARB                      0x201B
#define WGL_ALPHA_SHIFT_ARB                     0x201C
#define WGL_ACCUM_BITS_ARB                      0x201D
#define WGL_ACCUM_RED_BITS_ARB                  0x201E
#define WGL_ACCUM_GREEN_BITS_ARB                0x201F
#define WGL_ACCUM_BLUE_BITS_ARB                 0x2020
#define WGL_ACCUM_ALPHA_BITS_ARB                0x2021
#define WGL_DEPTH_BITS_ARB                      0x2022
#define WGL_STENCIL_BITS_ARB                    0x2023
#define WGL_AUX_BUFFERS_ARB                     0x2024

//Accepted as a value in the <piAttribIList> and <pfAttribFList>
//parameter arrays of wglChoosePixelFormatARB, and returned in the
//<piValues> parameter array of wglGetPixelFormatAttribivARB, and the
//<pfValues> parameter array of wglGetPixelFormatAttribfvARB:
#define WGL_NO_ACCELERATION_ARB                 0x2025
#define WGL_GENERIC_ACCELERATION_ARB            0x2026
#define WGL_FULL_ACCELERATION_ARB               0x2027

#define WGL_SWAP_EXCHANGE_ARB                   0x2028
#define WGL_SWAP_COPY_ARB                       0x2029
#define WGL_SWAP_UNDEFINED_ARB                  0x202A

#define WGL_TYPE_RGBA_ARB                       0x202B
#define WGL_TYPE_COLORINDEX_ARB                 0x202C

// https://registry.khronos.org/OpenGL/extensions/ARB/ARB_multisample.txt
//Accepted by the <piAttributes> parameter of
//wglGetPixelFormatAttribivEXT, wglGetPixelFormatAttribfvEXT, and
//the <piAttribIList> and <pfAttribIList> of wglChoosePixelFormatEXT:
#define WGL_SAMPLE_BUFFERS_ARB               0x2041
#define WGL_SAMPLES_ARB                      0x2042

//Accepted by the <cap> parameter of Enable, Disable, and IsEnabled,
//and by the <pname> parameter of GetBooleanv, GetIntegerv,
//GetFloatv, and GetDoublev:
#define GL_MULTISAMPLE_ARB                      0x809D
#define GL_SAMPLE_ALPHA_TO_COVERAGE_ARB         0x809E
#define GL_SAMPLE_ALPHA_TO_ONE_ARB              0x809F
#define GL_SAMPLE_COVERAGE_ARB                  0x80A0

//    Accepted by the <mask> parameter of PushAttrib:
#define GL_MULTISAMPLE_BIT_ARB                  0x20000000

//Accepted by the <pname> parameter of GetBooleanv, GetDoublev,
//GetIntegerv, and GetFloatv:
#define GL_SAMPLE_BUFFERS_ARB                   0x80A8
#define GL_SAMPLES_ARB                          0x80A9
#define GL_SAMPLE_COVERAGE_VALUE_ARB            0x80AA
#define GL_SAMPLE_COVERAGE_INVERT_ARB           0x80AB


static smol_wglCreateContext_proc *smol_wglCreateContext = NULL;
static smol_wglDeleteContext_proc* smol_wglDeleteContext = NULL;
static smol_wglMakeCurrent_proc* smol_wglMakeCurrent = NULL;
static smol_wglGetProcAddress_proc* smol_wglGetProcAddress = NULL;
static smol_wglSwapIntervalEXT_proc* smol_wglSwapIntervalEXT = NULL;

static smol_wglCreateContextAttribsARB_proc* smol_wglCreateContextAttribsARB = NULL;

static smol_wglGetPixelFormatAttribfvARB_proc* smol_wglGetPixelFormatAttribfvARB = NULL;
static smol_wglGetPixelFormatAttribivARB_proc* smol_wglGetPixelFormatAttribivARB = NULL;
static smol_wglChoosePixelFormatARB_proc* smol_wglChoosePixelFormatARB = NULL;

LRESULT CALLBACK smol_frame_handle_event(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);

smol_frame_t* smol_frame_create_advanced(const smol_frame_config_t* config) {

	RECT win_rect = { 0 };
	HWND wnd = NULL;
	POINT point = { 0 };
	DWORD ex_style = WS_VISIBLE | WS_SYSMENU | WS_MINIMIZEBOX;
	smol_frame_t* result = NULL;

	if(smol__wingdi == NULL) {
		smol__wingdi = LoadLibrary(TEXT("Gdi32.dll"));
		smol_GetStockObject = (smol_GetStockObject_proc*)GetProcAddress(smol__wingdi, "GetStockObject");
		smol_ChoosePixelFormat = (smol_ChoosePixelFormat_proc*)GetProcAddress(smol__wingdi, "ChoosePixelFormat");
		smol_SetPixelFormat = (smol_SetPixelFormat_proc*)GetProcAddress(smol__wingdi, "SetPixelFormat");
		smol_SwapBuffers = (smol_SwapBuffers_proc*)GetProcAddress(smol__wingdi, "SwapBuffers");
		smol_StretchDIBits = (smol_StretchDIBits_proc*)GetProcAddress(smol__wingdi, "StretchDIBits");
	}


	if(smol__wnd_class.cbSize == 0) {

		smol__wnd_class.cbSize = sizeof(smol__wnd_class);
		smol__wnd_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
		smol__wnd_class.lpfnWndProc = &smol_frame_handle_event;
		smol__wnd_class.cbClsExtra = 0;
		smol__wnd_class.cbWndExtra = 0;
		smol__wnd_class.hInstance = GetModuleHandle(0);
		smol__wnd_class.hIcon = LoadIcon(NULL, IDI_WINLOGO);
		smol__wnd_class.hCursor = LoadCursor(NULL, IDC_ARROW);
		smol__wnd_class.hbrBackground = (HBRUSH)smol_GetStockObject(BLACK_BRUSH);
		smol__wnd_class.lpszMenuName = NULL;
		smol__wnd_class.lpszClassName = TEXT("smol_frame");
		smol__wnd_class.hIconSm = NULL;
		
		ATOM registerResult = RegisterClassEx(&smol__wnd_class);
		
		SMOL_ASSERT("Window class initialization failed!" && registerResult);

		if(registerResult == 0) {
			return NULL;
		}

	}

	if(GetCursorPos(&point) == TRUE) {
		HMONITOR monitor = MonitorFromPoint(point, MONITOR_DEFAULTTONEAREST);
		MONITORINFO monitorInfo = { sizeof(monitorInfo) };
		if(GetMonitorInfo(monitor, &monitorInfo) != FALSE) {
			win_rect.left = ((monitorInfo.rcMonitor.left + monitorInfo.rcMonitor.right) - config->width) >> 1;
			win_rect.top = ((monitorInfo.rcMonitor.top + monitorInfo.rcMonitor.bottom) - config->height) >> 1;
			win_rect.right = win_rect.left + config->width;
			win_rect.bottom = win_rect.top + config->height;
		}
	}

	if(config->flags & SMOL_FRAME_CONFIG_HAS_TITLEBAR) ex_style |= WS_CAPTION;
	if(config->flags & SMOL_FRAME_CONFIG_HAS_MAXIMIZE_BUTTON) ex_style |= WS_MAXIMIZEBOX;
	if(config->flags & SMOL_FRAME_CONFIG_IS_RESIZABLE) ex_style |= WS_SIZEBOX;

	AdjustWindowRect(&win_rect, ex_style, FALSE);

	
	result = SMOL_ALLOC_INSTANCE(smol_frame_t);
	SMOL_ASSERT("Unable to allocate result!" && result);
	memset(result, 0, sizeof(smol_frame_t));

	const TCHAR* title_text = NULL;

#ifdef UNICODE
	wchar_t wide_title[256] = { 0 };
	MultiByteToWideChar(CP_UTF8, MB_COMPOSITE, config->title, (int)strlen(config->title), wide_title, 256);
	title_text = wide_title;
#else 
	title_text = config->title;
#endif 

	wnd = CreateWindowEx(
		0, 
		smol__wnd_class.lpszClassName, 
		title_text,
		ex_style, 
		win_rect.left, 
		win_rect.top, 
		win_rect.right - win_rect.left, 
		win_rect.bottom - win_rect.top, 
		config->parent ? config->parent->frame_handle_win32 : NULL, 
		NULL, 
		smol__wnd_class.hInstance,
		result
	);

	if(IsWindow(wnd) == TRUE) {

		result->frame_handle_win32 = wnd;
		result->module_handle_win32 = smol__wnd_class.hInstance;
		result->event_queue = smol_event_queue_create(2048);
		result->width = config->width;
		result->height = config->height;

	} else {

		SMOL_ASSERT(!"Window creation failed!");
		SMOL_FREE(result);
		result = NULL;

	}

	if(config->gl_spec) {


		HDC dc = GetDC(wnd);
		const smol_frame_gl_spec_t* gl_spec = config->gl_spec;
		
		int has_gl_arb_functionality = (smol_wglChoosePixelFormatARB && smol_wglCreateContextAttribsARB && smol_wglGetPixelFormatAttribfvARB && smol_wglGetPixelFormatAttribivARB);

		if(!has_gl_arb_functionality) {

			HMODULE gl_module = LoadLibrary(TEXT("opengl32.dll"));

			smol_wglCreateContext = (smol_wglCreateContext_proc*)GetProcAddress(gl_module, "wglCreateContext");
			smol_wglDeleteContext = (smol_wglDeleteContext_proc*)GetProcAddress(gl_module, "wglDeleteContext");
			smol_wglMakeCurrent = (smol_wglMakeCurrent_proc*)GetProcAddress(gl_module, "wglMakeCurrent");
			smol_wglGetProcAddress = (smol_wglGetProcAddress_proc*)GetProcAddress(gl_module, "wglGetProcAddress");

			//This step is probably not necessary, since we already have a window. 
			HWND tmp = CreateWindowEx(0, smol__wnd_class.lpszClassName, TEXT(""), 0, 0, 0, 400, 300, NULL, NULL, smol__wnd_class.hInstance, NULL);

			HDC hdc = GetDC(tmp);

			PIXELFORMATDESCRIPTOR pfd = { 0 };
			pfd.nSize = sizeof(pfd);
			pfd.nVersion = 1;
			pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER | PFD_SWAP_EXCHANGE;
			pfd.iPixelType = PFD_TYPE_RGBA;
			pfd.cColorBits = 32;
			pfd.cDepthBits = 24;
			pfd.cAlphaBits = 8;
			pfd.iLayerType = PFD_MAIN_PLANE;

			int pixelFormat = smol_ChoosePixelFormat(hdc, &pfd);

			smol_SetPixelFormat(hdc, pixelFormat, &pfd);

			HGLRC tmpCtx = smol_wglCreateContext(hdc);
			smol_wglMakeCurrent(hdc, tmpCtx);

			smol_wglCreateContextAttribsARB = (smol_wglCreateContextAttribsARB_proc*)smol_wglGetProcAddress("wglCreateContextAttribsARB");
			smol_wglChoosePixelFormatARB = (smol_wglChoosePixelFormatARB_proc*)smol_wglGetProcAddress("wglChoosePixelFormatARB");
			smol_wglGetPixelFormatAttribfvARB = (smol_wglGetPixelFormatAttribfvARB_proc*)smol_wglGetProcAddress("wglGetPixelFormatAttribfvARB");
			smol_wglGetPixelFormatAttribivARB = (smol_wglGetPixelFormatAttribivARB_proc*)smol_wglGetProcAddress("wglGetPixelFormatAttribivARB");
			smol_wglSwapIntervalEXT = (smol_wglSwapIntervalEXT_proc*)smol_wglGetProcAddress("wglSwapIntervalEXT");
			has_gl_arb_functionality = (smol_wglChoosePixelFormatARB && smol_wglCreateContextAttribsARB && smol_wglGetPixelFormatAttribfvARB && smol_wglGetPixelFormatAttribivARB);

			if(has_gl_arb_functionality) {
				smol_wglMakeCurrent(hdc, NULL);
				smol_wglDeleteContext(tmpCtx);
				DestroyWindow(tmp);
			}

		}
		

		if(has_gl_arb_functionality) {

			int formats[64] = { 0 };
			UINT numFormats;
			PIXELFORMATDESCRIPTOR pfd = { 0 };

			int pixelformat_atribs[64] = {
				WGL_DRAW_TO_WINDOW_ARB, 1,
				WGL_SUPPORT_OPENGL_ARB, 1,
				WGL_DRAW_TO_WINDOW_ARB, 1,
				WGL_DOUBLE_BUFFER_ARB,  1,
				WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
				//WGL_COLOR_BITS_ARB,     gl_spec->red_bits + gl_spec->green_bits + gl_spec->blue_bits,
				WGL_RED_BITS_ARB,       gl_spec->red_bits,
				WGL_GREEN_BITS_ARB,     gl_spec->green_bits,
				WGL_BLUE_BITS_ARB,      gl_spec->blue_bits,
				WGL_ALPHA_BITS_ARB,     gl_spec->alpha_bits,
				WGL_DEPTH_BITS_ARB,     gl_spec->depth_bits,
				WGL_STENCIL_BITS_ARB,   gl_spec->stencil_bits,
				WGL_SAMPLE_BUFFERS_ARB, gl_spec->has_multi_sampling
			};
			

			//wglChoosePixelFormat(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
			BOOL res = smol_wglChoosePixelFormatARB(dc, pixelformat_atribs, NULL, 64, formats, &numFormats);

			UINT selected = 0;
			int samples_diff = 999;
			int selected_samples = 0;
			if(res && numFormats && gl_spec->has_multi_sampling) {
				for(UINT i = 0; i < numFormats; i++) {
					int ids[] = {
						WGL_SAMPLE_BUFFERS_ARB,
						WGL_SAMPLES_ARB,
						WGL_DEPTH_BITS_ARB,
						WGL_STENCIL_BITS_ARB
					};
					int values[4] = {0};
					BOOL has_feature = smol_wglGetPixelFormatAttribivARB(dc, formats[i], 0, 4, ids, values);
					
					if(values[2] != gl_spec->depth_bits || values[3] != gl_spec->stencil_bits)
						continue;
					if(values[0]) {
						int diff = values[1] - gl_spec->num_multi_samples;
						if(diff < 0) diff = -diff;
						if(diff < samples_diff) {
			
							pixelformat_atribs[16] = WGL_SAMPLE_BUFFERS_ARB;
							pixelformat_atribs[17] = gl_spec->has_multi_sampling;
							pixelformat_atribs[18] = WGL_SAMPLES_ARB;
							pixelformat_atribs[19] = values[1];

							samples_diff = diff;
							selected = i;
						}
						if(
							samples_diff == 0 && 
							values[3] == gl_spec->depth_bits && 
							values[4] == gl_spec->stencil_bits
						) 
							break;
					}
					
				}
			}

			if(smol_SetPixelFormat(dc, formats[selected], &pfd) == FALSE) {
				printf("Failed to set pixel format! Error: %08x\n", GetLastError());
			}

			int profile_mask = WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
			int context_flags = 0;
			if(gl_spec->is_debug)              context_flags |= WGL_CONTEXT_DEBUG_BIT_ARB;
			if(gl_spec->is_forward_compatible) context_flags |= WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;

			if(gl_spec->is_backward_compatible) profile_mask = WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;

			int context_attribs[] = {
				WGL_CONTEXT_MAJOR_VERSION_ARB, gl_spec->major_version,
				WGL_CONTEXT_MINOR_VERSION_ARB, gl_spec->major_version,
				WGL_CONTEXT_PROFILE_MASK_ARB, profile_mask,
				WGL_CONTEXT_FLAGS_ARB, context_flags,
				0
			};

			result->gl.context = smol_wglCreateContextAttribsARB(dc, NULL, context_attribs);
			smol_wglMakeCurrent(dc, result->gl.context);
		}

	}

	return result;


}

void smol_frame_set_title(smol_frame_t* frame, const char* title) {
#ifdef UNICODE
	wchar_t wide_title[256] = { 0 };
	MultiByteToWideChar(CP_UTF8, MB_COMPOSITE, title, (int)strlen(title), wide_title, 256);	
	SMOL_ASSERT("Failed to set window title!" && SetWindowText(frame->frame_handle_win32, wide_title));
#else 
	SMOL_ASSERT("Failed to set window title!" && SetWindowText(frame->frame_handle_win32, title));
#endif 
}

HWND smol_frame_get_win32_window_handle(smol_frame_t* frame) {
	return frame->frame_handle_win32;
}

HINSTANCE smol_frame_get_win32_module_handle(smol_frame_t* frame) {
	return frame->module_handle_win32;
}

void* smol_gl_get_proc_address(const char* func) {
	if(!smol_wglGetProcAddress) {
		fputs("OpenGL context not initialized!", stderr);
		return NULL;
	}
	return smol_wglGetProcAddress(func);
}

void smol_gl_set_vsync(int enabled) {
	smol_wglSwapIntervalEXT(enabled);
}

void smol_frame_destroy(smol_frame_t* frame) {

	if(frame->gl.context) {
		smol_wglMakeCurrent(GetDC(frame->frame_handle_win32), NULL);
		smol_wglDeleteContext(frame->gl.context);
	}

	SMOL_ASSERT(DestroyWindow(frame->frame_handle_win32));
	smol_event_queue_destroy(&frame->event_queue);
	SMOL_FREE(frame);
}

//Can be passed null, this will pump messages to every window (on windows at least)
void smol_frame_update(smol_frame_t* frame) {

	MSG msg;
	while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

}

int smol_frame_mapkey(WPARAM key, LPARAM ext);

int smol_frame_gl_swap_buffers(smol_frame_t* frame) {
	if(!frame->gl.context) return 0;
	return smol_SwapBuffers(GetDC(frame->frame_handle_win32));
}

void smol_frame_set_cursor_visibility(smol_frame_t* frame, int is_visible) {
	ShowCursor(is_visible);
}

LRESULT CALLBACK smol_frame_handle_event(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	if(msg == WM_CREATE) {
		CREATESTRUCT* createStruct = (CREATESTRUCT*)lParam;
		SetWindowLongPtr(wnd, GWLP_USERDATA, (LONG_PTR)createStruct->lpCreateParams);
		return 1;
	}

	smol_frame_t* frame = (smol_frame_t*)GetWindowLongPtr(wnd, GWLP_USERDATA);
	smol_frame_event_t event = { 0 };

	if(!frame || frame->frame_handle_win32 == NULL) 
		goto def_proc;

	event.frame = frame;

	switch(msg) {
		case WM_CLOSE: {
			event.type = SMOL_FRAME_EVENT_CLOSED;
			smol_event_queue_push_back(frame->event_queue, &event);
			frame->should_close = 1;
			return 1;
		} break;
		case WM_SIZE: {
			event.type = SMOL_FRAME_EVENT_RESIZE;
			event.size.width = LOWORD(lParam);
			event.size.height = HIWORD(lParam);
			frame->width = LOWORD(lParam);
			frame->height = HIWORD(lParam);
			smol_event_queue_push_back(frame->event_queue, &event);
			return 1;
		} break;
		case WM_MOUSEMOVE: {

			event.type = SMOL_FRAME_EVENT_MOUSE_MOVE;
			event.mouse.x = GET_X_LPARAM(lParam);
			event.mouse.y = GET_Y_LPARAM(lParam);
			event.mouse.dx = event.mouse.x - frame->old_mouse_x;
			event.mouse.dy = event.mouse.y - frame->old_mouse_y;
			smol_event_queue_push_back(frame->event_queue, &event);

			frame->old_mouse_x = event.mouse.x;
			frame->old_mouse_y = event.mouse.y;

			return 1;
		} break;
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_XBUTTONDOWN:
		{
			event.type = SMOL_FRAME_EVENT_MOUSE_BUTTON_DOWN;
			switch(msg) {
				case WM_LBUTTONDOWN: event.mouse.button = 1; break;
				case WM_RBUTTONDOWN: event.mouse.button = 2; break;
				case WM_MBUTTONDOWN: event.mouse.button = 3; break;
				case WM_XBUTTONDOWN: event.mouse.button = 4 + HIWORD(wParam); break;
			}

			event.mouse.x = GET_X_LPARAM(lParam);
			event.mouse.y = GET_Y_LPARAM(lParam);
			event.mouse.dx = event.mouse.x - frame->old_mouse_x;
			event.mouse.dy = event.mouse.y - frame->old_mouse_y;

			smol_event_queue_push_back(frame->event_queue, &event);
			frame->old_mouse_x = event.mouse.x;
			frame->old_mouse_y = event.mouse.y;

			return 1;
		} break;
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_XBUTTONUP:
		{
			event.type = SMOL_FRAME_EVENT_MOUSE_BUTTON_UP;
			
			switch(msg) {
				case WM_LBUTTONUP: event.mouse.button = 1; break;
				case WM_RBUTTONUP: event.mouse.button = 2; break;
				case WM_MBUTTONUP: event.mouse.button = 3; break;
				case WM_XBUTTONUP: event.mouse.button = 4 + HIWORD(wParam); break;
			}

			event.mouse.x = GET_X_LPARAM(lParam);
			event.mouse.y = GET_Y_LPARAM(lParam);
			smol_event_queue_push_back(frame->event_queue, &event);
			
			frame->old_mouse_x = event.mouse.x;
			frame->old_mouse_y = event.mouse.y;

			return 1;
		} break;
		case WM_MOUSEWHEEL: {
			event.type = SMOL_FRAME_EVENT_MOUSE_VER_WHEEL;
			event.mouse.x = GET_X_LPARAM(lParam);
			event.mouse.y = GET_Y_LPARAM(lParam);
			event.mouse.dx = event.mouse.x - frame->old_mouse_x;
			event.mouse.dy = event.mouse.y - frame->old_mouse_y;

			int delta = GET_WHEEL_DELTA_WPARAM(wParam);
			 
			event.mouse.dz = (delta > 0) ? 1 : -1;
			frame->mouse_z_accum += event.mouse.dz;
			event.mouse.z = frame->mouse_z_accum;

			smol_event_queue_push_back(frame->event_queue, &event);

			frame->old_mouse_x = event.mouse.x;
			frame->old_mouse_y = event.mouse.y;
			return 1;
		} break;
		#ifdef _MSC_VER
		case WM_MOUSEHWHEEL: {

			event.type = SMOL_FRAME_EVENT_MOUSE_HOR_WHEEL;
			event.mouse.x = GET_X_LPARAM(lParam);
			event.mouse.y = GET_Y_LPARAM(lParam);
			event.mouse.dx = event.mouse.x - frame->old_mouse_x;
			event.mouse.dy = event.mouse.y - frame->old_mouse_y;

			int delta = GET_WHEEL_DELTA_WPARAM(wParam);
			 
			event.mouse.dw = (delta > 0) ? 1 : -1;
			frame->mouse_w_accum += event.mouse.dw;
			event.mouse.w = frame->mouse_w_accum;

			smol_event_queue_push_back(frame->event_queue, &event);

			frame->old_mouse_x = event.mouse.x;
			frame->old_mouse_y = event.mouse.y;
			
			return 1;
		} break;
		#endif
		case WM_KEYDOWN:
		case WM_KEYUP: {

			int released = (msg == WM_KEYUP);

			event.type = released ? SMOL_FRAME_EVENT_KEY_UP : SMOL_FRAME_EVENT_KEY_DOWN;
			event.key.code = smol_frame_mapkey(wParam, lParam);

			smol_event_queue_push_back(frame->event_queue, &event);

			return 0;
		} break;
		case WM_CHAR: {

			unsigned int chr = (unsigned int)wParam;
			unsigned int surrogate = (unsigned int)(wParam >> 10);

			if(surrogate == 0xD800) {
				frame->high_utf16_surrogate = (wParam & 0x3FF);
				return 0;
			} else if(surrogate == 0xDC00) {
				chr = ((unsigned int)frame->high_utf16_surrogate << 10) | ((unsigned int)(wParam) & 0x3FF);
				frame->high_utf16_surrogate = 0;
			}
			
			event.type = SMOL_FRAME_EVENT_TEXT_INPUT;
			event.input.codepoint = chr;

			smol_event_queue_push_back(frame->event_queue, &event);

			return 0;
		} break;
		case WM_SETFOCUS: {

			event.type = SMOL_FRAME_EVENT_FOCUS_GAINED;
			smol_event_queue_push_back(frame->event_queue, &event);

			return 0;
		} break;
		case WM_KILLFOCUS: {

			event.type = SMOL_FRAME_EVENT_FOCUS_LOST;
			smol_event_queue_push_back(frame->event_queue, &event);

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

	char bytes[sizeof(BITMAPINFO) + 24] = { 0 };

	BITMAPINFO* bmi = (BITMAPINFO*)bytes;
	bmi->bmiHeader.biSize = sizeof(bmi->bmiHeader);
	bmi->bmiHeader.biWidth = width;
	bmi->bmiHeader.biHeight = -height;
	bmi->bmiHeader.biPlanes = 1;
	bmi->bmiHeader.biBitCount = 32;
	bmi->bmiHeader.biCompression = BI_BITFIELDS;
	*(((DWORD*)bmi->bmiColors)+0) = 0x000000FF;
	*(((DWORD*)bmi->bmiColors)+1) = 0x0000FF00;
	*(((DWORD*)bmi->bmiColors)+2) = 0x00FF0000;

	smol_StretchDIBits(GetDC(frame->frame_handle_win32), dstX, dstY, dstW, dstH, srcX, srcY, srcW, srcH, pixels, bmi, DIB_RGB_COLORS, SRCCOPY);
	
}

#endif 
#pragma endregion 

#pragma region Linux/X11 Implementation
#if defined(SMOL_PLATFORM_LINUX)


int smol_frame_mapkey(int key);

#	if defined(SMOL_FRAME_BACKEND_X11)

Atom smol__wm_delete_window_atom;
Atom smol__frame_handle_atom;
Cursor smol__frame_empty_cursor;
void* smol__X11_so;

#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
#endif

#ifndef GLX_VERSION_1_0
#define GLX_VERSION_1_0 1
typedef struct __GLXcontextRec *GLXContext;
typedef XID GLXDrawable;
typedef XID GLXPixmap;
#define GLX_EXTENSION_NAME "GLX"
#define GLX_PbufferClobber 0
#define GLX_BufferSwapComplete 1
#define __GLX_NUMBER_EVENTS 17
#define GLX_BAD_SCREEN 1
#define GLX_BAD_ATTRIBUTE 2
#define GLX_NO_EXTENSION 3
#define GLX_BAD_VISUAL 4
#define GLX_BAD_CONTEXT 5
#define GLX_BAD_VALUE 6
#define GLX_BAD_ENUM 7
#define GLX_USE_GL 1
#define GLX_BUFFER_SIZE 2
#define GLX_LEVEL 3
#define GLX_RGBA 4
#define GLX_DOUBLEBUFFER 5
#define GLX_STEREO 6
#define GLX_AUX_BUFFERS 7
#define GLX_RED_SIZE 8
#define GLX_GREEN_SIZE 9
#define GLX_BLUE_SIZE 10
#define GLX_ALPHA_SIZE 11
#define GLX_DEPTH_SIZE 12
#define GLX_STENCIL_SIZE 13
#define GLX_ACCUM_RED_SIZE 14
#define GLX_ACCUM_GREEN_SIZE 15
#define GLX_ACCUM_BLUE_SIZE 16
#define GLX_ACCUM_ALPHA_SIZE 17
typedef XVisualInfo* (APIENTRYP PFNGLXCHOOSEVISUALPROC)(Display* dpy, int screen, int * attribList);
typedef GLXContext (APIENTRYP PFNGLXCREATECONTEXTPROC)(Display* dpy, XVisualInfo* vis, GLXContext shareList, Bool direct);
typedef void (APIENTRYP PFNGLXDESTROYCONTEXTPROC)(Display* dpy, GLXContext ctx);
typedef Bool (APIENTRYP PFNGLXMAKECURRENTPROC)(Display* dpy, GLXDrawable drawable, GLXContext ctx);
typedef void (APIENTRYP PFNGLXCOPYCONTEXTPROC)(Display* dpy, GLXContext src, GLXContext dst, unsigned long mask);
typedef void (APIENTRYP PFNGLXSWAPBUFFERSPROC)(Display* dpy, GLXDrawable drawable);
typedef GLXPixmap (APIENTRYP PFNGLXCREATEGLXPIXMAPPROC)(Display* dpy, XVisualInfo* visual, Pixmap pixmap);
typedef void (APIENTRYP PFNGLXDESTROYGLXPIXMAPPROC)(Display* dpy, GLXPixmap pixmap);
typedef Bool (APIENTRYP PFNGLXQUERYEXTENSIONPROC)(Display* dpy, int * errorb, int * event);
typedef Bool (APIENTRYP PFNGLXQUERYVERSIONPROC)(Display* dpy, int * maj, int * min);
typedef Bool (APIENTRYP PFNGLXISDIRECTPROC)(Display* dpy, GLXContext ctx);
typedef int (APIENTRYP PFNGLXGETCONFIGPROC)(Display* dpy, XVisualInfo* visual, int attrib, int * value);
typedef GLXContext (APIENTRYP PFNGLXGETCURRENTCONTEXTPROC)(void);
typedef GLXDrawable (APIENTRYP PFNGLXGETCURRENTDRAWABLEPROC)(void);
typedef void (APIENTRYP PFNGLXWAITGLPROC)(void);
typedef void (APIENTRYP PFNGLXWAITXPROC)(void);
typedef void (APIENTRYP PFNGLXUSEXFONTPROC)(Font font, int first, int count, int list);
#endif

#ifndef GL_VERSION_1_0
#define GL_VERSION_1_0 1
typedef void GLvoid;
typedef unsigned int GLenum;
typedef float GLfloat;
typedef int GLint;
typedef int GLsizei;
typedef unsigned int GLbitfield;
typedef double GLdouble;
typedef unsigned int GLuint;
typedef unsigned char GLboolean;
typedef unsigned char GLubyte;
typedef char GLbyte;
typedef short GLshort;
typedef unsigned short GLushort;
#endif 

#ifndef GLX_VERSION_1_1
#define GLX_VERSION_1_1 1
#define GLX_VENDOR 0x1
#define GLX_VERSION 0x2
#define GLX_EXTENSIONS 0x3
typedef const char * (APIENTRYP PFNGLXQUERYEXTENSIONSSTRINGPROC)(Display* dpy, int screen);
typedef const char * (APIENTRYP PFNGLXQUERYSERVERSTRINGPROC)(Display* dpy, int screen, int name);
typedef const char * (APIENTRYP PFNGLXGETCLIENTSTRINGPROC)(Display* dpy, int name);
#endif 

#ifndef GLX_VERSION_1_2
#define GLX_VERSION_1_2 1
typedef Display* (APIENTRYP PFNGLXGETCURRENTDISPLAYPROC)(void);
#endif 

#ifndef GLX_VERSION_1_3
#define GLX_VERSION_1_3 1
typedef XID GLXContextID;
typedef struct __GLXFBConfigRec *GLXFBConfig;
typedef XID GLXWindow;
typedef XID GLXPbuffer;
#define GLX_WINDOW_BIT 0x00000001
#define GLX_PIXMAP_BIT 0x00000002
#define GLX_PBUFFER_BIT 0x00000004
#define GLX_RGBA_BIT 0x00000001
#define GLX_COLOR_INDEX_BIT 0x00000002
#define GLX_PBUFFER_CLOBBER_MASK 0x08000000
#define GLX_FRONT_LEFT_BUFFER_BIT 0x00000001
#define GLX_FRONT_RIGHT_BUFFER_BIT 0x00000002
#define GLX_BACK_LEFT_BUFFER_BIT 0x00000004
#define GLX_BACK_RIGHT_BUFFER_BIT 0x00000008
#define GLX_AUX_BUFFERS_BIT 0x00000010
#define GLX_DEPTH_BUFFER_BIT 0x00000020
#define GLX_STENCIL_BUFFER_BIT 0x00000040
#define GLX_ACCUM_BUFFER_BIT 0x00000080
#define GLX_CONFIG_CAVEAT 0x20
#define GLX_X_VISUAL_TYPE 0x22
#define GLX_TRANSPARENT_TYPE 0x23
#define GLX_TRANSPARENT_INDEX_VALUE 0x24
#define GLX_TRANSPARENT_RED_VALUE 0x25
#define GLX_TRANSPARENT_GREEN_VALUE 0x26
#define GLX_TRANSPARENT_BLUE_VALUE 0x27
#define GLX_TRANSPARENT_ALPHA_VALUE 0x28
#define GLX_DONT_CARE 0xFFFFFFFF
#define GLX_NONE 0x8000
#define GLX_SLOW_CONFIG 0x8001
#define GLX_TRUE_COLOR 0x8002
#define GLX_DIRECT_COLOR 0x8003
#define GLX_PSEUDO_COLOR 0x8004
#define GLX_STATIC_COLOR 0x8005
#define GLX_GRAY_SCALE 0x8006
#define GLX_STATIC_GRAY 0x8007
#define GLX_TRANSPARENT_RGB 0x8008
#define GLX_TRANSPARENT_INDEX 0x8009
#define GLX_VISUAL_ID 0x800B
#define GLX_SCREEN 0x800C
#define GLX_NON_CONFORMANT_CONFIG 0x800D
#define GLX_DRAWABLE_TYPE 0x8010
#define GLX_RENDER_TYPE 0x8011
#define GLX_X_RENDERABLE 0x8012
#define GLX_FBCONFIG_ID 0x8013
#define GLX_RGBA_TYPE 0x8014
#define GLX_COLOR_INDEX_TYPE 0x8015
#define GLX_MAX_PBUFFER_WIDTH 0x8016
#define GLX_MAX_PBUFFER_HEIGHT 0x8017
#define GLX_MAX_PBUFFER_PIXELS 0x8018
#define GLX_PRESERVED_CONTENTS 0x801B
#define GLX_LARGEST_PBUFFER 0x801C
#define GLX_WIDTH 0x801D
#define GLX_HEIGHT 0x801E
#define GLX_EVENT_MASK 0x801F
#define GLX_DAMAGED 0x8020
#define GLX_SAVED 0x8021
#define GLX_WINDOW 0x8022
#define GLX_PBUFFER 0x8023
#define GLX_PBUFFER_HEIGHT 0x8040
#define GLX_PBUFFER_WIDTH 0x8041
typedef GLXFBConfig * (APIENTRYP PFNGLXGETFBCONFIGSPROC)(Display* dpy, int screen, int * nelements);
typedef GLXFBConfig * (APIENTRYP PFNGLXCHOOSEFBCONFIGPROC)(Display* dpy, int screen, const int * attrib_list, int * nelements);
typedef int (APIENTRYP PFNGLXGETFBCONFIGATTRIBPROC)(Display* dpy, GLXFBConfig config, int attribute, int * value);
typedef XVisualInfo* (APIENTRYP PFNGLXGETVISUALFROMFBCONFIGPROC)(Display* dpy, GLXFBConfig config);
typedef GLXWindow (APIENTRYP PFNGLXCREATEWINDOWPROC)(Display* dpy, GLXFBConfig config, Window win, const int * attrib_list);
typedef void (APIENTRYP PFNGLXDESTROYWINDOWPROC)(Display* dpy, GLXWindow win);
typedef GLXPixmap (APIENTRYP PFNGLXCREATEPIXMAPPROC)(Display* dpy, GLXFBConfig config, Pixmap pixmap, const int * attrib_list);
typedef void (APIENTRYP PFNGLXDESTROYPIXMAPPROC)(Display* dpy, GLXPixmap pixmap);
typedef GLXPbuffer (APIENTRYP PFNGLXCREATEPBUFFERPROC)(Display* dpy, GLXFBConfig config, const int * attrib_list);
typedef void (APIENTRYP PFNGLXDESTROYPBUFFERPROC)(Display* dpy, GLXPbuffer pbuf);
typedef void (APIENTRYP PFNGLXQUERYDRAWABLEPROC)(Display* dpy, GLXDrawable draw, int attribute, unsigned int * value);
typedef GLXContext (APIENTRYP PFNGLXCREATENEWCONTEXTPROC)(Display* dpy, GLXFBConfig config, int render_type, GLXContext share_list, Bool direct);
typedef Bool (APIENTRYP PFNGLXMAKECONTEXTCURRENTPROC)(Display* dpy, GLXDrawable draw, GLXDrawable read, GLXContext ctx);
typedef GLXDrawable (APIENTRYP PFNGLXGETCURRENTREADDRAWABLEPROC)(void);
typedef int (APIENTRYP PFNGLXQUERYCONTEXTPROC)(Display* dpy, GLXContext ctx, int attribute, int * value);
typedef void (APIENTRYP PFNGLXSELECTEVENTPROC)(Display* dpy, GLXDrawable draw, unsigned long event_mask);
typedef void (APIENTRYP PFNGLXGETSELECTEDEVENTPROC)(Display* dpy, GLXDrawable draw, unsigned long * event_mask);
#endif

#ifndef GLX_VERSION_1_4
#define GLX_VERSION_1_4 1
typedef void (APIENTRY *__GLXextFuncPtr)(void);
#define GLX_SAMPLE_BUFFERS 100000
#define GLX_SAMPLES 100001
typedef __GLXextFuncPtr (APIENTRYP PFNGLXGETPROCADDRESSPROC)(const GLubyte * procName);
#endif 

#ifndef GLX_ARB_create_context
#define GLX_ARB_create_context
#define GLX_CONTEXT_DEBUG_BIT_ARB 0x00000001
#define GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x00000002
#define GLX_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB 0x2092
#define GLX_CONTEXT_FLAGS_ARB 0x2094
#endif 

#ifndef GLX_ARB_create_context_profile
#define GLX_ARB_create_context_profile 1
#define GLX_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#define GLX_CONTEXT_PROFILE_MASK_ARB 0x9126
#endif

typedef Display* smol_XOpenDisplay_proc(const char* display);
typedef int smol_XCloseDisplay_proc(Display* display);
typedef int smol_XGetWindowAttributes_proc(Display* display, Window parentWindow, XWindowAttributes* attirbutes);
typedef Window smol_XCreateWindow_proc(Display* display, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned int border_width, unsigned int depth, unsigned int window_class, Visual* visual,unsigned int value_mask, XSetWindowAttributes* XWindowAttributes);
typedef int smol_XDestroyWindow_proc(Display* display,Window w);
typedef int smol_XMapWindow_proc(Display* display, Window window);
typedef int smol_XFlush_proc(Display* display);
typedef void smol_XSetWMNormalHints_proc(Display* display, Window window, XSizeHints* hints);
typedef Atom smol_XInternAtom_proc(Display* display, _Xconst char* atom_name, Bool only_if_exists);
typedef Status smol_XSetWMProtocols_proc(Display* display, Window window, Atom* protocols, int count);
typedef int smol_XStoreName_proc(Display* display, Window window, _Xconst char* window_name);
typedef XIM smol_XOpenIM_proc(Display* display, struct _XrmHashBucketRec* rdb, char* res_name, char* res_class);
typedef XImage* smol_XCreateImage_proc(Display* display, Visual* visual, unsigned int depth, int format, int current_offset, char* data, unsigned int width, unsigned int height, int bitmap_pad, int bytes_per_line);
typedef int smol_XDestroyImage_proc(XImage* image);
typedef GC smol_XCreateGC_proc(Display* display, Drawable drawable, unsigned long valuemask, XGCValues* values);
typedef int smol_XFreeGC_proc(Display* display, GC gc);
typedef Status smol_XGetIMValues_proc(XIM im, ...) _X_SENTINEL(0);
typedef XIC smol_XCreateIC_proc(XIM im, ...) _X_SENTINEL(0);
typedef void smol_XSetICFocus_proc(XIC  ic);
typedef int smol_XChangeProperty_proc(Display* display, Window window, Atom property, Atom type, int format, int mode, _Xconst unsigned char* data, int nelements);
typedef void smol_XFree_proc(void* data);
typedef Colormap smol_XCreateColormap_proc(Display* display, Window w, Visual* visual, int alloc);
typedef int smol_XInstallColormap_proc(Display* display, Colormap colormap);
typedef void smol_XDestroyIC_proc(XIC ic);
typedef Status smol_XCloseIM_proc(XIM im);
typedef int smol_XNextEvent_proc(Display* display, XEvent* event_return);
typedef Bool smol_XFilterEvent_proc(XEvent* event, Window window);
typedef int smol_XPending_proc(Display* display);
typedef int smol_XPeekEvent_proc(Display* display, XEvent* event_return);
typedef KeySym smol_XLookupKeysym_proc(XKeyEvent* key_event, int index);
typedef int smol_Xutf8LookupString_proc(XIC ic,XKeyPressedEvent* event, char* buffer_return, int bytes_buffer, KeySym* keysym_return, Status* status_return);
typedef int smol_XPending_proc(Display* display);
typedef XSizeHints* smol_XAllocSizeHints_proc(void);
typedef int smol_XPutImage_proc(Display* display, Drawable d, GC gc, XImage* image, int src_x, int src_y, int dest_x, int dest_y, unsigned int width, unsigned int height);

typedef Pixmap smol_XCreatePixmap_proc(Display *display, Drawable d, unsigned int width, unsigned int height, unsigned int depth);
typedef void smol_XFreePixmap_proc(Display* display, Pixmap pixmap);
typedef Cursor smol_XCreatePixmapCursor_proc(Display *display, Pixmap source, Pixmap mask, XColor *foreground_color, XColor *background_color, unsigned int x, unsigned int y);
typedef void smol_XFreeCursor_proc(Display* display, Cursor cursor);
typedef void smol_XDefineCursor_proc(Display *display, Window w, Cursor cursor);
typedef void smol_XUndefineCursor_proc(Display *display, Window w);



static smol_XOpenDisplay_proc* smol_XOpenDisplay;
static smol_XCloseDisplay_proc* smol_XCloseDisplay;
static smol_XGetWindowAttributes_proc* smol_XGetWindowAttributes;
static smol_XCreateWindow_proc* smol_XCreateWindow;
static smol_XDestroyWindow_proc* smol_XDestroyWindow;
static smol_XMapWindow_proc* smol_XMapWindow;
static smol_XFlush_proc* smol_XFlush;
static smol_XSetWMNormalHints_proc* smol_XSetWMNormalHints;
static smol_XInternAtom_proc* smol_XInternAtom;
static smol_XSetWMProtocols_proc* smol_XSetWMProtocols;
static smol_XStoreName_proc* smol_XStoreName;
static smol_XOpenIM_proc* smol_XOpenIM;
static smol_XCreateImage_proc* smol_XCreateImage;
static smol_XDestroyImage_proc* smol_XDestroyImage;
static smol_XCreateGC_proc* smol_XCreateGC;
static smol_XFreeGC_proc* smol_XFreeGC;
static smol_XOpenDisplay_proc* smol_XOpenDisplay;
static smol_XGetWindowAttributes_proc* smol_XGetWindowAttributes;
static smol_XCreateWindow_proc* smol_XCreateWindow;
static smol_XAllocSizeHints_proc* smol_XAllocSizeHints;
static smol_XSetWMNormalHints_proc* smol_XSetWMNormalHints;
static smol_XInternAtom_proc* smol_XInternAtom;
static smol_XInternAtom_proc* smol_XInternAtom;
static smol_XSetWMProtocols_proc* smol_XSetWMProtocols;
static smol_XStoreName_proc* smol_XStoreName;
static smol_XMapWindow_proc* smol_XMapWindow;
static smol_XOpenIM_proc* smol_XOpenIM;
static smol_XGetIMValues_proc* smol_XGetIMValues;
static smol_XCreateIC_proc* smol_XCreateIC;
static smol_XSetICFocus_proc* smol_XSetICFocus;
static smol_XFlush_proc* smol_XFlush;
static smol_XChangeProperty_proc* smol_XChangeProperty;
static smol_XFree_proc* smol_XFree;
static smol_XCreateColormap_proc* smol_XCreateColormap;
static smol_XInstallColormap_proc* smol_XInstallColormap;
static smol_XDestroyWindow_proc* smol_XDestroyWindow;
static smol_XDestroyIC_proc* smol_XDestroyIC;
static smol_XCloseIM_proc* smol_XCloseIM;
static smol_XCloseDisplay_proc* smol_XCloseDisplay;
static smol_XNextEvent_proc* smol_XNextEvent;
static smol_XFilterEvent_proc* smol_XFilterEvent;
static smol_XPeekEvent_proc* smol_XPeekEvent;
static smol_XLookupKeysym_proc* smol_XLookupKeysym;
static smol_Xutf8LookupString_proc* smol_Xutf8LookupString;
static smol_XPending_proc* smol_XPending;
static smol_XPutImage_proc* smol_XPutImage;

static smol_XCreatePixmap_proc* smol_XCreatePixmap;
static smol_XFreePixmap_proc* smol_XFreePixmap;
static smol_XCreatePixmapCursor_proc* smol_XCreatePixmapCursor;
static smol_XFreeCursor_proc* smol_XFreeCursor;
static smol_XDefineCursor_proc* smol_XDefineCursor;
static smol_XUndefineCursor_proc* smol_XUndefineCursor;


typedef XVisualInfo* smol_glXGetVisualFromFBConfig_proc(Display* display, Window window);
typedef void smol_glXMakeCurrent_proc(Display* display, Window window, GLXContext context);
typedef XVisualInfo* smol_glXChooseFBConfig_proc(Display* display, int screen, const int* attribs, int* result);
typedef void* smol_glXGetProcAddress_proc(unsigned char* proc);
typedef int smol_glXGetFBConfigAttrib_proc(Display* display, GLXFBConfig config, int attribute, int* value);
typedef void smol_glXSwapBuffers_proc(Display* display, Window window);
typedef GLXContext smol_glXCreateContextAttribsARB_proc(Display *dpy, GLXFBConfig config,  GLXContext share_context, Bool direct, const int *attrib_list);
typedef void smol_glxSwapIntervalEXT_proc(int);

static smol_glXGetVisualFromFBConfig_proc* smol_glXGetVisualFromFBConfig = NULL;
static smol_glXMakeCurrent_proc* smol_glXMakeCurrent = NULL;
static smol_glXGetProcAddress_proc* smol_glXGetProcAddress = NULL;
static smol_glXChooseFBConfig_proc* smol_glXChooseFBConfig = NULL;
static smol_glXGetFBConfigAttrib_proc* smol_glXGetFBConfigAttrib = NULL;
static smol_glXSwapBuffers_proc* smol_glXSwapBuffers = NULL;
static smol_glXCreateContextAttribsARB_proc* smol_glXCreateContextAttribsARB = NULL;
static smol_glxSwapIntervalEXT_proc* smol_glXSwapIntervalEXT = NULL;

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
	renderer->image = smol_XCreateImage(
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

		smol_XDestroyImage(frame->renderer->image);
	

	} else {
		renderer->gc = smol_XCreateGC(frame->display_server_connection, frame->frame_window, 0, NULL);
	}

	return renderer;
}

smol_frame_t* smol_frame_create_advanced(const smol_frame_config_t* config) {

	//TODO: Window parenting

	if(!smol__X11_so) {
		if(!smol__X11_so) smol__X11_so = dlopen("libX11.so.6", RTLD_NOW);
		if(!smol__X11_so) smol__X11_so = dlopen("libX11.so.5", RTLD_NOW);
		if(!smol__X11_so) smol__X11_so = dlopen("libX11.so.4", RTLD_NOW);
		if(!smol__X11_so) smol__X11_so = dlopen("libX11.so.3", RTLD_NOW);
		if(!smol__X11_so) smol__X11_so = dlopen("libX11.so.2", RTLD_NOW);
		if(!smol__X11_so) smol__X11_so = dlopen("libX11.so.1", RTLD_NOW);
		if(!smol__X11_so) smol__X11_so = dlopen("libX11.so", RTLD_NOW);

		smol_XOpenDisplay = (smol_XOpenDisplay_proc*)dlsym(smol__X11_so, "XOpenDisplay");
		smol_XCloseDisplay = (smol_XCloseDisplay_proc*)dlsym(smol__X11_so, "XCloseDisplay");
		smol_XGetWindowAttributes = (smol_XGetWindowAttributes_proc*)dlsym(smol__X11_so, "XGetWindowAttributes");
		smol_XCreateWindow = (smol_XCreateWindow_proc*)dlsym(smol__X11_so, "XCreateWindow");
		smol_XDestroyWindow = (smol_XDestroyWindow_proc*)dlsym(smol__X11_so, "XDestroyWindow");
		smol_XMapWindow = (smol_XMapWindow_proc*)dlsym(smol__X11_so, "XMapWindow");
		smol_XFlush = (smol_XFlush_proc*)dlsym(smol__X11_so, "XFlush"); 
		smol_XSetWMNormalHints = (smol_XSetWMNormalHints_proc*)dlsym(smol__X11_so, "XSetWMNormalHints");
		smol_XInternAtom = (smol_XInternAtom_proc*)dlsym(smol__X11_so, "XInternAtom");
		smol_XSetWMProtocols = (smol_XSetWMProtocols_proc*)dlsym(smol__X11_so, "XSetWMProtocols");
		smol_XStoreName = (smol_XStoreName_proc*)dlsym(smol__X11_so, "XStoreName");
		smol_XOpenIM = (smol_XOpenIM_proc*)dlsym(smol__X11_so, "XOpenIM");
		smol_XCreateImage = (smol_XCreateImage_proc*)dlsym(smol__X11_so, "XCreateImage"); 
		smol_XDestroyImage = (smol_XDestroyImage_proc*)dlsym(smol__X11_so, "XDestroyImage");
		smol_XPutImage = (smol_XPutImage_proc*)dlsym(smol__X11_so, "XPutImage");
		smol_XCreateGC = (smol_XCreateGC_proc*)dlsym(smol__X11_so, "XCreateGC"); 
		smol_XFreeGC = (smol_XFreeGC_proc*)dlsym(smol__X11_so, "XFreeGC");
		smol_XOpenDisplay = (smol_XOpenDisplay_proc*)dlsym(smol__X11_so, "XOpenDisplay"); 
		smol_XGetWindowAttributes = (smol_XGetWindowAttributes_proc*)dlsym(smol__X11_so, "XGetWindowAttributes"); 
		smol_XCreateWindow = (smol_XCreateWindow_proc*)dlsym(smol__X11_so, "XCreateWindow"); 
		smol_XAllocSizeHints = (smol_XAllocSizeHints_proc*)dlsym(smol__X11_so, "XAllocSizeHints"); 
		smol_XInternAtom = (smol_XInternAtom_proc*)dlsym(smol__X11_so, "XInternAtom");
		smol_XSetWMProtocols = (smol_XSetWMProtocols_proc*)dlsym(smol__X11_so, "XSetWMProtocols"); 
		smol_XStoreName = (smol_XStoreName_proc*)dlsym(smol__X11_so, "XStoreName"); 
		smol_XMapWindow = (smol_XMapWindow_proc*)dlsym(smol__X11_so, "XMapWindow"); 
		smol_XOpenIM = (smol_XOpenIM_proc*)dlsym(smol__X11_so, "XOpenIM"); 
		smol_XGetIMValues = (smol_XGetIMValues_proc*)dlsym(smol__X11_so, "XGetIMValues"); 
		smol_XCreateIC = (smol_XCreateIC_proc*)dlsym(smol__X11_so, "XCreateIC"); 
		smol_XSetICFocus = (smol_XSetICFocus_proc*)dlsym(smol__X11_so, "XSetICFocus"); 
		smol_XFlush = (smol_XFlush_proc*)dlsym(smol__X11_so, "XFlush"); 
		smol_XChangeProperty = (smol_XChangeProperty_proc*)dlsym(smol__X11_so, "XChangeProperty"); 
		smol_XFree = (smol_XFree_proc*)dlsym(smol__X11_so, "XFree"); 
		smol_XCreateColormap = (smol_XCreateColormap_proc*)dlsym(smol__X11_so, "XCreateColormap"); 
		smol_XInstallColormap = (smol_XInstallColormap_proc*)dlsym(smol__X11_so, "XInstallColormap"); 
		smol_XStoreName = (smol_XStoreName_proc*)dlsym(smol__X11_so, "XStoreName"); 
		smol_XDestroyWindow = (smol_XDestroyWindow_proc*)dlsym(smol__X11_so, "XDestroyWindow"); 
		smol_XDestroyIC = (smol_XDestroyIC_proc*)dlsym(smol__X11_so, "XDestroyIC"); 
		smol_XCloseIM = (smol_XCloseIM_proc*)dlsym(smol__X11_so, "XCloseIM"); 
		smol_XCloseDisplay = (smol_XCloseDisplay_proc*)dlsym(smol__X11_so, "XCloseDisplay"); 
		smol_XNextEvent = (smol_XNextEvent_proc*)dlsym(smol__X11_so, "XNextEvent"); 
		smol_XFilterEvent = (smol_XFilterEvent_proc*)dlsym(smol__X11_so, "XFilterEvent"); 
		smol_XPeekEvent = (smol_XPeekEvent_proc*)dlsym(smol__X11_so, "XPeekEvent"); 
		smol_XLookupKeysym = (smol_XLookupKeysym_proc*)dlsym(smol__X11_so, "XLookupKeysym"); 
		smol_Xutf8LookupString = (smol_Xutf8LookupString_proc*)dlsym(smol__X11_so, "Xutf8LookupString"); 
		smol_XPending = (smol_XPending_proc*)dlsym(smol__X11_so, "XPending"); 
		
		smol_XCreatePixmap = (smol_XCreatePixmap_proc*)dlsym(smol__X11_so, "XCreatePixmap");
		smol_XFreePixmap = (smol_XFreePixmap_proc*)dlsym(smol__X11_so, "XFreePixmap");
		smol_XCreatePixmapCursor = (smol_XFreePixmap_proc*)dlsym(smol__X11_so, "XCreatePixmapCursor");
		smol_XFreeCursor = (smol_XFreeCursor_proc*)dlsym(smol__X11_so, "XFreeCursor");
		smol_XDefineCursor = (smol_XDefineCursor_proc*)dlsym(smol__X11_so, "XDefineCursor");
		smol_XUndefineCursor = (smol_XUndefineCursor_proc*)dlsym(smol__X11_so, "XUndefineCursor");

	}

	//XSetError
	
	Display* display = smol_XOpenDisplay(NULL);
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

	if(smol_XGetWindowAttributes(display, parentWindow, &attributes) == 0)
		return NULL;
	
	setAttributes.event_mask = (
		ExposureMask        | KeyPressMask      | 
		KeyReleaseMask      | ButtonPressMask   | 
		ButtonReleaseMask   | PointerMotionMask | 
		ButtonMotionMask    | FocusChangeMask
	);

	result_window = smol_XCreateWindow(
		display, 
		parentWindow, 
		(attributes.width + config->width) >> 1, 
		(attributes.height + config->height) >> 1, 
		config->width, 
		config->height, 
		0,
		CopyFromParent, 
		InputOutput,
		CopyFromParent, 
		CWEventMask | CWColormap, 
		&setAttributes
	);

	if(result_window == None)
		return NULL;


	if(!(config->flags & SMOL_FRAME_CONFIG_HAS_MAXIMIZE_BUTTON)) {

		XSizeHints* sizeHints = smol_XAllocSizeHints();
		sizeHints->flags = PMinSize | PMaxSize;
		sizeHints->min_width = sizeHints->max_width = config->width;
		sizeHints->min_height = sizeHints->max_height = config->height;
		smol_XSetWMNormalHints(display, result_window, sizeHints);

		setAttributes.override_redirect = True;
	}
	

	if(smol__wm_delete_window_atom == None) { 
		smol__wm_delete_window_atom = smol_XInternAtom(display, "WM_DELETE_WINDOW", False);
	}

	if(smol__frame_handle_atom == None) {
		smol__frame_handle_atom = smol_XInternAtom(display, "SMOL_FRAME_HANDLE", False);
	}

	status = smol_XSetWMProtocols(display, result_window, &smol__wm_delete_window_atom, 1);

	SMOL_ASSERT("Failed to set Window protocols!" && (status != 0));

/*

XChangeProperty(
      Display *display,
      Window w,
      Atom property, type,
      int format,
      int mode,
      unsigned char *data,
      int nelements)

*/


	smol_XStoreName(display, result_window, config->title);
	smol_XMapWindow(display, result_window);

	im = smol_XOpenIM(display, NULL, NULL, NULL);
	smol_XGetIMValues(im, XNQueryInputStyle, &styles, NULL);
	ic = smol_XCreateIC(im, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, result_window, NULL);

	smol_XSetICFocus(ic);
	smol_XFlush(display);

	result = SMOL_ALLOC_INSTANCE(smol_frame_t);
	result->display_server_connection = display;
	result->frame_window = result_window;
	result->event_queue = smol_event_queue_create(2048);
	result->renderer = NULL;
	result->width = config->width;
	result->height = config->height;
	result->ic = ic;
	result->im = im;
	result->renderer = NULL;

	smol_XChangeProperty(display, result_window, smol__frame_handle_atom, XA_STRING, 8, PropertyChangeMask, (const unsigned char*)&result, sizeof(smol_frame_t*));
	smol_XFlush(display);

	if(!(config->gl_spec)) {
		result->renderer = smol_renderer_create(result);
	} else {

		smol_frame_gl_spec_t* spec = config->gl_spec;
		
		int initialized = (smol_glXGetProcAddress && smol_glXGetVisualFromFBConfig && smol_glXChooseFBConfig && smol_glXSwapBuffers && smol_glXCreateContextAttribsARB);
		if(!initialized) {
			void* libgl = dlopen("libGL.so", RTLD_NOW);
			if(libgl == NULL) libgl = dlopen("libGL.so.1", RTLD_NOW);

			SMOL_ASSERT("Couldn't load libGL.so dynamically!" && libgl);

			smol_glXGetVisualFromFBConfig = (smol_glXGetVisualFromFBConfig_proc*)dlsym(libgl, "glXGetVisualFromFBConfig");
			smol_glXChooseFBConfig = (smol_glXChooseFBConfig_proc*)dlsym(libgl, "glXChooseFBConfig");
			smol_glXMakeCurrent = (smol_glXMakeCurrent_proc*)dlsym(libgl, "glXMakeCurrent");
			smol_glXGetFBConfigAttrib = (smol_glXGetFBConfigAttrib_proc*)dlsym(libgl, "glXGetFBConfigAttrib");
			smol_glXGetProcAddress = (smol_glXGetProcAddress_proc*)dlsym(libgl, "glXGetProcAddress");
		 	smol_glXSwapBuffers = (smol_glXSwapBuffers_proc*)dlsym(libgl, "glXSwapBuffers");
			
			smol_glXCreateContextAttribsARB = smol_glXGetProcAddress((unsigned char*)"glXCreateContextAttribsARB");
			smol_glXSwapIntervalEXT = smol_glXGetProcAddress((unsigned char*)"glXSwapIntervalEXT");

		}

		int attribs[64] = {
			GLX_X_RENDERABLE, 1,
			GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
			GLX_RENDER_TYPE, GLX_RGBA_BIT,
			GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
			GLX_DOUBLEBUFFER, 1, 
			GLX_RED_SIZE, spec->red_bits,
			GLX_GREEN_SIZE, spec->green_bits,
			GLX_BLUE_SIZE, spec->blue_bits,
			GLX_ALPHA_SIZE, spec->alpha_bits,
			GLX_DEPTH_SIZE, spec->depth_bits,
			GLX_STENCIL_SIZE, spec->stencil_bits,
			0, 0
		};

		/*
			GLX_SAMPLE_BUFFERS, spec->has_multi_sampling,
			GLX_SAMPLES, spec->num_multi_samples,
		*/

		GLXFBConfig* config = NULL;
		{
			int count = 0;
			GLXFBConfig* configs = smol_glXChooseFBConfig(display, DefaultScreen(display), attribs, &count);
			int bestFbc = -1; 
			int bestSampleCount = 999;
			SMOL_ASSERT("No configurations!" && count);

			for(int i = 0; i < count; i++) {
				XVisualInfo* vi = smol_glXGetVisualFromFBConfig(display, configs[i]);
				if(vi) {
					int sampleBuffers, num_samples;
					smol_glXGetFBConfigAttrib(display, configs[i], GLX_SAMPLE_BUFFERS, &sampleBuffers);
					smol_glXGetFBConfigAttrib(display, configs[i], GLX_SAMPLES, &num_samples);
					
					int diff = (spec->num_multi_samples - num_samples);
					if(diff < 0) diff = -diff;

					if(sampleBuffers && diff < bestSampleCount) {
						bestFbc = i;
						bestSampleCount = num_samples;
					}
				}
				smol_XFree(vi);
			}

			if(bestFbc > 0) {
				attribs[22] = GLX_SAMPLE_BUFFERS;
				attribs[23] = 1;
				attribs[24] = GLX_SAMPLES;
				attribs[25] = bestSampleCount;
			}

			config = configs[bestFbc];
			smol_XFree(configs);
		}

		XVisualInfo* visualinfo = smol_glXGetVisualFromFBConfig(display, config);

		Window root_window = RootWindow(display, visualinfo->screen);
		Colormap color_map = smol_XCreateColormap(display, root_window, visualinfo->visual, AllocNone);

		int num_colormaps = smol_XInstallColormap(display, color_map);

		smol_XFlush(display);

		int context_attribs[] = {
			GLX_CONTEXT_MAJOR_VERSION_ARB, spec->major_version,
			GLX_CONTEXT_MINOR_VERSION_ARB, spec->minor_version,
			GLX_CONTEXT_FLAGS_ARB, 
				(spec->is_debug ? GLX_CONTEXT_DEBUG_BIT_ARB : 0) |
				(spec->is_forward_compatible ? GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB : 0),
			GLX_CONTEXT_PROFILE_MASK_ARB, spec->is_backward_compatible ? 
				GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB : 
				GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
			0, 0
		};

		/*
		GLXContext glXCreateContextAttribsARB(Display *dpy, GLXFBConfig config, GLXContext share_context, Bool direct, const int *attrib_list);
		*/
		result->gl.context = smol_glXCreateContextAttribsARB(display, config, NULL, True, context_attribs);
		smol_glXMakeCurrent(result->display_server_connection, result->frame_window, result->gl.context);

	}

	if(!smol__frame_empty_cursor) {
	
		Pixmap blank = smol_XCreatePixmap(display, DefaultRootWindow(display), 1, 1, 1);
		XColor dummy = { 0 };
		smol__frame_empty_cursor = smol_XCreatePixmapCursor(display, blank, blank, &dummy, &dummy, 0, 0);

	}
	//result->window_attributes = setAttributes;
	//result->root_window = rootWindow;


	return result;
}

void smol_frame_set_title(smol_frame_t* frame, const char* title) {
	smol_XStoreName(frame->display_server_connection, frame->frame_window, title);
	smol_XFlush(frame->display_server_connection);
}

int smol_frame_gl_swap_buffers(smol_frame_t* frame) {
	if(!frame->gl.context) return 0;
	smol_glXSwapBuffers(frame->display_server_connection, frame->frame_window);
	return 1;
}

void* smol_gl_load_symbol(const char* func) {
	if(!smol_glXGetProcAddress) {
		fputs("OpenGL context not initialized!", stderr);
		return NULL;
	}
	return smol_glXGetProcAddress(func);
}

void smol_gl_set_vsync(int enabled) {
	if(!smol_glXSwapIntervalEXT) {
		fputs("OpenGL context not initialized!", stderr);
		return;
	}
	smol_glXSwapIntervalEXT(enabled);	
}

void smol_frame_destroy(smol_frame_t* frame) {

	smol_XDestroyWindow(frame->display_server_connection, frame->frame_window);
	smol_XDestroyIC(frame->ic);
	smol_XCloseIM(frame->im);
	smol_XCloseDisplay(frame->display_server_connection);

	smol_event_queue_destroy(&frame->event_queue);
	if(frame->renderer) smol_renderer_destroy(frame->renderer);
	SMOL_FREE(frame);

}

void smol_frame_set_cursor_visibility(smol_frame_t* frame, int visibility) {
	if(visibility == SMOL_FALSE) {
		smol_XDefineCursor(frame->display_server_connection, frame->frame_window, smol__frame_empty_cursor);
	} else {
		smol_XUndefineCursor(frame->display_server_connection, frame->frame_window);
	}
	smol_XFlush(frame->display_server_connection);
}

//Can be passed null, this will pump messages to every window (on windows at least)
void smol_frame_update(smol_frame_t* frame) {

	int button_indices[] = {0, 1, 3, 2, 4, 5};

	XEvent xevent;
	smol_frame_event_t event = { 0 };

	while(smol_XPending(frame->display_server_connection)) {
		smol_XNextEvent(frame->display_server_connection, &xevent);

		Atom actual_type;
		int actual_format;
		unsigned long num_items;
		unsigned long bytes_after;

		smol_frame_t* frame_ptr;
		//XGetWindowProperty(
		//	frame->display_server_connection, 
		//	xevent.xany.window, 
		//	smol__frame_handle_atom, 
		//	0, 
		//	8, 
		//	False, 
		//	AnyPropertyType,  
		//	&actual_type, 
		//	&actual_format, 
		//	&num_items, 
		//	&bytes_after, 
		//	(unsigned char**)&frame_ptr
		//);

		//SMOL_ASSERT(actual_type == XA_STRING && actual_format == 8);

		smol_XFilterEvent(&xevent, frame->frame_window);
		switch(xevent.type) {
			case Expose: {
				//mWidth = attribs.width;
				//mHeight = attribs.height;
				event.type = SMOL_FRAME_EVENT_RESIZE;
				event.size.width = xevent.xexpose.width;
				event.size.height = xevent.xexpose.height;

				if(frame->width != xevent.xexpose.width && frame->height != xevent.xexpose.height && frame->renderer) {
					frame->renderer = smol_renderer_create(frame);
				} 

				frame->width = xevent.xexpose.width;
				frame->height = xevent.xexpose.height;

				smol_event_queue_push_back(frame->event_queue, &event);

			} break;
			case ClientMessage: {
				//if(xevent.xclient.type == smol__wm_protocols_atom) {
					//puts("Should quit.");
				if(xevent.xclient.data.l[0] == smol__wm_delete_window_atom) {
					event.type = SMOL_FRAME_EVENT_CLOSED;
					frame->should_close = 1;
					smol_event_queue_push_back(frame->event_queue, &event);
				}
				//}
				
			} break;
			case KeyPress:
			case KeyRelease: 
			{

				int physical = 1;
				if(xevent.type == KeyRelease) {
					XEvent next;
					if (smol_XPending(frame->display_server_connection)) {
						smol_XPeekEvent(frame->display_server_connection, &next);
						if(next.type == KeyPress && next.xkey.time == xevent.xkey.time && next.xkey.keycode == xevent.xkey.keycode) 
							physical = 0;
					}
				}

				//This break could be within the code block above.
				if(!physical)
					break;

				event.type = ((xevent.type == KeyRelease) && physical) ? SMOL_FRAME_EVENT_KEY_UP: SMOL_FRAME_EVENT_KEY_DOWN;

				KeySym keysym = smol_XLookupKeysym(&xevent.xkey, 0);
				event.key.code = smol_frame_mapkey(keysym);
				smol_event_queue_push_back(frame->event_queue, &event);

				if(event.type == SMOL_FRAME_EVENT_KEY_DOWN) {
					char buffer[6] = {0};
					Status status = 0;
					int count = smol_Xutf8LookupString(frame->ic, &xevent.xkey, buffer, 6, &keysym, &status);
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
						event.input.codepoint = unicode;
						smol_event_queue_push_back(frame->event_queue, &event);
					}
				}

			} 
			break;
			case MotionNotify: {
				event.type = SMOL_FRAME_EVENT_MOUSE_MOVE;
				event.mouse.dx = xevent.xmotion.x - frame->old_mouse_x;
				event.mouse.dy = xevent.xmotion.y - frame->old_mouse_y;


				event.mouse.x = xevent.xmotion.x;
				event.mouse.y = xevent.xmotion.y;
				frame->old_mouse_x = event.mouse.x;
				frame->old_mouse_y = event.mouse.y;
				smol_event_queue_push_back(frame->event_queue, &event);
			} break;
			case ButtonPress:
			case ButtonRelease: 
			{
				if(xevent.xbutton.button < 4) {
					event.type = xevent.type == ButtonPress ? SMOL_FRAME_EVENT_MOUSE_BUTTON_DOWN : SMOL_FRAME_EVENT_MOUSE_BUTTON_UP;
					event.mouse.button = button_indices[xevent.xbutton.button];

					event.mouse.dx = xevent.xmotion.x - frame->old_mouse_x;
					event.mouse.dy = xevent.xmotion.y - frame->old_mouse_y;

					event.mouse.x = xevent.xmotion.x;
					event.mouse.y = xevent.xmotion.y;

					smol_event_queue_push_back(frame->event_queue, &event);
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

					smol_event_queue_push_back(frame->event_queue, &event);

				}
			} 
			break;
			case FocusIn: {
				event.type = SMOL_FRAME_EVENT_FOCUS_GAINED;
				smol_event_queue_push_back(frame->event_queue, &event);
			} break;
			case FocusOut: {
				event.type = SMOL_FRAME_EVENT_FOCUS_LOST;
				smol_event_queue_push_back(frame->event_queue, &event);
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

	Visual *visual = DefaultVisual(frame->display_server_connection, DefaultScreen(frame->display_server_connection));

	unsigned int red_mul = ((~(visual->red_mask))+1) & visual->red_mask;
	unsigned int green_mul = ((~(visual->green_mask))+1) & visual->green_mask;
	unsigned int blue_mul = ((~(visual->blue_mask))+1) & visual->blue_mask;
	

	for(int y = startY; y < endY; y++) {
		int sY = srcY + ((srcH * y) / dstH);
		int dY = dstY + y;
		for(int x = startX; x < endX; x++) {
			int sX = srcX + ((srcW * x) / dstW);
			int dX = dstX + x;
			unsigned int pixel = pixels[sX + sY * width];
			renderer->pixel_data[dX + dY * renderer->width] = 
				(((pixel >> 0x00) & 0xFF) * red_mul) |
				(((pixel >> 0x08) & 0xFF) * green_mul) |
				(((pixel >> 0x10) & 0xFF) * blue_mul) 
			;
		}
	}

	smol_XPutImage(frame->display_server_connection, frame->frame_window, renderer->gc, renderer->image, 0, 0, 0, 0, renderer->width, renderer->height);
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
static xcb_atom_t smol__wm_delete_window_atom;
static xcb_atom_t smol__wm_protocols_atom;
static int smol__num_frames;
static xcb_key_symbols_t* smol__keysyms;

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

const char* egl_error(EGLint error) {
	switch(error) {
		case EGL_SUCCESS:             return "EGL_SUCCESS";
		case EGL_NOT_INITIALIZED:     return "EGL_NOT_INITIALIZED";
		case EGL_BAD_ACCESS:          return "EGL_BAD_ACCESS";
		case EGL_BAD_ALLOC:           return "EGL_BAD_ALLOC";
		case EGL_BAD_ATTRIBUTE:       return "EGL_BAD_ATTRIBUTE";
		case EGL_BAD_CONTEXT:         return "EGL_BAD_CONTEXT";
		case EGL_BAD_CONFIG:          return "EGL_BAD_CONFIG";
		case EGL_BAD_CURRENT_SURFACE: return "EGL_BAD_CURRENT_SURFACE";
		case EGL_BAD_DISPLAY:         return "EGL_BAD_DISPLAY";
		case EGL_BAD_SURFACE:         return "EGL_BAD_SURFACE";
		case EGL_BAD_MATCH:           return "EGL_BAD_MATCH";
		case EGL_BAD_PARAMETER:       return "EGL_BAD_PARAMETER";
		case EGL_BAD_NATIVE_PIXMAP:   return "EGL_BAD_NATIVE_PIXMAP";
		case EGL_BAD_NATIVE_WINDOW:   return "EGL_BAD_NATIVE_WINDOW";
		case EGL_CONTEXT_LOST: 		  return "EGL_CONTEXT_LOST";
	}
	return "";
}

#ifdef __egl_h_
void eg_debug_proc(EGLenum error, const char *command, EGLint messageType, EGLLabelKHR threadLabel, EGLLabelKHR objectLabel, const char* message) {
	if(
		(messageType == EGL_DEBUG_MSG_CRITICAL_KHR) || 
		(messageType == EGL_DEBUG_MSG_ERROR_KHR)
	) {
		puts((messageType == EGL_DEBUG_MSG_CRITICAL_KHR) ? "CRITICAL EGL ERROR!" : "NON-CRITICAL EGL ERROR!");
		printf("EGL error: %s\nCommand: %s\nMessage: %s\n", egl_error(error), command, message);
		SMOL_BREAKPOINT();
	}
}
#endif 

smol_frame_t* smol_frame_create_advanced(const smol_frame_config_t* config) {

	int screen_id;
	xcb_connection_t* connection = xcb_connect(NULL, &screen_id);

	int error = xcb_connection_has_error(connection);
	SMOL_ASSERT("Couldn't form a connection to X-server!" && (error == 0));
	if(error) {
		return NULL;
	}

	const xcb_setup_t* setup = xcb_get_setup(connection);

	xcb_screen_t* screen = xcb_setup_roots_iterator(setup).data;

	xcb_window_t window = xcb_generate_id(connection);

	xcb_visualid_t default_visual_id = screen->root_visual;
    xcb_visualtype_t *visual = NULL;
    xcb_depth_iterator_t depth_iter = xcb_screen_allowed_depths_iterator(screen);
    for (; depth_iter.rem; xcb_depth_next(&depth_iter)) {
        xcb_visualtype_iterator_t visual_iter = xcb_depth_visuals_iterator(depth_iter.data);
        for (; visual_iter.rem; xcb_visualtype_next(&visual_iter)) {
            if (visual_iter.data->visual_id == default_visual_id) {
                visual = visual_iter.data;
                break;
            }
        }
        if (visual) {
            break;
        }
    }

	struct xkb_context* kbcontext;
	struct xkb_keymap* kbkeymap;
	struct xkb_state* kbstate;

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

	uint32_t colormap = 0;

	uint32_t values[] = {event_mask, screen->default_colormap, 0};

	{
		xcb_void_cookie_t cookie = xcb_create_window(
			connection, 
			XCB_COPY_FROM_PARENT, 
			window, 
			screen->root, 
			(screen->width_in_pixels + config->width) >> 1, 
			(screen->height_in_pixels + config->height) >> 1, 
			config->width, 
			config->height,
			0, 
			XCB_WINDOW_CLASS_INPUT_OUTPUT, 
			XCB_COPY_FROM_PARENT, 
			XCB_CW_EVENT_MASK | XCB_CW_COLORMAP, 
			values
		);

		xcb_generic_error_t* error = xcb_request_check(connection, cookie);
		if(error) {
			printf("Error code: %d\n", error->error_code);
		}
	}

	if(!(config->flags & SMOL_FRAME_CONFIG_HAS_MAXIMIZE_BUTTON)) {
		xcb_size_hints_t size_hints = {0};
		xcb_icccm_size_hints_set_min_size(&size_hints, config->width, config->height);
		xcb_icccm_size_hints_set_max_size(&size_hints, config->width, config->height);

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
		strlen(config->title), 
		(const void*)config->title
	);

	xcb_map_window(connection, window);
	xcb_flush(connection);

	{
		uint16_t major, minor;
		uint8_t base_event;
		uint8_t error;

		int res = xkb_x11_setup_xkb_extension(
			connection, 
			XCB_XKB_MAJOR_VERSION, 
			XCB_XKB_MINOR_VERSION,
			XKB_X11_SETUP_XKB_EXTENSION_NO_FLAGS,
			&major,
			&minor, 
			&base_event,
			&error
		);

		SMOL_ASSERT("Failed to setup xkb extension for x11!" && res);
	}


	int device_id = xkb_x11_get_core_keyboard_device_id(connection);

	SMOL_ASSERT("Invalid device id!" && device_id >= 0);

	//TODO: Error checking
	kbcontext = xkb_context_new(0);

	kbkeymap = xkb_x11_keymap_new_from_device(kbcontext, connection, device_id, XKB_KEYMAP_COMPILE_NO_FLAGS);

	kbstate = xkb_x11_state_new_from_device(kbkeymap, connection, device_id);

	smol_frame_t* result = SMOL_ALLOC_INSTANCE(smol_frame_t);
	memset(result, 0, sizeof(smol_frame_t));
	result->display_server_connection = connection;
	result->event_queue = smol_event_queue_create(2048);
	result->width = config->width;
	result->height = config->height;
	result->screen = screen;
	result->visual = visual;
	result->frame_window = window;
	result->kbcontext = kbcontext;
	result->kbkeymap = kbkeymap;
	result->kbstate = kbstate;
	result->renderer = NULL;

	if(smol__num_frames == 0) {
		smol__keysyms = xcb_key_symbols_alloc(connection);
	}
	smol__num_frames++;

	if(!config->gl_spec) {
		result->renderer = smol_renderer_create(result);
	} else {

		smol_frame_gl_spec_t* spec = config->gl_spec;

		const char* exts = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
		const char* has_egl_ext_platform_xcb = strstr(exts, "EGL_EXT_platform_xcb");
		const char* has_egl_khr_debug = strstr(exts, "EGL_KHR_debug");
		
		/*{
			for(
				char* cur = exts,* next = NULL; 
				cur && (next  = strchr(cur, ' ')); 
				cur = next+1
			) {
				printf("%.*s\n", (int)(next - cur), cur);
			}
		}*/
		
		if(!has_egl_khr_debug) {
			puts("No extension present 'EGL_KHR_debug'!");
		}

		if(!has_egl_ext_platform_xcb) {
			puts("No extension present 'EGL_EXT_platform_xcb'!");
		}


/*
#define EGL_ASSERT \
		{ \
			EGLint error = eglGetError(); \
			if(error != EGL_SUCCESS) {  \
				printf("%s\n", egl_error(error)); \
				SMOL_ASSERT(0); \
			} \
		}; (void)0
*/
#define EGL_ASSERT

		PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT = (PFNEGLGETPLATFORMDISPLAYEXTPROC)eglGetProcAddress("eglGetPlatformDisplayEXT");
		PFNEGLDEBUGMESSAGECONTROLKHRPROC eglDebugMessageControlKHR = (PFNEGLDEBUGMESSAGECONTROLKHRPROC)eglGetProcAddress("eglDebugMessageControlKHR");
		
		eglDebugMessageControlKHR(eg_debug_proc, NULL);
		
		EGLDisplay egl_display = NULL;

		EGLint args[] = {
			EGL_PLATFORM_XCB_SCREEN_EXT,
			screen_id,
			EGL_NONE
		};
		
		egl_display = eglGetPlatformDisplayEXT(EGL_PLATFORM_XCB_EXT, connection, args);
		//egl_display = eglGetDisplay((EGLNativeDisplayType)connection);
		//egl_display = eglGetDisplay((EGLNativeDisplayType)NULL);
		EGL_ASSERT;

		EGLint major, minor;
		eglInitialize(egl_display, &major, &minor);
		EGL_ASSERT;
		
		printf("EGL version: %d.%d\n", major, minor);
		//EGLDisplay egl_display = eglGetPlatformDisplayEXT((EGLNativeDisplayType)connection);

		EGLConfig egl_configs[128] = { 0 };
		EGLint num_configs = 0;
		EGLint attributes[64] = {
			EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
			EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
			EGL_RED_SIZE, spec->red_bits,
			EGL_GREEN_SIZE, spec->green_bits,
			EGL_BLUE_SIZE, spec->blue_bits,
			EGL_ALPHA_SIZE, spec->alpha_bits,
			EGL_DEPTH_SIZE, spec->depth_bits,
			EGL_STENCIL_SIZE, spec->stencil_bits,
			EGL_NONE
		};

		if(!eglChooseConfig(egl_display, attributes, egl_configs, 128, &num_configs)) {
			printf("Failed to get configs!");
		}

		int best_config = 0;
		int max_diff = 999;
		{
			for(int i = 0; i < num_configs; i++) {
				EGLConfig egl_config = egl_configs[i];
				EGLint has_sample_buffers;
				eglGetConfigAttrib(egl_display, egl_config, EGL_SAMPLE_BUFFERS, &has_sample_buffers);
				if(has_sample_buffers) {
					EGLint num_samples;
					eglGetConfigAttrib(egl_display, egl_config, EGL_SAMPLES, &num_samples);

					int diff = spec->num_multi_samples - num_samples;
					if(diff < 0) diff = -diff;

					if(diff < max_diff) {
						best_config = i;
						max_diff = diff;
					}

				}
			}
		}

		xcb_visualid_t visual_id;
		eglGetConfigAttrib(egl_display, egl_configs[best_config], EGL_NATIVE_VISUAL_ID, (EGLint *)&visual_id);
		
		int color_map = xcb_generate_id(connection);

		xcb_void_cookie_t cookie;

		cookie = xcb_create_colormap_checked(connection, XCB_COLORMAP_ALLOC_NONE, color_map, screen->root, visual_id);
		{
			xcb_generic_error_t* error = xcb_request_check(connection, cookie);
			if(error) {
				printf("XCB error!\n\tError Code: %d\n\tMajor: %d\n\tMinor: %d\n", error->error_code, error->major_code, error -> minor_code);
			}
		}

		cookie = xcb_install_colormap(connection, color_map);
		{
			xcb_generic_error_t* error = xcb_request_check(connection, cookie);
			if(error) {
				printf("XCB error!\n Error Code: %d\nMajor: %d\nMinor: %d\n", error->error_code, error->major_code, error -> minor_code);
			}
		}

		EGLBoolean bind_result = eglBindAPI(EGL_OPENGL_API);
		if(bind_result != EGL_TRUE) {
			printf("API binding failed!\n");
		}
		EGL_ASSERT;

		EGLint context_attributes[] = {
			EGL_CONTEXT_MAJOR_VERSION, spec->major_version,
			EGL_CONTEXT_MINOR_VERSION, spec->minor_version,
			EGL_CONTEXT_OPENGL_PROFILE_MASK, spec->is_backward_compatible ?
				EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT : 
				EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
			EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE, spec->is_forward_compatible ? 
				EGL_TRUE : 
				EGL_FALSE,
			EGL_CONTEXT_OPENGL_DEBUG, spec->is_debug ? 
				EGL_TRUE : 
				EGL_FALSE,
			EGL_NONE
		};

	
		EGLContext egl_context = eglCreateContext(egl_display, egl_configs[best_config], EGL_NO_CONTEXT, context_attributes);
		EGL_ASSERT;
	
		EGLSurface egl_surface = eglCreatePlatformWindowSurface(egl_display, egl_configs[best_config], &window, NULL);
		EGL_ASSERT;


		eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);


		result->gl.display = egl_display;
		result->gl.context = egl_context;
		result->gl.surface = egl_surface;
	}
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

void smol_frame_set_cursor_visibility(smol_frame_t* frame, int visibility) {
	(void)frame;
	(void)visibility;
}

int smol_frame_gl_swap_buffers(smol_frame_t* frame) {
	if(!frame) return 0;
	return eglSwapBuffers(frame->gl.display, frame->gl.surface);
}

void smol_frame_destroy(smol_frame_t* frame) {
	
	xkb_state_unref(frame->kbstate);
	xkb_keymap_unref(frame->kbkeymap);
	xkb_context_unref(frame->kbcontext);

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

		uint8_t response = xevent->response_type & 0x7F;
		smol_frame_event_t event = { 0 };

		switch(response) {
			case XCB_EXPOSE: {

				xcb_expose_event_t* ev = (xcb_expose_event_t*)xevent;

				if(frame->width != ev->width && frame->height != ev->height) {
					frame->renderer = smol_renderer_create(frame);
					frame->width = ev->width;
					frame->height = ev->height;

					event.type = SMOL_FRAME_EVENT_RESIZE;
					event.size.width = ev->width;
					event.size.height = ev->height;
					smol_event_queue_push_back(frame->event_queue, &event);
				} 

				xcb_flush(frame->display_server_connection);

			} break;
			case XCB_CLIENT_MESSAGE: {
				xcb_client_message_event_t* ev = (xcb_client_message_event_t*)xevent; 
				if(ev->data.data32[0] == smol__wm_delete_window_atom) {
					
					frame->should_close = 1;

					event.type = SMOL_FRAME_EVENT_CLOSED;
					smol_event_queue_push_back(frame->event_queue, &event);
				}
			} break;
			case XCB_KEY_PRESS: 
			case XCB_KEY_RELEASE: 
			{

				int physical = 1;
				xcb_key_press_event_t* ev = (xcb_key_press_event_t*)xevent;

				xkb_state_update_key(frame->kbstate, ev->detail, response == XCB_KEY_PRESS ? XKB_KEY_DOWN : XKB_KEY_UP);

				if(response == XCB_KEY_PRESS) {
					xcb_generic_event_t* next = xcb_poll_for_queued_event(frame->display_server_connection);
					if(next && (next->response_type & 0x7F) == XCB_KEY_RELEASE) {
						xcb_key_press_event_t* nev = (xcb_key_press_event_t*)next;
						if(nev->response_type == XCB_KEY_RELEASE && ev->time == nev->time && ev->detail == nev->detail) {
							physical = 0;
						}
					}
				}

				if(!physical)
					break;

				event.type = ((xevent->response_type & 0x7F) == XCB_KEY_PRESS && physical) ? SMOL_FRAME_EVENT_KEY_DOWN : SMOL_FRAME_EVENT_KEY_UP;
				xcb_keysym_t keysym = xcb_key_symbols_get_keysym(smol__keysyms, ev->detail, 0);
				event.key.code = smol_frame_mapkey(keysym);
				smol_event_queue_push_back(frame->event_queue, &event);

				
				if(event.type == SMOL_FRAME_EVENT_KEY_DOWN) {
					event.type = SMOL_FRAME_EVENT_TEXT_INPUT;
					event.input.codepoint  = xkb_state_key_get_utf32(frame->kbstate, ev->detail);
					smol_event_queue_push_back(frame->event_queue, &event);
				}

				
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

				smol_event_queue_push_back(frame->event_queue, &event);
			} break;
			case XCB_BUTTON_PRESS: 
			case XCB_BUTTON_RELEASE:
			{
				xcb_button_press_event_t* ev = (xcb_button_press_event_t*)xevent;

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


				smol_event_queue_push_back(frame->event_queue, &event);

			} break;
			case XCB_FOCUS_IN: {
				event.type = SMOL_FRAME_EVENT_FOCUS_GAINED;
				smol_event_queue_push_back(frame->event_queue, &event);
			} break;
			case XCB_FOCUS_OUT: {
				event.type = SMOL_FRAME_EVENT_FOCUS_LOST;
				smol_event_queue_push_back(frame->event_queue, &event);
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

	xcb_visualtype_t* visual = frame->visual;
	unsigned int red_mul = ((~(visual->red_mask))+1) & visual->red_mask;
	unsigned int green_mul = ((~(visual->green_mask))+1) & visual->green_mask;
	unsigned int blue_mul = ((~(visual->blue_mask))+1) & visual->blue_mask;
	

	for(int y = startY; y < endY; y++) {
		int sY = srcY + ((srcH * y) / dstH);
		int dY = dstY + y;
		for(int x = startX; x < endX; x++) {
			int sX = srcX + ((srcW * x) / dstW);
			int dX = dstX + x;
			unsigned int pixel = pixels[sX + sY * width];
			renderer->pixel_data[dX + dY * renderer->width] = 
				(((pixel >> 0x00) & 0xFF) * red_mul) |
				(((pixel >> 0x08) & 0xFF) * green_mul) |
				(((pixel >> 0x10) & 0xFF) * blue_mul) 
			;
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

#ifdef SMOL_PLATFORM_WEB

EM_BOOL smol_mouse_event(int event_type, const EmscriptenMouseEvent* mouse_event, void* user_data);
EM_BOOL smol_wheel_event(int event_type, const EmscriptenWheelEvent* event, void* user_data);
EM_BOOL smol_key_event(int event_type, const EmscriptenKeyboardEvent *key_event, void *user_data);
EM_BOOL smol_focus_event(int event_type, const EmscriptenFocusEvent* focus_event, void* user_data);
EM_BOOL smol_fullscreen_event(int event_type, const EmscriptenFullscreenChangeEvent* fullscreen_event, void* user_data);
//smol_key smol_frame_map_key(unsigned int vk, unsigned int location);
//smol_key smol_frame_map_keycode(EM_UTF8 code[32]);

EM_JS(void, smol_add_key, (const char* name, int value), {
	const name_str = UTF8ToString(name);
	Module.keymap[name_str] = value;
});

EM_JS(smol_key, smol_frame_map_keycode, (const char* code), {
	const name_str = UTF8ToString(code);
	const value = Module.keymap[name_str];
	return value;
});

smol_frame_t* smol_frame_create_advanced(const smol_frame_config_t* config) {

	smol_frame_t* frame = (smol_frame_t*)SMOL_ALLOC_INSTANCE(smol_frame_t);
	*frame = (smol_frame_t){ 0 };
	frame->element = config->web_element != NULL ? config->web_element : "canvas";
	frame->event_queue = smol_event_queue_create(2048);
	emscripten_set_canvas_element_size(frame->element, config->width, config->height);
	frame->width = config->width;
	frame->height = config->height;

	if(config->gl_spec) {
		smol_frame_gl_spec_t* spec = config->gl_spec;
		EmscriptenWebGLContextAttributes attribs = { 
			.alpha = spec->alpha_bits > 0 ? 1 : 0,
			.depth = spec->depth_bits > 0 ? 1 : 0,
			.stencil = spec->stencil_bits > 0 ? 1 : 0,
			.antialias = spec->has_multi_sampling,
			.premultipliedAlpha = 1,
			.preserveDrawingBuffer = 1,
			.powerPreference = 0,
			.failIfMajorPerformanceCaveat = 0,
			.majorVersion = spec->major_version,
			.minorVersion = spec->minor_version,
			.enableExtensionsByDefault = 1,
			.explicitSwapControl = 0,
			.proxyContextToMainThread = 0,
			.renderViaOffscreenBackBuffer = 1,
		};
		
		frame->gl.context = emscripten_webgl_create_context(frame->element, &attribs);

		emscripten_webgl_make_context_current(frame->gl.context);

	}
	else {
		//Insert 2D rendering context into the module
		EM_ASM({
			const elem = UTF8ToString($0);
			const canvas = document.getElementById(elem);
			Module.ctx2D = canvas.getContext("2d");
			Module.canvas = canvas;
			Module.ctx2D.imageSmoothingEnabled = false;
			document.getElementById(elem).focus();

		}, frame->element);
	}

	//Store the keymappings on javascript side.
	EM_ASM({
		Module.keymap = {};
		if(Module["canvas"] === undefined) {
			const elem = UTF8ToString($0);
			Module.canvas = document.getElementById(elem);
		}
		Module.canvas.addEventListener('click', function() {
			Module.canvas.focus();
		});
	}, frame->element);

	SMOL_ASSERT(emscripten_set_fullscreenchange_callback(frame->element, frame, 1, &smol_fullscreen_event) == EMSCRIPTEN_RESULT_SUCCESS);

	SMOL_ASSERT(emscripten_set_mousemove_callback(frame->element, frame, 1, &smol_mouse_event) == EMSCRIPTEN_RESULT_SUCCESS);
	SMOL_ASSERT(emscripten_set_mouseup_callback(frame->element, frame, 1, &smol_mouse_event) == EMSCRIPTEN_RESULT_SUCCESS);
	SMOL_ASSERT(emscripten_set_mousedown_callback(frame->element, frame, 1, &smol_mouse_event) == EMSCRIPTEN_RESULT_SUCCESS);

	SMOL_ASSERT(emscripten_set_wheel_callback(frame->element, frame, 1, &smol_wheel_event) == EMSCRIPTEN_RESULT_SUCCESS);

	SMOL_ASSERT(emscripten_set_keydown_callback(frame->element, frame, 1, &smol_key_event) == EMSCRIPTEN_RESULT_SUCCESS);
	SMOL_ASSERT(emscripten_set_keyup_callback(frame->element, frame, 1, &smol_key_event) == EMSCRIPTEN_RESULT_SUCCESS);
	SMOL_ASSERT(emscripten_set_keypress_callback(frame->element, frame, 1, &smol_key_event) == EMSCRIPTEN_RESULT_SUCCESS);

	SMOL_ASSERT(emscripten_set_focus_callback(frame->element, frame, 1, &smol_focus_event) == EMSCRIPTEN_RESULT_SUCCESS);
	SMOL_ASSERT(emscripten_set_blur_callback(frame->element, frame, 1, &smol_focus_event) == EMSCRIPTEN_RESULT_SUCCESS);

	smol_add_key("AltLeft", SMOLK_LALT);
	smol_add_key("AltRight", SMOLK_RALT);
	smol_add_key("ArrowDown", SMOLK_DOWN);
	smol_add_key("ArrowLeft", SMOLK_LEFT);
	smol_add_key("ArrowRight", SMOLK_RIGHT);
	smol_add_key("ArrowUp", SMOLK_UP);
	//smol_add_key( "AudioVolumeMute", SMOLK_UNKNOWN);
	//smol_add_key( "Backquote", SMOLK_UNKNOWN);
	smol_add_key("Backslash", SMOLK_BACKSLASH);
	smol_add_key("Backspace", SMOLK_BACKSPACE);
	//smol_add_key("BracketLeft", SMOLK_UNKNOWN);
	//smol_add_key("BracketRight", SMOLK_UNKNOWN);
	//smol_add_key("BrowserBack", SMOLK_UNKNOWN);
	//smol_add_key("BrowserFavorites", SMOLK_UNKNOWN);
	//smol_add_key("BrowserForward", SMOLK_UNKNOWN);
	//smol_add_key("BrowserHome", SMOLK_UNKNOWN);
	//smol_add_key("BrowserRefresh", SMOLK_UNKNOWN);
	//smol_add_key("BrowserSearch", SMOLK_UNKNOWN);
	//smol_add_key("BrowserStop", SMOLK_UNKNOWN);
	smol_add_key("CapsLock", SMOLK_CAPSLOCK);
	smol_add_key("Comma", SMOLK_COMMA);
	smol_add_key("ContextMenu", SMOLK_MENU);
	smol_add_key("ControlLeft", SMOLK_LCONTROL);
	smol_add_key("ControlRight", SMOLK_RCONTROL);
	smol_add_key("Dead", SMOLK_UNKNOWN);
	smol_add_key("Delete", SMOLK_DELETE);
	smol_add_key("Digit0", SMOLK_NUM0);
	smol_add_key("Digit1", SMOLK_NUM1);
	smol_add_key("Digit2", SMOLK_NUM2);
	smol_add_key("Digit3", SMOLK_NUM3);
	smol_add_key("Digit4", SMOLK_NUM4);
	smol_add_key("Digit5", SMOLK_NUM5);
	smol_add_key("Digit6", SMOLK_NUM6);
	smol_add_key("Digit7", SMOLK_NUM7);
	smol_add_key("Digit8", SMOLK_NUM8);
	smol_add_key("Digit9", SMOLK_NUM9);
	smol_add_key("End", SMOLK_END);
	smol_add_key("Enter", SMOLK_RETURN);
	smol_add_key("Equal", SMOLK_EQUAL);
	smol_add_key("Escape", SMOLK_ESC);
	smol_add_key("F1", SMOLK_F1);
	smol_add_key("F10", SMOLK_F10);
	smol_add_key("F11", SMOLK_F11);
	smol_add_key("F12", SMOLK_F12);
	smol_add_key("F2", SMOLK_F2);
	smol_add_key("F3", SMOLK_F3);
	smol_add_key("F4", SMOLK_F4);
	smol_add_key("F5", SMOLK_F5);
	smol_add_key("F6", SMOLK_F6);
	smol_add_key("F7", SMOLK_F7);
	smol_add_key("F8", SMOLK_F8);
	smol_add_key("F9", SMOLK_F9);
	smol_add_key("Home", SMOLK_HOME);
	smol_add_key("Insert", SMOLK_INSERT);
	//smol_add_key("IntlYen", SMOLK_UNKNOWN);
	smol_add_key("KeyA", SMOLK_A);
	smol_add_key("KeyB", SMOLK_N);
	smol_add_key("KeyC", SMOLK_C);
	smol_add_key("KeyD", SMOLK_D);
	smol_add_key("KeyE", SMOLK_E);
	smol_add_key("KeyF", SMOLK_F);
	smol_add_key("KeyG", SMOLK_G);
	smol_add_key("KeyH", SMOLK_H);
	smol_add_key("KeyI", SMOLK_I);
	smol_add_key("KeyJ", SMOLK_J);
	smol_add_key("KeyK", SMOLK_K);
	smol_add_key("KeyL", SMOLK_L);
	smol_add_key("KeyM", SMOLK_M);
	smol_add_key("KeyN", SMOLK_N);
	smol_add_key("KeyO", SMOLK_O);
	smol_add_key("KeyP", SMOLK_P);
	smol_add_key("KeyQ", SMOLK_Q);
	smol_add_key("KeyR", SMOLK_R);
	smol_add_key("KeyS", SMOLK_S);
	smol_add_key("KeyT", SMOLK_T);
	smol_add_key("KeyU", SMOLK_U);
	smol_add_key("KeyV", SMOLK_V);
	smol_add_key("KeyW", SMOLK_W);
	smol_add_key("KeyX", SMOLK_X);
	smol_add_key("KeyY", SMOLK_Y);
	smol_add_key("KeyZ", SMOLK_Z);
	//smol_add_key("MediaPlayPause", SMOLK_UNKNOWN);
	//smol_add_key("MediaStop", SMOLK_UNKNOWN);
	//smol_add_key("MediaTrackNext", SMOLK_UNKNOWN);
	//smol_add_key("MediaTrackPrevious", SMOLK_UNKNOWN);
	smol_add_key("Minus", SMOLK_MINUS);
	smol_add_key("NumLock", SMOLK_NUMLOCK);
	smol_add_key("Numpad0", SMOLK_NUMPAD0);
	smol_add_key("Numpad1", SMOLK_NUMPAD1);
	smol_add_key("Numpad2", SMOLK_NUMPAD2);
	smol_add_key("Numpad3", SMOLK_NUMPAD3);
	smol_add_key("Numpad4", SMOLK_NUMPAD4);
	smol_add_key("Numpad5", SMOLK_NUMPAD5);
	smol_add_key("Numpad6", SMOLK_NUMPAD6);
	smol_add_key("Numpad7", SMOLK_NUMPAD7);
	smol_add_key("Numpad8", SMOLK_NUMPAD8);
	smol_add_key("Numpad9", SMOLK_NUMPAD9);
	smol_add_key("NumpadAdd", SMOLK_ADD);
	smol_add_key("NumpadDecimal", SMOLK_PERIOD);
	smol_add_key("NumpadDivide", SMOLK_DIVIDE);
	smol_add_key("NumpadEnter", SMOLK_ENTER);
	smol_add_key("NumpadEqual", SMOLK_EQUAL);
	smol_add_key("NumpadMultiply", SMOLK_MULTIPLY);
	smol_add_key("NumpadSubtract", SMOLK_MINUS);
	smol_add_key("PageDown", SMOLK_PAGEDOWN);
	smol_add_key("PageUp", SMOLK_PAGEUP);
	smol_add_key("Pause", SMOLK_PAUSE);
	smol_add_key("Pause", SMOLK_PAUSE);
	smol_add_key("Period", SMOLK_PERIOD );
	smol_add_key("PrintScreen", SMOLK_PRINTSCREEN);
	smol_add_key("Quote", SMOLK_QUOTE);
	smol_add_key("ScrollLock", SMOLK_SCROLLLOCK);
	smol_add_key("Semicolon", SMOLK_SEMICOLON);
	smol_add_key("ShiftLeft", SMOLK_LSHIFT);
	smol_add_key("ShiftRight", SMOLK_RSHIFT);
	smol_add_key("Slash", SMOLK_SLASH);
	smol_add_key("Space", SMOLK_SPACE);
	smol_add_key("Tab", SMOLK_TAB);
	return frame;

}

EMSCRIPTEN_KEEPALIVE
EM_BOOL smol_fullscreen_event(int event_type, const EmscriptenFullscreenChangeEvent* fullscreen_event, void* user_data) {
	smol_frame_t* frame = (smol_frame_t*)user_data;
	smol_event_queue_t* queue = frame->event_queue;
	smol_frame_event_t ev = { 0 };
	ev.frame = frame;
	return 0;
}

EMSCRIPTEN_KEEPALIVE 
EM_BOOL smol_mouse_event(int event_type, const EmscriptenMouseEvent* mouse_event, void* user_data) {
		
	static const int button_mapping[] = {1, 3, 2};

	smol_frame_t* frame = (smol_frame_t*)user_data;
	smol_event_queue_t* queue = frame->event_queue;
	smol_frame_event_t ev = { 0 };
	ev.frame = frame;

	switch(event_type) {
		case EMSCRIPTEN_EVENT_MOUSEMOVE: 
		case EMSCRIPTEN_EVENT_MOUSEDOWN:
		case EMSCRIPTEN_EVENT_MOUSEUP:
		{

			ev.type = SMOL_FRAME_EVENT_MOUSE_MOVE;

			ev.mouse.x = mouse_event->targetX;
			ev.mouse.y = mouse_event->targetY;

			ev.mouse.dx = mouse_event->movementX;
			ev.mouse.dy = mouse_event->movementY;

			frame->old_mouse_x = ev.mouse.x;
			frame->old_mouse_y = ev.mouse.y;

			if(event_type == EMSCRIPTEN_EVENT_MOUSEMOVE)
				smol_event_queue_push_back(queue, &ev);

			if(event_type == EMSCRIPTEN_EVENT_MOUSEDOWN || event_type == EMSCRIPTEN_EVENT_MOUSEUP) {
				ev.type = (event_type == EMSCRIPTEN_EVENT_MOUSEDOWN) ? SMOL_FRAME_EVENT_MOUSE_BUTTON_DOWN : SMOL_FRAME_EVENT_MOUSE_BUTTON_UP;
				ev.mouse.button = button_mapping[mouse_event->button];
				smol_event_queue_push_back(queue, &ev);
			}
		}
		break;
		default:
			return 0;
	}
	return 1;
}

EMSCRIPTEN_KEEPALIVE
EM_BOOL smol_key_event(int event_type, const EmscriptenKeyboardEvent* key_event, void *user_data) {
	

	smol_frame_t* frame = (smol_frame_t*)user_data;
	smol_event_queue_t* queue = frame->event_queue;
	smol_frame_event_t ev = { 0 };
	ev.frame = frame;

	switch(event_type) {
		case EMSCRIPTEN_EVENT_KEYDOWN:
		case EMSCRIPTEN_EVENT_KEYUP: {
			ev.type = (event_type == EMSCRIPTEN_EVENT_KEYDOWN) ? SMOL_FRAME_EVENT_KEY_DOWN : SMOL_FRAME_EVENT_KEY_UP;
			//ev.key.code = smol_frame_map_key(key_event->keyCode, key_event->location);
			ev.key.code = smol_frame_map_keycode(key_event->code);
			smol_event_queue_push_back(queue, &ev);
			
			if(event_type == EMSCRIPTEN_EVENT_KEYDOWN) {

				const char* buffer = key_event->key;
				unsigned int unicode = 0;
				if(
					(key_event->key[0] >= 'A' && key_event->key[0] <= 'Z') && 
					(ev.key.code < SMOLK_A || ev.key.code > SMOLK_Z)
				) break; //Discard keys whom key name doesn't equate the keyCode. For example key 'Backspace' key's key buffer will contain 'B' 
				//eventhough it should contain NOTHING, it should have UTF-8 value of 8, instead of 'B'. Same goes with the other control keys.

				if((buffer[0] & 0xFE) == 0xFE) unicode = (buffer[0] & 0x01) << 0x1E | (buffer[1] & 0x3F) << 0x18 | (buffer[2] & 0x3F) << 0x12 | (buffer[3] & 0x3F) << 0x0C | (buffer[4] & 0x3F) << 0x06 | (buffer[5] & 0x3F);
				if((buffer[0] & 0xFC) == 0xFC) unicode = (buffer[0] & 0x03) << 0x18 | (buffer[1] & 0x3F) << 0x12 | (buffer[2] & 0x3F) << 0x0C | (buffer[3] & 0x3F) << 0x06 | (buffer[4] & 0x3F);
				if((buffer[0] & 0xF0) == 0xF0) unicode = (buffer[0] & 0x07) << 0x12 | (buffer[1] & 0x3F) << 0x0C | (buffer[2] & 0x3F) << 0x06 | (buffer[3] & 0x3F);
				if((buffer[0] & 0xE0) == 0xE0) unicode = (buffer[0] & 0x0F) << 0x0C | (buffer[1] & 0x3F) << 0x06 | (buffer[2] & 0x3F);
				if((buffer[0] & 0xC0) == 0xC0) unicode = (buffer[0] & 0x1F) << 0x06 | (buffer[1] & 0x3F);
				if((buffer[0] & 0x7F) != 0x00) unicode = (buffer[0] & 0x7F);
			
				
				if(unicode) {
					ev.type = SMOL_FRAME_EVENT_TEXT_INPUT;
					ev.input.codepoint = unicode;
					smol_event_queue_push_back(queue, &ev);
				}
			}

		} break;
		/* case EMSCRIPTEN_EVENT_KEYPRESS: {

			if(event_type == EMSCRIPTEN_EVENT_KEYPRESS) {
				ev.type = SMOL_FRAME_EVENT_TEXT_INPUT;

				unsigned int unicode = 0;
				const char* buffer = key_event->charValue;

				if((buffer[0] & 0xFE) == 0xFE) unicode = (buffer[0] & 0x01) << 30 | (buffer[1] & 0x3F) << 24 | (buffer[2] & 0x3F) << 18 | (buffer[3] & 0x3F) << 12 | (buffer[4] & 0x3F) << 6 | (buffer[5] & 0x3F); break;
				if((buffer[0] & 0xFC) == 0xFC) unicode = (buffer[0] & 0x03) << 24 | (buffer[1] & 0x3F) << 18 | (buffer[2] & 0x3F) << 12 | (buffer[3] & 0x3F) << 6 | (buffer[4] & 0x3F); break;
				if((buffer[0] & 0xF0) == 0xF0) unicode = (buffer[0] & 0x07) << 18 | (buffer[1] & 0x3F) << 12 | (buffer[2] & 0x3F) << 6 | (buffer[3] & 0x3F);
				if((buffer[0] & 0xE0) == 0xE0) unicode = (buffer[0] & 0x0F) << 12 | (buffer[1] & 0x3F) << 6 | (buffer[2] & 0x3F);
				if((buffer[0] & 0xC0) == 0xC0) unicode = (buffer[0] & 0x1F) << 6 | (buffer[1] & 0x3F);
				if((buffer[0] & 0x80) == 0x80) unicode = (buffer[0] & 0x7F);
				
				puts(buffer);

				if(unicode) {
					ev.input.codepoint = unicode;
					smol_event_queue_push_back(queue, &ev);
				}
			}

		} break; */
	}
	return 1;
}

EMSCRIPTEN_KEEPALIVE
EM_BOOL smol_wheel_event(int event_type, const EmscriptenWheelEvent* wheel_event, void *user_data) {

	smol_frame_t* frame = (smol_frame_t*)user_data;
	smol_event_queue_t* queue = frame->event_queue;
	smol_frame_event_t ev = { 0 };
	ev.frame = frame;

	int delX = 0;
	int delY = 0;
	
	if(wheel_event->deltaX) delX = (wheel_event->deltaX < 0.) ? -1 : 1;
	if(wheel_event->deltaY) delY = (wheel_event->deltaX < 0.) ? -1 : 1;

	frame->mouse_z_accum += delX;
	frame->mouse_w_accum += delY;
	ev.mouse.z = frame->mouse_z_accum;
	ev.mouse.w = frame->mouse_w_accum;
	ev.mouse.dz = delX;
	ev.mouse.dw = delY;

	return 1;

}

EMSCRIPTEN_KEEPALIVE
EM_BOOL smol_focus_event(int event_type, const EmscriptenFocusEvent* focus_event, void* user_data) {
	
	smol_frame_t* frame = (smol_frame_t*)user_data;
	smol_event_queue_t* queue = frame->event_queue;
	smol_frame_event_t ev = { 0 };

	ev.frame = frame;

	switch(event_type) {
		case EMSCRIPTEN_EVENT_FOCUS: ev.type = SMOL_FRAME_EVENT_FOCUS_GAINED; break;
		case EMSCRIPTEN_EVENT_BLUR: ev.type = SMOL_FRAME_EVENT_FOCUS_LOST; break;
		default: return 0;
	}

	smol_event_queue_push_back(queue, &ev);

	return 1;
}

void smol_frame_set_title(smol_frame_t* frame, const char* title) {
	(void)frame;
	(void)title;
}

void smol_frame_update(smol_frame_t* frame) {
	//Schedules the browser to do something else than just render_thread_running the main loop.
	emscripten_sleep(0);
}

void smol_frame_set_cursor_visibility(smol_frame_t* frame, int visibility) {
	(void)frame;
	(void)visibility;
}

void smol_frame_destroy(smol_frame_t* frame) {
	(void)frame;
}

void smol_frame_blit_pixels(
	smol_frame_t* frame, 
	unsigned int* pixBuf, 
	int pixBufWidth, 
	int pixBufHeight, 
	int dstX, 
	int dstY, 
	int dstW, 
	int dstH, 
	int srcX, 
	int srcY, 
	int srcW, 
	int srcH
) {
	EM_ASM({

		const xscale_fact = $6 / $10;
		const yscale_fact = $6 / $10;
		
		const imageData = new ImageData($10, $11);
		imageData.data.set(new Uint8ClampedArray(Module.HEAPU8.buffer, $1 + ($8 + $9 * $2) * 4, $10 * $11 * 4));

		Module.ctx2D.putImageData(imageData, $4, $5, 0, 0, $6, $7);
		Module.ctx2D.drawImage(Module.canvas, 0, 0, xscale_fact * Module.canvas.width, yscale_fact * Module.canvas.height);
	}, frame->element, pixBuf, pixBufWidth, pixBufHeight, dstX, dstY, dstW, dstH, srcX, srcY, srcW, srcH);
}

void smol_gl_set_vsync(int enabled) {
	(void)enabled;
}

int smol_frame_gl_swap_buffers(smol_frame_t* frame) {
	(void)frame;
}


#endif 

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
