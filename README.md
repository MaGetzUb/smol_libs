# smol_libs

My attempt of making single header C-libraries for *maximum portability. 
* no apple platform support yet.

So far I've made 
* [smol_frame.h](https://github.com/MaGetzUb/smol_libs/blob/master/smol_frame.h) for creating simple windows for graphics programming. In the sample code I use [TSoding](https://github.com/tsoding/)'s [olive.c](https://github.com/tsoding/olive.c).
* [smol_utils.h](https://github.com/MaGetzUb/smol_libs/blob/master/smol_utils.h) containing only one function `smol_timer()` to measure delta time in a frame. `smol_timer()` on windows returns the computer uptime, and on linux returns the seconds(microseconds included) since unix epoch. I was considering to use `clock_gettime()` instead of `gettimeofday()`, but `gettimeofday()` seem to require no additioncal compiler arguments. Some recently added features are utf8<->utf32 and utf16<->utf32 conversions, as well as filesystem scanning and entire file reading. 
* [smol_input.h](https://github.com/MaGetzUb/smol_libs/blob/master/smol_input.h) a complimentary header for input management, has functions for checking is key hit(=pressed), down (=being pressed) and up (=released) and for mouse buttons too. Also it contains functions for mouse location on a window, as well as wheel delta/orientation. These functions will change when multiple frames and shared event queues are properly implemented.
* [smol_canvas.h](https://github.com/MaGetzUb/smol_libs/blob/master/smol_canvas.h) My own attempt for software rendering 2D shapes, lines, circles, images, and text triangles(not working yet).
* [smol_math.h](https://github.com/MaGetzUb/smol_libs/blob/master/smol_math.h) A rudimentary linear algebra library, for vectors, quaternions and  matrices (only compatible with OpenGL atm).
* [smol_text_renderer.h](https://github.com/MaGetzUb/smol_libs/blob/master/smol_text_renderer.h) A simple text renderer that batch renders a text. 

Also there's several tests: 
* [smol_frame_test.c](https://github.com/MaGetzUb/smol_libs/blob/master/smol_frame_test.c) to test window events and inputs (also pixel blitting with `smol_frame_blit_pixels`)
* [smol_snake_test.c](https://github.com/MaGetzUb/smol_libs/blob/master/smol_snake_test.c) a simple game to prove that the api can be utilized for games.
* [smol_gl_test.c](https://github.com/MaGetzUb/smol_libs/blob/master/smol_gl_test.c) a simple rotating hello triangle to demonstrate OpenGL support.
* [smol_snake3d_test.c](https://github.com/MaGetzUb/smol_libs/blob/master/smol_snake3d_test.c) a simple game to prove that the api can be utilized for 3D games, with OpenGL.
* [smol_smol_go_test.c](https://github.com/MaGetzUb/smol_libs/blob/master/smol_go_test.c) a simple go game with smol_canvas.
* [smol_pix_font_creator.c](https://github.com/MaGetzUb/smol_libs/blob/master/smol_pix_font_creator.c) a pixel font creator for generating (ASCII) glyph atlas for `Olivec_Font`. Utilises also [tinyFileDialogs](https://sourceforge.net/projects/tinyfiledialogs/).
* [smol_canvas_test.c](https://github.com/MaGetzUb/smol_libs/blob/master/smol_canvas_test.c) to demonstrated rendering with smol_canvas. 
* [smol_audio_test.c](https://github.com/MaGetzUb/smol_libs/blob/master/smol_audio_test.c) to demonstrated audio output. 

### Building on Windows
> _by using Microsoft Visual Studio 2022 Command prompt_
> `smol_frame_test.c`, `smol_snake_test.c`, or any other test, should be able to be compiled with command: 
> ```
> cl.exe /Zi /EHsc /nologo /Fo:.\build\ .\smol_xxxx_test.c /link /OUT:.\build\smol_xxxx_test.exe
> ```
> and `smol_pix_font_creator.c` can be built with:
> ```
> cl.exe /Zi /EHsc /nologo /Fo:.\build\ .\smol_pix_font_creator.c /link comdlg32.lib /OUT:.\build\smol_pix_font_creator.exe
> ```
>
> _by using MinGW on windows_
> `gcc smol_xxxx_test.c -o smol_xxxx_test`

### Building on Linux
> I've used following commands to compile all the tests:
>
> Using Xlib:
> ```
> gcc -fdiagnostics-color=always -std=c99 -g -pedantic -Wall smol_xxxx_test.c -lm -o build/smol_xxxx_test
> ```
> 
> Using xcb:
> ```
>  gcc -fdiagnostics-color=always -std=c99 -g -pedantic -Wall smol_xxxx_test.c -lxcb -lxcb-icccm lxcb-keysyms -lxcb-xkb -lxkbcommon -lxkbcommon-x11 -lm -o build/smol_xxxx_test
> ```

### On Emscripten
> _This command should apply everything in this repo, if it doesn't I'll update it later_
> ```
> emcc smol_xxxx_test.c -o smol_xxx_test.html -sFULL_ES2=1 -sFULL_ES3=1 -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2  -s WASM=1 -sAUDIO_WORKLET=1 -sWASM_WORKERS=1 -sASYNCIFY=1 
> ```

TODO: build scripts
