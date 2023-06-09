#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <locale.h>
#include <ctype.h>

//#define SMOL_FRAME_BACKEND_XCB
#define SMOL_FRAME_IMPLEMENTATION
#include "smol_frame.h"

#define SMOL_INPUT_IMPLEMENTATION
#include "smol_input.h"

#define SMOL_UTILS_IMPLEMENTATION
#include "smol_utils.h"

#define OLIVEC_IMPLEMENTATION
#include "thirdparty/olive.c"

#include "smol_font_16x16.h"

static Olivec_Font smol_font = {
	.glyphs = &PXF_SMOL_FONT_16X16_DATA[0][0][0],
	.width = PXF_SMOL_FONT_16X16_WIDTH,
	.height = PXF_SMOL_FONT_16X16_HEIGHT
};


typedef unsigned int Uint32;

Uint32 surface[800 * 600];

int utf32_to_utf8(unsigned int chr, int bufLen, char* buf);


int main(int numArgs, const char* argv[]) {
	
#ifdef SMOL_PLATFORM_WINDOWS
	SetConsoleOutputCP(CP_UTF8);
#endif 
	setlocale(LC_ALL, 0);

	smol_frame_t* frame = smol_frame_create(800, 600, "Smol Frame :)");

	smol_frame_set_title(frame, "Hello, world!");


	Olivec_Canvas canvas = olivec_canvas(surface, 800, 600, 800);

	bool running = true;

	int anchorX = 400;
	int anchorY = 300;

	int fps = 0, frameCounter = 0;
	double timeAccum = 0.;
	double tp1, dt;

	float red = 0.f;
	float blue = 0.f;

	char buf[256] = {0};

	printf("%lf\n", smol_timer());
	char input_buffer[256] = {0};
	int input_cursor = 0;

	while(running) {

		tp1 = smol_timer();

		smol_frame_update(frame);
		smol_inputs_flush();

		for(smol_frame_event_t event; smol_frame_acquire_event(frame, &event);) {
			if(event.type == SMOL_FRAME_EVENT_CLOSED) {
				printf("Frame close requested.\n");
				running = false;
				break;
			}
			else if(event.type == SMOL_FRAME_EVENT_RESIZE) {
				printf("Frame resolution size is now: (%d x %d)\n", event.size.width, event.size.height);
				break;
			} else if(event.type == SMOL_FRAME_EVENT_TEXT_INPUT) {
				if(event.unicode < 127 && isprint(event.unicode)) 
					input_buffer[input_cursor++] = (char)event.unicode;
			} else if(event.type == SMOL_FRAME_EVENT_KEY_DOWN) {
				if(event.key.code == SMOLK_BACKSPACE && input_cursor > 0) {
					input_buffer[--input_cursor] = 0;
				}
			}
			else {
				smol_inputs_update(&event);
			}

			/*else if(event.type == SMOL_FRAME_EVENT_MOUSE_BUTTON_DOWN) {
				printf("Mouse button (%d) down event at (%d, %d)\n", event.mouse.button, event.mouse.x, event.mouse.y);
				break;
			}
			else if(event.type == SMOL_FRAME_EVENT_MOUSE_BUTTON_UP) {
				printf("Mouse button (%d) up event at (%d, %d)\n", event.mouse.button, event.mouse.x, event.mouse.y);
			}
			else if(event.type == SMOL_FRAME_EVENT_MOUSE_MOVE) {
				mouseX = event.mouse.x;
				mouseY = event.mouse.y;
			}
			else if(event.type == SMOL_FRAME_EVENT_KEY_DOWN) {
				if(event.key.code == SMOLK_RETURN)
					puts("Return pressed\n");
			}
			else if(event.type == SMOL_FRAME_EVENT_KEY_UP) {
				if(event.key.code == SMOLK_RETURN)
					puts("Return released");
			} else if(event.type == SMOL_FRAME_EVENT_MOUSE_VER_WHEEL) {
				printf("Mouse wheel: %d (accum) %d (delta)\n", event.mouse.z, event.mouse.dz);
			}
			else if(event.type == SMOL_FRAME_EVENT_FOCUS_LOST) {
				puts("Focus lost.");
			}
			else if(event.type == SMOL_FRAME_EVENT_FOCUS_GAINED) {
				puts("Focus gained.");
			}*/

		}

		olivec_fill(canvas, 0xFF0000AA);

		if(smol_mouse_hit(1)) {
			anchorX = smol_mouse_x();
			anchorY = smol_mouse_y();
			red = 255.f;
		}

		if(smol_mouse_up(1)) {
			blue = 255.f;
		}

		if(smol_key_down(SMOLK_SPACE)) {
			olivec_rect(canvas, 250, 200, 75, 35, 0xFFCC88AA);
		}


		if(red > 0.f) red -= 100.f * (float)dt; else red = 0.f;
		if(blue > 0.f) blue -= 100.f * (float)dt; else blue = 0.f;
	

		olivec_rect(canvas, 180, 150, 75, 30, 0xFFCCAA88);
		olivec_line(canvas, anchorX, anchorY, smol_mouse_x(), smol_mouse_y(), 0xFFFF00FF);

		olivec_circle(canvas, smol_mouse_x(), smol_mouse_y(), 10+smol_mouse_z()*2, 0xFF00CC00 | ((int)red) << 16 | (int)blue);
		olivec_text(canvas, "hello, world!", 10, 10, smol_font, 1, 0xFF00FFCC);

		snprintf(buf, 256, "Input: %s", input_buffer);
		olivec_text(canvas, buf, 10, 200, smol_font, 1, 0xFF00FFCC);


		olivec_text(canvas, "!\"#$%&'()*+,-./0123456789:;<=>?@", 10, 25, smol_font, 1, 0xFFAA00DD);
		olivec_text(canvas, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 10, 45, smol_font, 1, 0xFFAA00DD);
		olivec_text(canvas, "[\\]^_`", 10, 65, smol_font, 1, 0xFFAA00DD);
		olivec_text(canvas, "abcdefghijklmnopqrstuvwxyz", 10, 85, smol_font, 1, 0xFFAA00DD);
		olivec_text(canvas, "{|}~", 10, 105, smol_font, 1, 0xFFAA00DD);


		snprintf(buf, 256, "FPS: %d", fps);
		olivec_text(canvas, buf, 0, 600-32, smol_font, 2, 0xFFCCFF00);

		smol_frame_blit_pixels(frame, surface, 800, 600, 0, 0, 800, 600, 0, 0, 800, 600);


		dt = smol_timer() - tp1;
		timeAccum += dt;

		if(timeAccum > 1.) {
			fps = frameCounter;
			frameCounter = 0;
			timeAccum -= 1.;
		}
		frameCounter++;

	}


	smol_frame_destroy(frame);

	return 0;
}


