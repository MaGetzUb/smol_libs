#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>

#define SMOL_FRAME_BACKEND_XCB
#define SMOL_FRAME_IMPLEMENTATION
#include "smol_frame.h"

#define SMOL_INPUT_IMPLEMENTATION
#include "smol_input.h"

#define SMOL_UTILS_IMPLEMENTATION
#include "smol_utils.h"

#define OLIVEC_IMPLEMENTATION
#include "thirdparty/olive.c"

#include "smol_font.h"

static Olivec_Font smol_font = {
	.glyphs = &PXF_SMOL_FONT_DATA[0][0][0],
	.width = PXF_SMOL_FONT_WIDTH,
	.height = PXF_SMOL_FONT_HEIGHT
};

#define SNAKE_SIZE 20
#define ARENA_W 40
#define ARENA_H 25

#define SCREEN_W (ARENA_W * SNAKE_SIZE)
#define SCREEN_H (ARENA_W * SNAKE_SIZE)

uint32_t pixels[600][800];

typedef struct {
	int x, y;
} snek_t;


#define SPEED_FORMULA (0.3 - fmin(pow((snek_len - 3) / 100., 1.25), 1.0)*0.275)

int main(int argc, char* argv[]) {

	smol_frame_t* frame  = smol_frame_create(800, 600, "snek");
	Olivec_Canvas canvas = olivec_canvas(pixels, 800, 600, 800);


	snek_t snek_pieces[10000];
	int snek_len = 3;
	int dx = 1, dy = 0;
	snek_pieces[0] = (snek_t){ARENA_W / 2, ARENA_H / 2};

	for(int i = 1; i < snek_len; i++) {
		snek_pieces[i].x = snek_pieces[i - 1].x-1;
		snek_pieces[i].y = snek_pieces[i - 1].y;
	}
	double next_step = SPEED_FORMULA;
	double move_step = next_step;

	printf("Step speed %lf\n", next_step);

	double time_accum = 0.;
	double dt = 0., tp1 = 0.;
	int fps = 0, frame_counter = 0;

	int pickup_x = rand() % ARENA_W;
	int pickup_y = rand() % ARENA_H;

	char guibuf[512];

	int dir_changed = 0;
	int failed = 0;

	while(!smol_frame_is_closed(frame) && !failed) {

		tp1 = smol_timer();

		smol_frame_update(frame);
		smol_inputs_flush();

		for(smol_frame_event_t ev; smol_frame_acquire_event(frame, &ev);) {
			smol_inputs_update(&ev);
		}

		if((smol_key_hit(SMOLK_W) || smol_key_hit(SMOLK_UP)) && dx != 0) dy = -1, dx = 0;
		if((smol_key_hit(SMOLK_S) || smol_key_hit(SMOLK_DOWN)) && dx != 0) dy = 1, dx = 0;

		if((smol_key_hit(SMOLK_D) || smol_key_hit(SMOLK_RIGHT)) && dy != 0) dy = 0, dx = 1;
		if((smol_key_hit(SMOLK_A) || smol_key_hit(SMOLK_LEFT)) && dy != 0) dy = 0, dx = -1;

		if(move_step <= 0.) {

			for(uint32_t i = snek_len-1; i > 0; i--) {
				snek_pieces[i].x = snek_pieces[i - 1].x;
				snek_pieces[i].y = snek_pieces[i - 1].y;
			}

			snek_pieces[0].x += dx;
			snek_pieces[0].y += dy;
			
			if(snek_pieces[0].x > (ARENA_W - 1)) snek_pieces[0].x = 0;
			if(snek_pieces[0].y > (ARENA_H - 1)) snek_pieces[0].y = 0;
			
			if(snek_pieces[0].x < 0) snek_pieces[0].x = (ARENA_W - 1);
			if(snek_pieces[0].y < 0) snek_pieces[0].y = (ARENA_H - 1);

			for(uint32_t i = 1; i < snek_len; i++) {
				if(snek_pieces[0].x == snek_pieces[i].x && snek_pieces[0].y == snek_pieces[i].y) {
					failed = 1;
					break;
				}
			}

			if(failed) {
				break;
			}

			if(snek_pieces[0].x == pickup_x && snek_pieces[0].y == pickup_y) {
				snek_pieces[snek_len].x = snek_pieces[snek_len-1].x;
				snek_pieces[snek_len].y = snek_pieces[snek_len-1].y;
				
				pickup_x = rand() % ARENA_W;
				pickup_y = rand() % ARENA_H;

				snek_len++;

				next_step = SPEED_FORMULA;
				if(next_step < 0.) next_step = 0.;

				printf("Step speed %lf\n", next_step);
			}

			move_step = next_step;

		}
		else {
			move_step -= dt;
		}

		olivec_fill(canvas, 0xFF000000AA);
		olivec_rect(canvas, 0, 500, 800, 300, 0xFF444444);
		olivec_rect(canvas, 0, 500, 800, 3, 0xFF888888);

		olivec_rect(canvas, pickup_x * SNAKE_SIZE, pickup_y * SNAKE_SIZE, SNAKE_SIZE, SNAKE_SIZE, 0xFFAA6644);

		sprintf(guibuf, "Points: %d", (snek_len - 3));
		olivec_text(canvas, guibuf, 10, 510, smol_font, 2, 0xFFFFFF00);

		for(int i = 0; i < snek_len; i++) {
			uint32_t col = (128.+cos((double)i * 6.283/5.0)*63.0);
			col |= ((col * 3) / 4) << 8;
			col |= 0xFF000000;
			olivec_rect(canvas, snek_pieces[i].x*SNAKE_SIZE, snek_pieces[i].y*SNAKE_SIZE, SNAKE_SIZE, SNAKE_SIZE, i == 0 ? 0xFFAACCFF : col);
		}

	
		smol_frame_blit_pixels(frame, pixels[0], 800, 600, 0, 0, 800, 600, 0, 0, 800, 600);

		dt = smol_timer() - tp1;
		time_accum += dt;

		if(time_accum > 1.) {
			fps = frame_counter;
			frame_counter = 0;
			time_accum -= 1.;
		}
		frame_counter++;
	}

	if(failed) {
		printf("You died w/ %d points.", (snek_len-3));
	}

	smol_frame_destroy(frame);

	return 0;

}