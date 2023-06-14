# smol_libs

My attempt of making single header C-libraries for maximum portability. 

So far I've made 
* [smol_frame.h](https://github.com/MaGetzUb/smol_libs/blob/master/smol_frame.h) for creating simple windows for graphics programming. In the sample code I use [TSoding](https://github.com/tsoding/)'s [olive.c](https://github.com/tsoding/olive.c).
* [smol_utils.h](https://github.com/MaGetzUb/smol_libs/blob/master/smol_utils.h) containing only one function `smol_timer()` to measure delta time in a frame. `smol_timer()` on windows returns the computer uptime, and on linux returns the seconds(microseconds included) since unix epoch. I was considering to use `clock_gettime()` instead of `gettimeofday()`, but `gettimeofday()` seem to require no additioncal compiler arguments.
* [smol_input.h](https://github.com/MaGetzUb/smol_libs/blob/master/smol_input.h) a complimentary header for input management, has functions for checking is key hit(=pressed), down (=being pressed) and up (=released) and for mouse buttons too. Also it contains functions for mouse location on a window, as well as wheel delta/orientation. These functions will change when multiple frames and shared event queues are properly implemented.

Also there's several tests: 
[smol_frame_test.c](https://github.com/MaGetzUb/smol_libs/blob/master/smol_frame_test.c) to test window events and inputs (also pixel blitting with)
[smol_snake_test.c](https://github.com/MaGetzUb/smol_libs/blob/master/smol_snake_test.c) a simple game to prove that the api can be utilized for games.
[smol_pix_font_creator.c](https://github.com/MaGetzUb/smol_libs/blob/master/smol_pix_font_creator.c) a pixel font creator for generating (ASCII) glyph atlas for `Olivec_Font`. Utilises also [tinyFileDialogs](https://sourceforge.net/projects/tinyfiledialogs/).

### Building on Windows
> _by using Microsoft Visual Studio 2022 Command prompt_
> `smol_frame_test.c` and `smol_snake_test.c` should be able to be compiled with command: 
> ```
> cl.exe /Zi /EHsc /nologo /Fo:.\build\ .\smol_xxxx_test.c /link /OUT:.\build\smol_xxxx_test.exe
> ```
> and `smol_pix_font_creator.c` can be built with:
> ```
> cl.exe /Zi /EHsc /nologo /Fo:.\build\ .\smol_pix_font_creator.c /link comdlg32.lib /OUT:.\build\smol_pix_font_creator.exe
> ```

### Building on Linux
> I've used following commands to compile all the tests:
>
> Using Xlib:
> ```
> gcc -fdiagnostics-color=always -std=c99 -g -pedantic -Wall smol_xxxx_test.c -lX11 -lm -o build/smol_xxxx_test
> ```
> 
> Using xcb:
> ```
>  gcc -fdiagnostics-color=always -std=c99 -g -pedantic -Wall smol_xxxx_test.c -lxcb -lxcb-icccm lxcb-keysyms -lxcb-xkb -lxkbcommon -lxkbcommon-x11 -lm -o build/smol_xxxx_test
> ```

TODO: build scripts
