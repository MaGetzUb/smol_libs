#include <stdio.h>
#include <wchar.h>
#include <assert.h>

#include <dlfcn.h>
#include <unistd.h> // for usleep
#include <assert.h>

#include <X11/X.h>
#include <X11/Xlib.h>

typedef Display* smol_XOpenDisplay_proc(const char* display);
typedef int smol_XCloseDisplay_proc(Display* display);
typedef int smol_XGetWindowAttributes_proc(Display* display, Window parentWindow, XWindowAttributes* attirbutes);
typedef Window smol_XCreateWindow_proc(
    Display* display, 
    Window parent, 
    int x, 
    int y, 
    unsigned int width, 
    unsigned int height, 
    unsigned int border_width, 
    unsigned int depth, 
    unsigned int window_class, 
    Visual* visual,
    unsigned int value_mask,
    XSetWindowAttributes* XWindowAttributes
);
typedef int smol_XDestroyWindow_proc(Display* display,Window w);
typedef int smol_XMapWindow_proc(Display* display, Window window);
typedef int smol_XFlush_proc(Display* display);
typedef void smol_XSetWMNormalHints_proc(Display* display, Window window, XSizeHints* hints);
typedef Atom smol_XInternAtom_proc(Display* display, _Xconst char* atom_name, Bool only_if_exists);
typedef Status smol_XSetWMProtocols_proc(Display* display, Window window, Atom* protocols, int count);
typedef int smol_XStoreName_proc(Display* display, Window window, _Xconst char* window_name);
typedef XIM smol_XOpenIM_proc(Display* display, struct _XrmHashBucketRec* rdb, char* res_name, char* res_class);


typedef XImage* smol_XCreateImage_proc(
    Display* display, 
    Visual*	visual, 
    unsigned int depth, 
    int format, 
    int offset, 
    char* data, 
    unsigned int width, 
    unsigned int height,
    int	bitmap_pad,
    int bytes_per_line
);

