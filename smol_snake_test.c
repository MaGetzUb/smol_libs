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

#include "smol_font_16x16.h"

typedef struct {
	char offset_x;
	char width;
} Olivec_Font_Hor_Geometry;

typedef struct {
	const char* glyphs;
	int width;
	int height;
	Olivec_Font_Hor_Geometry* geometry;
} Olivec_Font_;

static Olivec_Font_ smol_font_ = {
	.glyphs = &PXF_SMOL_FONT_16X16_DATA[0][0][0],
	.width = PXF_SMOL_FONT_16X16_WIDTH,
	.height = PXF_SMOL_FONT_16X16_HEIGHT,
	.geometry = &PXF_SMOL_FONT_16X16_OFFSET_X_WIDTH
};


#define SNAKE_SIZE 10
#define ARENA_W 20
#define ARENA_H 20
#define HUD_H 40 //Pixels
#define PIXEL_SCALE 4

#define SCREEN_W (ARENA_W * SNAKE_SIZE)
#define SCREEN_H (ARENA_H * SNAKE_SIZE + HUD_H)

uint32_t pixels[SCREEN_H][SCREEN_W];

typedef struct {
	int x, y;
} snek_t;



#define SPEED_FORMULA (0.3 - fmin(pow((snek_len - 3) / 100., 1.25), 1.0)*0.275)


OLIVECDEF void olivec_text_(Olivec_Canvas oc, const char* text, int tx, int ty, Olivec_Font_ font, size_t glyph_size, uint32_t color);