int utf32_to_utf8(unsigned int chr, int buf_len, char* buf) {

	int clen = 1;
	if(chr >= 0x0000080) clen = 2;
	if(chr >= 0x0000800) clen = 3;
	if(chr >= 0x0010000) clen = 4;
	if(chr >= 0x010F800) clen = 5;
	if(chr >= 0x3FFFFFF) clen = 6;

	if(clen > buf_len) return 0;

	char* buf_byte = buf;

	switch(clen) {
		case 0:
		break;
		case 1:
			*buf_byte++ = chr & 0xFF;
		break;
		case 2:
			*buf_byte++ = 0xC0 | ((chr >> 6) & 0x1F);
			*buf_byte++ = 0x80 | (chr & 0x3F);
		break;
		case 3:
			*buf_byte++ = 0xE0 | ((chr >> 12) & 0xF);
			*buf_byte++ = 0x80 | ((chr >> 6) & 0x3F);
			*buf_byte++ = 0x80 | (chr & 0x3F);
		break;
		case 4:
			*buf_byte++ = 0xF0 | ((chr >> 18) & 0x7);
			*buf_byte++ = 0x80 | ((chr >> 12) & 0x3F);
			*buf_byte++ = 0x80 | ((chr >> 6) & 0x3F);
			*buf_byte++ = 0x80 | (chr & 0x3F);
		break;
		case 5:
			*buf_byte++ = 0xF8 | ((chr >> 24) & 0x3);
			*buf_byte++ = 0x80 | ((chr >> 18) & 0x3F);
			*buf_byte++ = 0x80 | ((chr >> 12) & 0x3F);
			*buf_byte++ = 0x80 | ((chr >>  6) & 0x3F);
			*buf_byte++ = 0x80 | (chr & 0x3F);
		break;
		case 6:
			*buf_byte++ = 0xFC | ((chr >> 31) &  0x1);
			*buf_byte++ = 0x80 | ((chr >> 24) & 0x3F);
			*buf_byte++ = 0x80 | ((chr >> 18) & 0x3F);
			*buf_byte++ = 0x80 | ((chr >> 12) & 0x3F);
			*buf_byte++ = 0x80 | ((chr >>  6) & 0x3F);
			*buf_byte++ = 0x80 | (chr & 0x3F);
		break;
	}

	return clen;

}