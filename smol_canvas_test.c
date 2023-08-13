#define _CRT_SECURE_NO_WARNINGS

#define SMOL_FRAME_IMPLEMENTATION
#include "smol_frame.h"

#define SMOL_INPUT_IMPLEMENTATION
#include "smol_input.h"

#define SMOL_CANVAS_IMPLEMENTATION
#include "smol_canvas.h"

#define SMOL_UTILS_IMPLEMENTATION
#include "smol_utils.h"

#include "smol_font_16x16.h"

#include <math.h>

int main() {

	printf("%s\n", smol_get_current_directory());

	smol_font_t font = smol_create_font(
		(const char*)PXF_SMOL_FONT_16X16_DATA,
		PXF_SMOL_FONT_16X16_WIDTH,
		PXF_SMOL_FONT_16X16_HEIGHT,
		(smol_font_hor_geometry_t*)PXF_SMOL_FONT_16X16_OFFSET_X_WIDTH
	); 

	smol_image_t img = { 0 };
	img = smol_load_image_qoi("res/gfx/test.qoi");

	smol_frame_t* frame = smol_frame_create(512, 512, "Canvas test");
	smol_canvas_t canvas = smol_canvas_create(512, 512);

	int origin_x = 20;
	int origin_y = 20;

	double loop_start = smol_timer();


	while(!smol_frame_is_closed(frame)) {

		double time = smol_timer() - loop_start;

		smol_frame_update(frame);
		smol_inputs_flush();

		SMOL_FRAME_EVENT_LOOP(frame, ev) {
			smol_inputs_update(&ev);
		}


		smol_canvas_clear(&canvas, SMOLC_BLACK);
		smol_canvas_set_color(&canvas, SMOLC_SKYBLUE);

		smol_canvas_push_scissor(&canvas);
		smol_canvas_set_scissor(&canvas, 0, 288/4, 512, 288+288/4);
		smol_canvas_draw_image(&canvas, &img, 10, 10);
		smol_canvas_pop_scissor(&canvas);
		//smol_canvas_draw_image_subrect_streched(&canvas, &img, 128, 128, 256+cos(time)*64.f, 128+sin(time)*64.f, 128, 32, 32 + sin(time) * 32.f, 64 + cosf(time) * 32.f);

		smol_canvas_set_color(&canvas, SMOLC_LIMEGREEN);
		smol_canvas_draw_rect(&canvas, 200, 150, 60, 40);

		smol_canvas_set_color(&canvas, SMOLC_HOTPINK);
		smol_canvas_fill_rect(&canvas, 200, 200, 60, 40);

		smol_canvas_set_color(&canvas, SMOLC_ORANGE);

		smol_canvas_push_scissor(&canvas);
		smol_canvas_set_scissor(&canvas, 15, 15, 100, 10);
		smol_canvas_draw_text(&canvas, 10+cosf(time*8.f)*10.f, 10.f + sinf(time * 2.) * 20.f, 1, "Hello, world!");
		smol_canvas_pop_scissor(&canvas);

		smol_canvas_set_color(&canvas, SMOLC_DARK_GREEN);
		//smol_canvas_fill_triangle(&canvas, 100, 150, 50, 200, smol_mouse_x(), smol_mouse_y());
		
		smol_canvas_set_color(&canvas, SMOLC_CYAN);
		smol_canvas_draw_circle(&canvas, smol_mouse_x(), smol_mouse_y(), 10);

		if(smol_mouse_hit(2)) {
			origin_x = smol_mouse_x();
			origin_y = smol_mouse_y();
		}

		if(smol_mouse_down(1)) {

			smol_canvas_push_color(&canvas);
			smol_canvas_set_color(&canvas, SMOLC_AQUA);
			smol_canvas_push_scissor(&canvas);
			smol_canvas_set_scissor(&canvas, 50, 50, 512 - 100, 512 - 100);
			smol_canvas_draw_line(&canvas, origin_x, origin_y, smol_mouse_x(), smol_mouse_y());
			smol_canvas_pop_scissor(&canvas);

			smol_canvas_pop_color(&canvas);
		
		}

		smol_canvas_set_color(&canvas, SMOLC_YELLOW);
		smol_canvas_draw_text_formated(&canvas, 10, 26, 1, "Mouse X: %d\nMouse Y: %d", smol_mouse_x(), smol_mouse_y());
		smol_canvas_draw_text_formated(&canvas, 10, 58, 2, "Time: %f", time);

		smol_canvas_present(&canvas, frame);
	}

	return 0;
}

