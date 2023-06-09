#define _CRT_SECURE_NO_WARNINGS
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
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


int main(int numArgs, const char* argv[]) {
	
	(void)numArgs;
	(void)argv;

#ifdef SMOL_PLATFORM_WINDOWS
	SetConsoleOutputCP(CP_UTF8);
#endif 
	setlocale(LC_ALL, 0);
	printf("Hello, world!\n");

	smol_frame_t* frame = smol_frame_create(800, 600, "Smol Frame :)");

	smol_frame_set_title(frame, "Hello, world!");


	Olivec_Canvas canvas = olivec_canvas(surface, 800, 600, 800);

	bool running = true;

	int anchorX = 400;
	int anchorY = 300;

	int fps = 0, frameCounter = 0;
	double timeAccum = 0.;
	double tp1, dt = 0.;

	float red = 0.f;
	float blue = 0.f;

	char buf[256] = {0};

	double blink_timer = 0.4;
	bool is_blinking = true;


	char input_buffer[256] = {0};
	int input_cursor = 0;

	tp1 = smol_timer();
	while(running) {


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
				int l = strlen(input_buffer);


				if(event.input.codepoint < 127 && isprint(event.input.codepoint)) {
					if(input_cursor < l) {
						for(int i = l; i >= input_cursor; i--) {
							input_buffer[i+1] = input_buffer[i];
						}
					}
					input_buffer[input_cursor++] = (char)event.input.codepoint;
				}
				char utf8[8] = { 0 };
				if(smol_utf32_to_utf8(event.input.codepoint, 8, utf8)) 
					printf("TYPED: %s       \r", utf8);
					blink_timer = 0.4;
					is_blinking = 1;
			} else if(event.type == SMOL_FRAME_EVENT_KEY_DOWN) {
				
				if(event.key.code == SMOLK_LEFT) input_cursor--, blink_timer = 0.4, is_blinking = 1;
				if(event.key.code == SMOLK_RIGHT) input_cursor++, blink_timer = 0.4, is_blinking = 1;

				int l = strlen(input_buffer);

				if(event.key.code == SMOLK_BACKSPACE && input_cursor > 0) {
					if(input_cursor == l)
						input_buffer[--input_cursor] = 0;
					else {
						
						for(int i = input_cursor; i <= l; i++) {
							input_buffer[i-1] = input_buffer[i];
						}
						input_cursor--;
					}

					blink_timer = 0.4;
					is_blinking = 1;
				}
				if(input_cursor < 0) input_cursor = 0;
				if(input_cursor > l) input_cursor = l;
			}

			smol_inputs_update(&event);

		}

		olivec_fill(canvas, OLIVEC_RGBA(0, 0, 170, 255));//0xFF0000AA);

		if(smol_mouse_hit(2)) {
			smol_frame_set_cursor_visibility(frame, 0);
		}


		if(smol_mouse_up(2)) {
			smol_frame_set_cursor_visibility(frame, 1);
		}


		if(smol_mouse_hit(1)) {
			anchorX = smol_mouse_x();
			anchorY = smol_mouse_y();
			red = 255.f;
		}

		if(smol_mouse_up(1)) {
			blue = 255.f;
		}

		if(smol_key_down(SMOLK_SPACE)) {
			olivec_rect(canvas, 250, 200, 75, 35, OLIVEC_RGBA(136, 204, 255, 255));
		}


		if(red > 0.f) red -= 100.f * (float)dt; else red = 0.f;
		if(blue > 0.f) blue -= 100.f * (float)dt; else blue = 0.f;
	
		//0xFFCCAA88
		olivec_rect(canvas, 180, 150, 75, 30, OLIVEC_RGBA(136, 170, 255,255));
		olivec_line(canvas, anchorX, anchorY, smol_mouse_x(), smol_mouse_y(), OLIVEC_RGBA(255,0,255, 255));

		//0xFF00CC00 | ((int)red) << 16 | (int)blue
		olivec_circle(canvas, smol_mouse_x(), smol_mouse_y(), 10+smol_mouse_z()*2, OLIVEC_RGBA(((unsigned char)red), 204, ((unsigned char)blue), 255));
		olivec_text(canvas, "hello, world!", 10, 10, smol_font, 1, OLIVEC_RGBA(204, 255, 0, 255));
		/*0xFF00FFCC*/
		snprintf(buf, 256, "Input: %s", input_buffer);
		olivec_text(canvas, buf, 10, 200, smol_font, 1, OLIVEC_RGBA(204, 255, 0, 255));

		if(is_blinking) {
			olivec_text(canvas, "_", 10+(strlen("Input: ")+input_cursor) * smol_font.width, 200, smol_font, 1, OLIVEC_RGBA(204, 255, 0, 255));
		}
		
		blink_timer -= dt;
		if(blink_timer < 0.0) {
			is_blinking = !is_blinking;
			blink_timer = 0.4;
		}

		//0xFFAA00DD
		unsigned int color = OLIVEC_RGBA(221, 0, 170, 255);
		olivec_text(canvas, "!\"#$%&'()*+,-./0123456789:;<=>?@", 10, 25, smol_font, 1, color);
		olivec_text(canvas, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 10, 45, smol_font, 1, color);
		olivec_text(canvas, "[\\]^_`", 10, 65, smol_font, 1, color);
		olivec_text(canvas, "abcdefghijklmnopqrstuvwxyz", 10, 85, smol_font, 1, color);
		olivec_text(canvas, "{|}~", 10, 105, smol_font, 1, color);


		snprintf(buf, 256, "FPS: %d", fps); //0xFFCCFF00
		olivec_text(canvas, buf, 0, 600-32, smol_font, 2, OLIVEC_RGBA(204, 255, 0, 255));

		smol_frame_blit_pixels(frame, surface, 800, 600, 0, 0, 800, 600, 0, 0, 800, 600);

		double tp2 = smol_timer();
		dt = tp2 - tp1;
		tp1 = tp2;
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

