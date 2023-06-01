#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define SMOL_FRAME_BACKEND_XCB
#define SMOL_FRAME_IMPLEMENTATION
#include "smol_frame.h"

#define SMOL_UTILS_IMPLEMENTATION
#include "smol_utils.h"

#define OLIVEC_IMPLEMENTATION
#include "thirdparty/olive.c"




typedef unsigned int Uint32;

Uint32 surface[800 * 600];


int main(int numArgs, const char* argv[]) {

	smol_frame_t* frame = smol_frame_create(800, 600, "Smol Frame :)");

#if 0
	for(Uint32 j = 0U; j < 600U; j++)
	for(Uint32 i = 0u; i < 800u; i++)
		surface[i + j * 800U] = (i & 255) ^ (j & 255);
#endif 

	Olivec_Canvas canvas = olivec_canvas(surface, 800, 600, 800);

	bool running = true;
	bool drawing = false;

	int mouseX = 0;
	int mouseY = 0;

	int fps = 0, frameCounter = 0;
	double timeAccum = 0.;
	double tp1, dt;

	char buf[256] = {0};

	printf("%lf\n", smol_timer());

	while(running) {

		double tp1 = smol_timer();

		smol_frame_update(frame);

		for(smol_frame_event_t event; smol_frame_acquire_event(frame, &event);) {
			if(event.type == SMOL_FRAME_EVENT_CLOSED) {
				printf("Frame close requested.\n");
				running = false;
				break;
			}
			else if(event.type == SMOL_FRAME_EVENT_RESIZE) {
				printf("Frame resolution size is now: (%d x %d)\n", event.size.width, event.size.height);
				break;
			}
			else if(event.type == SMOL_FRAME_EVENT_MOUSE_BUTTON_DOWN) {
				if(event.mouse.button == 1) drawing = true;
				printf("Mouse button (%d) down event at (%d, %d)\n", event.mouse.button, event.mouse.x, event.mouse.y);
				break;
			}
			else if(event.type == SMOL_FRAME_EVENT_MOUSE_BUTTON_UP) {
				if(event.mouse.button == 1) drawing = false;
			}
			else if(event.type == SMOL_FRAME_EVENT_MOUSE_MOVE) {
			#if 0
				if(drawing) {
					surface[event.mouse.x + event.mouse.y * 800] |= (255 << 16);
				}
			#endif 
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
			}

		}

		olivec_fill(canvas, 0xFF0000AA);

		olivec_rect(canvas, 180, 150, 75, 30, 0xFFCCAA88);
		olivec_line(canvas, 400, 300, mouseX, mouseY, 0xFFFF00FF);
		olivec_circle(canvas, mouseX, mouseY, 10, 0xFF00CC00);
		olivec_text(canvas, "hello, world!", 10, 10, olivec_default_font, 2, 0xFF00FFCC);

		snprintf(buf, 256, "fps: %d", fps);
		olivec_text(canvas, buf, 0, 588, olivec_default_font, 2, 0xFFCCFF00);

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
