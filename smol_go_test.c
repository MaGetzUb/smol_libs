#define _CRT_SECURE_NO_WARNINGS

#define SMOL_FRAME_IMPLEMENTATION
#include "smol_frame.h"

#define SMOL_CANVAS_IMPLEMENTATION
#include "smol_canvas.h"

#define SMOL_UTILS_IMPLEMENTATION
#include "smol_utils.h"

#define SMOL_INPUT_IMPLEMENTATION
#include "smol_input.h"


#include "smol_font_16x16.h"

#define CELL_SIDE 20

#define ARENA_W 40
#define ARENA_H 25

#define HUD_H 100

#define ARENA_W_PIXELS (ARENA_W * CELL_SIDE)
#define ARENA_H_PIXELS  (ARENA_H * CELL_SIDE)

#define SCREEN_W ARENA_W_PIXELS
#define SCREEN_H (ARENA_H_PIXELS + HUD_H)
#define BOARD_W (ARENA_W_PIXELS/CELL_SIDE)
#define BOARD_H (ARENA_H_PIXELS/CELL_SIDE)

char board[BOARD_W * BOARD_H];


int do_button(smol_canvas_t* canvas, int x, int y, int w, int h, const char* txt) {
	
	int highlight = 0;
	int pressed = 0;
	int released = 0;
	
	int tw, th;
	smol_text_size(canvas, 1, txt, &tw, &th);
	if(tw > w) w = tw+4;
	if(th > h) h = th+4;

	if(smol_mouse_x() >= x && smol_mouse_x() < (x + w) && smol_mouse_y() >= y && smol_mouse_y() < (y + h)) {
		highlight = 1;
		if(smol_mouse_down(1)) pressed = 1;
		if(smol_mouse_up(1)) released = 1;
	}

	smol_canvas_push_color(canvas);
	if(highlight) smol_canvas_lighten_color(canvas, 10);

	//Fill background
	smol_canvas_push_color(canvas);
	smol_canvas_darken_color(canvas, 30);
	smol_canvas_fill_rect(canvas, x, y, w, h);
	smol_canvas_pop_color(canvas);

	//Draw edges
	smol_canvas_push_color(canvas);
	smol_canvas_darken_color(canvas, 15);
	smol_canvas_draw_rect(canvas, x, y, w, h);
	smol_canvas_pop_color(canvas);

	//Draw text
	smol_canvas_push_color(canvas);
	smol_canvas_lighten_color(canvas, 20);
	if(pressed) smol_canvas_lighten_color(canvas, 50);

	smol_canvas_draw_text(
		canvas, 
		x + w/2 - tw/2 + pressed, 
		y + h/2 - th/2 + pressed, 
		1, 
		txt
	);
	
	smol_canvas_pop_color(canvas);
	smol_canvas_pop_color(canvas);

	return released;
}


int check_board(int pidx, int tx, int ty, int dx, int dy, int* sx, int* sy, int* ex, int* ey) {

	pidx = pidx + 1;
	int cx = tx;
	int cy = ty;
	int same_signs = 0;

	//Go to the opposite direction until it's not possible
	while(board[(cx-dx) + (cy-dy) * BOARD_W] == pidx) {
		cx -= dx;
		cy -= dy;
	}
	*sx = cx;
	*sy = cy;

	while(board[cx + cy * BOARD_W] == pidx) {
		same_signs++;
		if(same_signs >= 5) break;
		cx += dx;
		cy += dy;
	}

	*ex = cx;
	*ey = cy;

	return same_signs;
}