typedef int smol_XCreateGC_proc(Display* display, Drawable drawable, unsigned long valuemask, XGCValues* values);
typedef XIM smol_XOpenIM_proc(Display* dpy, struct _XrmHashBucketRec* rdb, char* res_name, char* res_class);
typedef Status smol_XGetIMValues_proc(char *XGetIMValues(XIM im, ...) _X_SENTINEL(0);
typedef XIC smol_XCreateIC_proc(XIM im, ...) _X_SENTINEL(0);
typedef void smol_XSetICFocus_proc(XIC  ic);
typedef int smol_XChangeProperty_proc(Display* display, Window window, Atom property, Atom type, int format, int mode, _Xconst unsigned char* data, int nelements);
typedef void smol_XFree_proc(void* data);
typedef Colormap smol_XCreateColormap_proc(Display* display, Window w, Visual* visual, int alloc);
typedef int smol_XInstallColormap_proc(Display* display, Colormap colormap);
typedef void smol_XDestroyIC_proc(XIC ic);
typedef Status smol_XCloseIM_proc(XIM im);
typedef int smol_XNextEvent_proc(Display* display, XEvent* event_return)
typedef Bool smol_XFilterEvent_proc(XEvent* event, Window window);
typedef int smol_XPending_proc(Display* display);

typedef int smol_XPeekEvent_proc(Display* display, XEvent* event_return);
typedef KeySym smol_XLookupKeysym_proc(XKeyEvent* key_event, int index);
typedef int smol_Xutf8LookupString_proc(
    XIC ic,
    XKeyPressedEvent* event,
    char* buffer_return,
    int bytes_buffer,
    KeySym* keysym_return,
    Status* status_return
);
typedef int smol_XPending_proc(Display* display);




/*
	im = XOpenIM(display, NULL, NULL, NULL);
	XGetIMValues(im, XNQueryInputStyle, &styles, NULL);
	ic = XCreateIC(im, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, result_window, NULL);

	XSetICFocus(ic);
*/


int main() {

    void* so = dlopen("libX11.so.7", RTLD_NOW);
    if(!so) so = dlopen("libX11.so.6", RTLD_NOW);
    if(!so) so = dlopen("libX11.so.5", RTLD_NOW);
    if(!so) so = dlopen("libX11.so.4", RTLD_NOW);
    if(!so) so = dlopen("libX11.so.3", RTLD_NOW);
    if(!so) so = dlopen("libX11.so.2", RTLD_NOW);
    if(!so) so = dlopen("libX11.so.1", RTLD_NOW);
    if(!so) so = dlopen("libX11.so", RTLD_NOW);

    smol_XOpenDisplay_proc* X_OpenDisplay = (smol_XOpenDisplay_proc*)dlsym(so, "XOpenDisplay"); 
    smol_XCloseDisplay_proc* X_CloseDisplay = (smol_XCloseDisplay_proc*)dlsym(so, "XCloseDisplay");
    smol_XGetWindowAttributes_proc* X_GetWindowAttributes = (smol_XGetWindowAttributes_proc*)dlsym(so, "XGetWindowAttributes");
    smol_XCreateWindow_proc* X_CreateWindow = (smol_XCreateWindow_proc*)dlsym(so, "XCreateWindow");
    smol_XMapWindow_proc* X_MapWindow = (smol_XMapWindow_proc*)dlsym(so, "XMapWindow");
    smol_XFlush_proc* X_Flush = (smol_XFlush_proc*)dlsym(so, "XFlush");

    Display* display = X_OpenDisplay(0);
    Window defaultRootWindow = DefaultRootWindow(display);
    XWindowAttributes attributes;
    X_GetWindowAttributes(display, defaultRootWindow, &attributes);

    XSetWindowAttributes set_attributes = {0};
	
	set_attributes.event_mask = (
		ExposureMask        | KeyPressMask      | 
		KeyReleaseMask      | ButtonPressMask   | 
		ButtonReleaseMask   | PointerMotionMask | 
		ButtonMotionMask    | FocusChangeMask
	);
    
    Window window = X_CreateWindow(
        display, 
        defaultRootWindow, 
        (attributes.width + 800) >> 1, 
        (attributes.height + 600) >> 1, 
        800, 
        600, 
        0,
		CopyFromParent, 
		InputOutput,
		CopyFromParent, 
		CWEventMask | CWColormap, 
		&set_attributes
    );


    X_MapWindow(display, window);
    X_Flush(display);

    sleep(3);

    X_CloseDisplay(display);
    close(so);

    return 0;
}


#ifdef __EMSCRIPTEN__

EM_JS(int, smol_audio_setup, (int num_channels, int num_buffer_samples, int num_buffers), {

	const AudioContext = (window.audioContext || window.webkitAudioContext);

	var audio_engine = {}
	audio_engine.context = new AudioContext();

	if(audio_engine.context == null || audio_context == undefined) {
		console.log("Unable to create audio context!");
		return 0;
	}

	var total_sample_count = num_channels * num_buffer_samples * num_buffers;

	audio_engine.num_frames = num_buffer_samples;
	audio_engine.num_buffer_samples = num_buffer_samples;
	audio_engine.num_buffers = num_buffers;
	audio_engine.num_channels = num_channels;
	audio_engine.buffer_index = 0;
	audio_engine.buffer_size = total_sample_count;
	audio_engine.buffer_len_secs = num_buffer_samples / audio_engine.context.sampleRate;
	audio_engine.next_play_time = 0;
	audio_engine.sample_buffer = Module._malloc(total_sample_count*2);

	audio_engine.sample_buffer_offsets = new Array(num_sample_buffers);
	audio_engine.sample_buffer_channels = new Array(numBuffers);

	for(var i = 0; i < num_sample_buffers; i++) {

		var pointers = new Uint32Array(gAudio.mNumChannels);
		

		
	}

	audio_engine.fill_callback = null;
	audio_engine.fill_callback_userdata = null;

	audio_engine.push_audio = function(buffer_offsets) {
		
		var buffer_source = audio_engine.context.createBufferSource();
		var audio_buffer = audio_engine.context.createBuffer(
			audio_engine.num_channels, 
			audio_engine.num_buffer_samples,
			audio_engine.context.sampleRate
		);
		buffer_source.connect(audio_engine.context.destination);

	}

	Module.audioEngine = audio_engine;
	

})



#endif 