int main(int argc, char* argv[]) {

	smol_frame_t* frame  = smol_frame_create(SCREEN_W*PIXEL_SCALE, SCREEN_H*PIXEL_SCALE, "snek");
	Olivec_Canvas canvas = olivec_canvas(pixels, SCREEN_W, SCREEN_H, SCREEN_W);

	char occupied_arena[ARENA_W * ARENA_H] = {0};

	snek_t snek_pieces[10000];
	int snek_len = 3;
	int sdx = 1, sdy = 0;
	int dx = 1, dy = 0;
	snek_pieces[0] = (snek_t){ARENA_W / 2, ARENA_H / 2};

	for(int i = 1; i < snek_len; i++) {
		snek_pieces[i].x = snek_pieces[i - 1].x-1;
		snek_pieces[i].y = snek_pieces[i - 1].y;
		occupied_arena[snek_pieces[i].x + snek_pieces[i].y * ARENA_W] = 1;
	}
	double next_step = SPEED_FORMULA;
	double move_step = next_step;

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

		if((smol_key_hit(SMOLK_W) || smol_key_hit(SMOLK_UP)) && sdx != 0) dy = -1, dx = 0;
		if((smol_key_hit(SMOLK_S) || smol_key_hit(SMOLK_DOWN)) && sdx != 0) dy = 1, dx = 0;

		if((smol_key_hit(SMOLK_D) || smol_key_hit(SMOLK_RIGHT)) && sdy != 0) dy = 0, dx = 1;
		if((smol_key_hit(SMOLK_A) || smol_key_hit(SMOLK_LEFT)) && sdy != 0) dy = 0, dx = -1;

		if(move_step <= 0.) {

			sdx = dx;
			sdy = dy;

			//Remove occupation from tail
			occupied_arena[snek_pieces[snek_len-1].x + snek_pieces[snek_len-1].y * ARENA_W] = 0;
			for(uint32_t i = snek_len-1; i > 0; i--) {
				snek_pieces[i].x = snek_pieces[i - 1].x;
				snek_pieces[i].y = snek_pieces[i - 1].y;
			}

			snek_pieces[0].x += sdx;
			snek_pieces[0].y += sdy;

			
			if(snek_pieces[0].x > (ARENA_W - 1)) 
				snek_pieces[0].x = 0;
			if(snek_pieces[0].y > (ARENA_H - 1)) 
				snek_pieces[0].y = 0;
			
			if(snek_pieces[0].x < 0) 
				snek_pieces[0].x = (ARENA_W - 1);
			if(snek_pieces[0].y < 0) 
				snek_pieces[0].y = (ARENA_H - 1);

			//Add occupation to head
			occupied_arena[snek_pieces[0].x + snek_pieces[0].y * ARENA_W] = 1;
			
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

				while(occupied_arena[pickup_x + pickup_y * ARENA_W]) {
					pickup_x = rand() % ARENA_W;
					pickup_y = rand() % ARENA_H;
				}

				snek_len++;

				next_step = SPEED_FORMULA;
				if(next_step < 0.) next_step = 0.;

			}

			move_step = next_step;

		}
		else {
			move_step -= dt;
		}

		olivec_fill(canvas, 0xFF000000AA);

		#if 1
		olivec_rect(canvas, 0, SCREEN_H-HUD_H, SCREEN_W, HUD_H, 0xFF444444);
		olivec_rect(canvas, 0, SCREEN_H-HUD_H, SCREEN_W, 3, 0xFF888888);

		olivec_rect(canvas, pickup_x * SNAKE_SIZE, pickup_y * SNAKE_SIZE, SNAKE_SIZE, SNAKE_SIZE, 0xFFAA6644);

		sprintf(guibuf, "Points: %d", (snek_len - 3));
		olivec_text_(canvas, guibuf, 10, SCREEN_H-HUD_H+10, smol_font_, 1, 0xFFFFFF00);
		sprintf(guibuf, "FPS: %d", fps);
		olivec_text_(canvas, guibuf, 10, SCREEN_H-HUD_H+26, smol_font_, 1, 0xFFFFFF00);

		for(int i = 0; i < snek_len; i++) {
			uint32_t col = (128.+cos((double)i * 6.283/5.0)*63.0);
			col |= ((col * 3) / 4) << 8;
			col |= 0xFF000000;
			olivec_rect(canvas, snek_pieces[i].x*SNAKE_SIZE, snek_pieces[i].y*SNAKE_SIZE, SNAKE_SIZE, SNAKE_SIZE, i == 0 ? 0xFFAACCFF : col);
		}
		#else 
		
		//Debug occupied map.
		for(int j = 0; j < ARENA_H; j++)
		for(int i = 0; i < ARENA_W; i++)
		{
			olivec_rect(canvas, i * SNAKE_SIZE, j * SNAKE_SIZE, SNAKE_SIZE, SNAKE_SIZE, occupied_arena[i + j * ARENA_W] ? 0xFFFFFFFF : 0xFF000000);
		}

		#endif 

		smol_frame_blit_pixels(frame, pixels[0], SCREEN_W, SCREEN_H, 0, 0, SCREEN_W * PIXEL_SCALE, SCREEN_H * PIXEL_SCALE, 0, 0, SCREEN_W, SCREEN_H);

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

OLIVECDEF void olivec_text_(Olivec_Canvas oc, const char *text, int tx, int ty, Olivec_Font_ font, size_t glyph_size, uint32_t color)
{
	int gx = tx;
    for (size_t i = 0; *text; ++i, ++text) {
        int gy = ty;
        const char *glyph = &font.glyphs[(*text)*sizeof(char)*font.width*font.height];
        for (int dy = 0; (size_t) dy < font.height; ++dy) {
            for (int dx = 0; (size_t) dx < font.geometry[*text].width+1; ++dx) {
                int px = gx + dx*glyph_size;
                int py = gy + dy*glyph_size;
                if (0 <= px && px < (int) oc.width && 0 <= py && py < (int) oc.height) {
                    if (glyph[dy*font.width + dx + font.geometry[*text].offset_x]) {
                        olivec_rect(oc, px, py, glyph_size, glyph_size, color);
                    }
                }
            }
        }
		if(*text == ' ')
			gx += font.geometry['_'].width;
		else 
			gx += (font.geometry[*text].width+2)*glyph_size;
    }
}