int main() {

	smol_frame_t* frame = smol_frame_create(SCREEN_W, SCREEN_H, "Go?");
	smol_canvas_t canvas = smol_canvas_create(SCREEN_W, SCREEN_H);

	smol_font_t font = smol_create_font(
		(char*)PXF_SMOL_FONT_16X16_DATA, 
		PXF_SMOL_FONT_16X16_WIDTH,
		PXF_SMOL_FONT_16X16_HEIGHT, 
		(smol_font_hor_geometry_t*)PXF_SMOL_FONT_16X16_OFFSET_X_WIDTH
	);

	smol_canvas_set_font(&canvas, &font);

	int turn;
	int victory_state;

	reset:
	turn = 0;
	victory_state = 0;
	memset(board, 0, BOARD_W * BOARD_H);
	int running = 1;

	int win_sx = 0;
	int win_sy = 0;
	int win_ex = 0;
	int win_ey = 0;
	int animating = 0;
	double anim_time = 0.;
	int animation_finished = 0;

	while(!smol_frame_is_closed(frame) && running) {
		smol_frame_update(frame);
		smol_inputs_flush();

		SMOL_FRAME_EVENT_LOOP(frame, ev) {
			smol_inputs_update(&ev);
		}

		int sx = smol_mouse_x() / CELL_SIDE;
		int sy = smol_mouse_y() / CELL_SIDE;

		if(smol_mouse_hit(1) && board[sx + sy * 40] == 0 && victory_state == 0) {


			turn = !turn;
			
			board[sx + sy * 40] = 1+turn;
			
			victory_state = (check_board(turn, sx, sy, 1, 0, &win_sx, &win_sy, &win_ex, &win_ey) == 5) ? (1+turn) : 0;
			if(victory_state) goto out;
			
			victory_state = (check_board(turn, sx, sy, 0, 1, &win_sx, &win_sy, &win_ex, &win_ey) == 5) ? (1+turn) : 0;
			if(victory_state) goto out;
			
			victory_state = (check_board(turn, sx, sy, 1, 1, &win_sx, &win_sy, &win_ex, &win_ey) == 5) ? (1+turn) : 0;
			if(victory_state) goto out;

			victory_state = (check_board(turn, sx, sy, 1,-1, &win_sx, &win_sy, &win_ex, &win_ey) == 5) ? (1+turn) : 0;
			if(victory_state) goto out;
		
		}

		out:
		if(victory_state && animation_finished == 0 && animating == 0) 
			animating = 1, anim_time = smol_timer();

		smol_canvas_clear(&canvas, SMOLC_DARKEST_GREY);

		smol_canvas_set_color(&canvas, SMOLC_DARKEST_GREY);
		smol_canvas_lighten_color(&canvas, 50);
		if(victory_state == 0) 
			smol_canvas_fill_rect(&canvas, sx * CELL_SIDE, sy* CELL_SIDE, CELL_SIDE, CELL_SIDE);

		smol_canvas_set_color(&canvas, SMOLC_GREY);

		for(int y = 0; y <= BOARD_H; y++)
			smol_canvas_draw_line(&canvas, 0, y*CELL_SIDE, ARENA_W_PIXELS, y*CELL_SIDE);
			
		for(int x = 0; x < BOARD_W; x++)
			smol_canvas_draw_line(&canvas, x*CELL_SIDE, 0, x*CELL_SIDE, ARENA_H_PIXELS);
			
		for(int y = 0; y < BOARD_H; y++) 
		for(int x = 0; x < BOARD_W; x++)
		{

			switch(board[x + y * BOARD_W]) {
				case 1:
					smol_canvas_set_color(&canvas, SMOLC_SKYBLUE);
					smol_canvas_draw_line(&canvas, x * CELL_SIDE, y * CELL_SIDE, x * CELL_SIDE + CELL_SIDE, y * CELL_SIDE + CELL_SIDE);
					smol_canvas_draw_line(&canvas, x * CELL_SIDE, y * CELL_SIDE + CELL_SIDE, x * CELL_SIDE + CELL_SIDE, y * CELL_SIDE);
				break;
				case 2:
					smol_canvas_set_color(&canvas, SMOLC_ORANGE);
					smol_canvas_draw_circle(&canvas, x * CELL_SIDE + CELL_SIDE/2, y * CELL_SIDE + CELL_SIDE/2, CELL_SIDE/2);
				break;
			}

		}

		if(animating == 1) {

			if((smol_timer() - anim_time) > 2.0f) {
				animating = 0;
				animation_finished = 1;
			}



			
			float t = smol_remapf(smol_timer() - anim_time, 0.f, 1.5f, 0.f, 1.f);

			if(t >= 1.0f) t = 1.0f;
			float it = 1.0f - t;

			float ex = (float)win_sx * it + (float)win_ex * t;
			float ey = (float)win_sy * it + (float)win_ey * t;

			smol_canvas_push_color(&canvas);
			smol_canvas_set_color(&canvas, SMOLC_RED);
			smol_canvas_draw_line(
				&canvas, 
				win_sx * CELL_SIDE + CELL_SIDE/2,
				win_sy * CELL_SIDE + CELL_SIDE/2, 
				ex * CELL_SIDE + CELL_SIDE/2, 
				ey * CELL_SIDE + CELL_SIDE/2
			);
			smol_canvas_pop_color(&canvas);
		}

		{
			char txt[64];
			int tbw, tbh;
			sprintf(txt, "Player: #%d", (1+turn));
			smol_canvas_set_color(&canvas, SMOLC_GREY);
			smol_text_size(&canvas, 2, txt, &tbw, &tbh);
			smol_canvas_draw_text(&canvas, 20, SCREEN_H - 50 - tbh/2, 2, txt);
		}

		if(victory_state && animation_finished) {

			smol_canvas_set_color(&canvas, SMOLC_DARK_GREY);
			smol_canvas_fill_rect(&canvas, 200, 150, 400, 300);

			smol_canvas_set_color(&canvas, SMOLC_LIGHT_GREY);
			smol_canvas_draw_rect(&canvas, 200, 150, 400, 300);

			smol_canvas_set_color(&canvas, ((smol_pixel_t[]) { SMOLC_BLANK, SMOLC_SKYBLUE, SMOLC_ORANGE })[victory_state]);

			const char* str = ((const char* []) { "", "Player 1 won!", "Player 2 won!" })[victory_state];


			int w = 0, h = 0;
			smol_text_size(&canvas, 2, str, &w, &h);
			smol_canvas_draw_text(&canvas, 400 - w/2, 300 - h / 2, 2, str);

			smol_canvas_darken_color(&canvas, 20);
			
			smol_canvas_set_color(&canvas, SMOLC_DARK_GREEN);
			smol_text_size(&canvas, 1, "New game", &w, &h);
			if(do_button(&canvas, 350 - w/2, 450 - 40, 50, 8, "New game"))
				goto reset;
			
			smol_canvas_set_color(&canvas, SMOLC_DARK_RED);
			smol_text_size(&canvas, 1, "Exit game", &w, &h);
			if(do_button(&canvas, 450 - w/2, 450 - 40, 50, 8, "Exit game"))
				running = 0;
		}

		smol_canvas_present(&canvas, frame);
	}



	return 0;
}