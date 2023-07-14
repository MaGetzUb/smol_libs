/*
Copyright © 2023 Marko Ranta (Discord: coderunner)

This software is provided *as-is*, without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/

#ifndef SMOL_CANVAS_H
#define SMOL_CANVAS_H

#ifndef SMOL_FRAME_H
#include "smol_frame.h"
#endif 
#ifndef SMOL_MATH_H
#include "smol_math.h"
#endif 

#include <string.h>

#define SMOL_RGBA(R, G, B, A) (smol_pixel_t){ R, G, B, A }

#define SMOL_RGB(R, G, B)		SMOL_RGBA(R,   G,   B, 255)
#define SMOLC_BLANK				SMOL_RGBA(0,   0,   0,   0)

#define SMOLC_BLACK				SMOL_RGB(  0,   0,   0)
#define SMOLC_WHITE				SMOL_RGB(255, 255, 255)
#define SMOLC_GREY				SMOL_RGB(128, 128, 128)

#define SMOLC_RED				SMOL_RGB(255,   0,   0)
#define SMOLC_GREEN				SMOL_RGB(  0, 255,   0)
#define SMOLC_BLUE				SMOL_RGB(  0,   0, 255)

#define SMOLC_DARK_RED			SMOL_RGB(127,   0,   0)
#define SMOLC_DARK_GREEN		SMOL_RGB(  0, 127,   0)
#define SMOLC_DARK_BLUE			SMOL_RGB(  0,   0, 127)

#define SMOLC_YELLOW			SMOL_RGB(255, 255,   0)
#define SMOLC_CYAN				SMOL_RGB(  0, 255, 255)
#define SMOLC_MAGENTA			SMOL_RGB(255,   0, 255)

#define SMOLC_DARK_YELLOW		SMOL_RGB(127, 127,   0)
#define SMOLC_DARK_CYAN			SMOL_RGB(  0, 127, 127)
#define SMOLC_DARK_MAGENTA		SMOL_RGB(127,   0, 127)

#define SMOLC_ORANGE			SMOL_RGB(255, 127,   0)
#define SMOLC_HOTPINK			SMOL_RGB(255,   0, 127)
#define SMOLC_AQUA				SMOL_RGB(  0, 255, 127)
#define SMOLC_LIMEGREEN			SMOL_RGB(127, 255,   0)
#define SMOLC_VIOLET			SMOL_RGB(127,   0, 255)
#define SMOLC_SKYBLUE			SMOL_RGB( 0,  127, 255)

#define SMOLC_BROWN				SMOL_RGB(127,  63,   0)
#define SMOLC_DARK_HOTPINK		SMOL_RGB(127,   0, 63)
#define SMOLC_DARK_AQUA			SMOL_RGB(  0, 127, 63)
#define SMOLC_DARK_LIMEGREEN	SMOL_RGB(63, 127,   0)
#define SMOLC_DARK_VIOLET		SMOL_RGB(63,   0, 127)
#define SMOLC_DARK_SKYBLUE		SMOL_RGB( 0,  63, 127)

#if _WIN64
typedef unsigned long long smol_size_t;
#else 
typedef unsigned int smol_size_t;
#endif 
typedef unsigned int smol_u32;
typedef unsigned short smol_u16;
typedef unsigned char smol_u8;

typedef struct _smol_canvas_t smol_canvas_t;
typedef union _smol_pixel_t {
	struct { smol_u8  r, g, b, a; };
	smol_u32 pixel;
} smol_pixel_t;

typedef struct _smol_stack_t {
	void* data;
	smol_u32 element_size;
	smol_u32 element_count;
	smol_u32 total_allocation;
} smol_stack_t;

typedef struct _smol_image_t {
	smol_pixel_t* pixel_data;
	void(*free_func)(void*);
	smol_u32 width;
	smol_u32 height;
} smol_image_t;

typedef struct _smol_font_hor_geometry_t {
	char offset_x;
	char width;
} smol_font_hor_geometry_t;

//Basically same as olivec_font, but characters can be non-monospaced.
typedef struct _smol_font_t {
	const char* glyphs;
	int glyph_width;
	int glyph_height;
	smol_font_hor_geometry_t* geometry;
} smol_font_t;

typedef smol_pixel_t(*smol_pixel_blend_func_proc)(smol_pixel_t, smol_pixel_t, smol_u32, smol_u32);

smol_pixel_t smol_pixel_blend_overwrite(smol_pixel_t dst, smol_pixel_t src, smol_u32 x, smol_u32 y) {
	return src;
}

smol_pixel_t smol_pixel_blend_add(smol_pixel_t dst, smol_pixel_t src, smol_u32 x, smol_u32 y) {
	
	smol_u32 r = src.r*src.a + dst.r;
	smol_u32 g = src.g*src.a + dst.g;
	smol_u32 b = src.b*src.a + dst.b;
	smol_u32 a = src.a*src.a + dst.a;

	if(r > 255) r = 255;
	if(g > 255) g = 255;
	if(b > 255) b = 255;
	if(a > 255) a = 255;
	
	return (smol_pixel_t){ (smol_u8)r, (smol_u8)g, (smol_u8)b, (smol_u8)255 };

}

smol_pixel_t smol_pixel_blend_mul(smol_pixel_t dst, smol_pixel_t src, smol_u32 x, smol_u32 y) {
	
	smol_u16 r = (src.r * dst.r) / 255;
	smol_u16 g = (src.g * dst.g) / 255;
	smol_u16 b = (src.b * dst.b) / 255;
	smol_u16 a = (src.a * dst.a) / 255;

	return (smol_pixel_t){ (smol_u8)r, (smol_u8)g, (smol_u8)b, (smol_u8)255 };

}

smol_pixel_t smol_pixel_blend_mix(smol_pixel_t dst, smol_pixel_t src, smol_u32 x, smol_u32 y) {

	smol_u32 isa = 0xFF - src.a;
	smol_u32 sa =  0x00 + src.a;

	smol_u8 r = (sa * src.r + isa * dst.a) / 255U;
	smol_u8 g = (sa * src.g + isa * dst.g) / 255U;
	smol_u8 b = (sa * src.b + isa * dst.b) / 255U;
	smol_u8 a = (sa * src.a + isa * dst.a) / 255U;
	
	return (smol_pixel_t){ r, g, b, 255 };

}

smol_pixel_t smol_pixel_blend_alpha_clip(smol_pixel_t dst, smol_pixel_t src, smol_u32 x, smol_u32 y) {
	if(src.a > 127)
		return (smol_pixel_t){ src.r, src.g, src.b, 255 };
	return dst;
}

smol_image_t smol_image_create_advanced(smol_u32 width, smol_u32 height, smol_pixel_t* buffer, smol_pixel_t color) {

	smol_image_t image = { 0 };

	//When buffer is zero, new is allocated.;
	if(!buffer) {
		smol_size_t num_bytes = width * height * sizeof(smol_pixel_t);
		buffer = malloc(num_bytes);
		for(int i = 0; i < width*height; i++)
			((smol_pixel_t*)buffer)[i] = color;
		image.free_func = &free;
	}

	if(buffer) {
		image.pixel_data = buffer;
		image.width = width;
		image.height = height;
	}

	return image;

}

#define smol_image_create_from_buffer(width, height, buffer) smol_image_create_advanced(width, height, buffer, SMOLC_BLANK)
#define smol_image_create(width, height) smol_image_create_advanced(width, height, NULL, SMOLC_BLANK)
#define smol_image_create_filled(width, height, fill_color) smol_image_create_advanced(width, height, NULL, fill_color)
#define smol_image_pixel_index(image, x, y) image->pixel_data[x + y * image->width]

SMOL_INLINE void smol_image_destroy(smol_image_t* image) {
	if(image->free_func) {
		image->free_func(image->pixel_data);
	}
	image->pixel_data = NULL;
	image->width = 0;
	image->height = 0;
}

SMOL_INLINE void smol_image_putpixel(smol_image_t* img, smol_u32 x, smol_u32 y, smol_pixel_t color) {
	img->pixel_data[x + y * img->width] = color;
}

SMOL_INLINE void smol_image_blendpixel(smol_image_t* img, smol_u32 x, smol_u32 y, smol_pixel_t pixel, smol_pixel_blend_func_proc blend) {
	smol_image_pixel_index(img, x, y) = blend(smol_image_pixel_index(img, x, y), pixel, x, y);
}

SMOL_INLINE smol_pixel_t smol_image_getpixel(smol_image_t* img, smol_u32 x, smol_u32 y) {
	return img->pixel_data[x + y * img->width];
}

typedef struct _smol_canvas_t {
	smol_image_t draw_surface;
	smol_stack_t color_stack;
	smol_stack_t transform_stack;
	smol_stack_t blend_funcs;
} smol_canvas_t;

smol_stack_t smol_stack_create(smol_u32 element_size, smol_u32 element_count) {
	
	smol_stack_t stack = { 0 };
	stack.total_allocation = element_size * element_count;
	stack.element_size = element_size;
	stack.element_count = 0;
	stack.data = malloc(stack.total_allocation);
	
	return stack;

}

#define smol_stack_new(type, element_count) smol_stack_create(sizeof(type), element_count)
#define smol_stack_data(stack, type) ((type*)stack.data)
#define smol_stack_for_each(stack, type, it) for(type* it = stack.data; (it - stack.data) < (stack.element_count*stack.element_size); it += stack.element_size)
#define smol_stack_back(stack, type) ((type*)stack.data)[stack.element_count-1]

void* smol_stack_push(smol_stack_t* stack, void* data) {
	SMOL_ASSERT("BUFFER OVERFLOW!" && (stack->element_count < (stack->total_allocation / stack->element_size)));
	void* dest = (char*)stack->data + stack->element_count++ * stack->element_size;
	return memcpy(
		dest, 
		data, 
		stack->element_size
	);
}

void smol_stack_pop(smol_stack_t* stack) {
	stack->element_count--;
}

void smol_stack_clear(smol_stack_t* stack) {
	stack->element_count = 0;
}

smol_canvas_t smol_canvas_create(smol_u32 width, smol_u32 height) {

	smol_canvas_t canvas = { 0 };
	canvas.draw_surface = smol_image_create(width, height);
	canvas.color_stack = smol_stack_new(smol_pixel_t, 128);
	canvas.transform_stack = smol_stack_new(smol_m3_t, 128);
	canvas.blend_funcs = smol_stack_new(smol_pixel_blend_func_proc, 128);

	smol_stack_push(&canvas.color_stack, &SMOLC_WHITE);
	smol_stack_push(&canvas.blend_funcs, &(smol_pixel_blend_func_proc){ smol_pixel_blend_overwrite });

	return canvas;
}

void smol_canvas_set_color(smol_canvas_t* canvas, smol_pixel_t color) {
	smol_stack_back(canvas->color_stack, smol_pixel_t) = color;
}

void smol_canvas_clear(smol_canvas_t* canvas, smol_pixel_t color) {
	memset(canvas->draw_surface.pixel_data, color.pixel, canvas->draw_surface.width * canvas->draw_surface.height * 4);
}

void smol_canvas_push_color(smol_canvas_t* canvas) {
	smol_stack_push(&canvas->color_stack, &smol_stack_back(canvas->color_stack, smol_pixel_t));
}

void smol_canvas_pop_color(smol_canvas_t* canvas) {
	smol_stack_pop(&canvas->color_stack);
}

void smol_canvas_push_blend(smol_canvas_t* canvas) {
	smol_stack_push(&canvas->blend_funcs, &smol_stack_back(canvas->blend_funcs, smol_pixel_blend_func_proc));
}

void smol_canvas_pop_blend(smol_canvas_t* canvas) {
	smol_stack_pop(&canvas->color_stack);
}

void smol_canvas_draw_pixel(smol_canvas_t* canvas, int x, int y) {
	if(x < 0 || y < 0 || x > (int)canvas->draw_surface.width || y > (int)canvas->draw_surface.height) 
		return;
	smol_pixel_t color = smol_stack_back(canvas->color_stack, smol_pixel_t);
	smol_pixel_blend_func_proc blendfunc = smol_stack_back(canvas->blend_funcs, smol_pixel_blend_func_proc);
	smol_image_blendpixel(&canvas->draw_surface, x, y, color, blendfunc);

}

void smol_canvas_draw_line(smol_canvas_t* canvas, int x0, int y0, int x1, int y1) {

	int cw = canvas->draw_surface.width;
	int ch = canvas->draw_surface.height;
	
	{
		int left =   x0 < x1 ? x0 : x1;
		int right =  x0 > x1 ? x0 : x1;
		int top =    y0 < y1 ? y0 : y1;
		int bottom = y0 > y1 ? y0 : y1;

		//Test if lines bounding box clips the screen rect
		if(!(left < cw && top < ch && right >= 0 && bottom >= 0))
			return;
	}

#define CLIP_LINE_X(xa, ya, xb, yb, edgex, side) \
	if(xa side edgex && xa side xb) \
		ya = ya + ((yb - ya) * (edgex - xa)) / (xb - xa), \
		xa = edgex \


#define CLIP_LINE_Y(xa, ya, xb, yb, edgey, side) \
	if(ya side edgey && ya side yb) \
		xa = xa + (xb - xa) * (edgey - ya) / (yb - ya), \
		ya = edgey \
	


#if 1
	//Clip against left edge
	CLIP_LINE_X(x0, y0, x1, y1, 0, < );
	CLIP_LINE_Y(x0, y0, x1, y1, 0, < );

	CLIP_LINE_X(x1, y1, x0, y0, 0, < );
	CLIP_LINE_Y(x1, y1, x0, y0, 0, < );

	//Clip against right edge
	CLIP_LINE_X(x0, y0, x1, y1, cw-1, > );
	CLIP_LINE_Y(x0, y0, x1, y1, ch-1, > );

	CLIP_LINE_X(x1, y1, x0, y0, cw-1, > );
	CLIP_LINE_Y(x1, y1, x0, y0, ch-1, > );

	if(x0 == x1 && y0 == y1)
		return;

#endif 
#undef CLIP_LINE_X
#undef CLIP_LINE_Y
	
	int dx = (x1 - x0);
	int dy = (y1 - y0);

	int step_x = +1;
	int step_y = -1;

	if(dx < 0) {
		step_x = -step_x;
		dx = -dx;
	}
	if(dy > 0) {
		step_y = -step_y;
		dy = -dy;
	}


	int err = dx + dy;
	int err2 = 0;

	smol_pixel_t color = smol_stack_back(canvas->color_stack, smol_pixel_t);
	smol_pixel_blend_func_proc blendfunc = smol_stack_back(canvas->blend_funcs, smol_pixel_blend_func_proc);

	for(;;) {
		smol_image_blendpixel(&canvas->draw_surface, x0, y0, color, blendfunc);
		err2 = 2 * err;
		if(err2 >= dy) {
			if(x0 == x1) break;
			err += dy;
			x0 += step_x;
		}
		if(err2 <= dx) {
			if(y0 == y1) break;
			err += dx;
			y0 += step_y;
		}
	}

}

void smol_canvas_draw_image(smol_canvas_t* canvas, smol_image_t* image, int x, int y) {

	smol_u32 cw = canvas->draw_surface.width;
	smol_u32 ch = canvas->draw_surface.height;
	int src_x = 0;
	int src_y = 0;
	int dst_w = image->width;
	int dst_h = image->height;

	int left = x;
	int right = x + dst_w;
	int top = y;
	int bottom = y + dst_h;

	if(left < 0) {
		src_x += (-left);
		left += (-left);
	}

	if(top < 0) {
		src_y += (-top);
		top += (-top);
	}

	if(right > (int)cw) {
		right -= cw;
	}
	
	if(bottom > (int)ch) {
		bottom -= ch;
	}

	
	smol_pixel_blend_func_proc blend = smol_stack_back(canvas->blend_funcs, smol_pixel_blend_func_proc);

	for(int py = top, iy = 0; py < bottom; py++, iy++)
	for(int px = left, ix = 0; px < right; px++, ix++) 
	{
		smol_image_blendpixel(&canvas->draw_surface, px, py, smol_image_getpixel(image, src_x+ix, src_y+iy), blend);
	}
}

void smol_canvas_draw_image_subrect_streched(smol_canvas_t* canvas, smol_image_t* image, int x, int y, int dst_w, int dst_h, int src_x, int src_y, int src_w, int src_h) {

	smol_u32 cw = canvas->draw_surface.width;
	smol_u32 ch = canvas->draw_surface.height;

	int left = x;
	int right = x + dst_w;
	int top = y;
	int bottom = y + dst_h;

	if(left < 0) {
		src_x += (-left);
		left += (-left);
	}

	if(top < 0) {
		src_y += (-top);
		top += (-top);
	}

	if(right > (int)cw) {
		//src_w -= (right - cw);
		right -= cw;
	}
	
	if(bottom > (int)ch) {
		//src_h -= (bottom - ch);
		bottom -= ch;
	}

	
	smol_pixel_blend_func_proc blend = smol_stack_back(canvas->blend_funcs, smol_pixel_blend_func_proc);

	for(int py = top, iy = 0; py < bottom; py++, iy++)
	for(int px = left, ix = 0; px < right; px++, ix++) 
	{
		smol_image_blendpixel(&canvas->draw_surface, px, py, smol_image_getpixel(image, src_x+ix*src_w/dst_w, src_y+iy*src_h/dst_h), blend);
	}

}

void smol_canvas_draw_circle(smol_canvas_t* canvas, int xc, int yc, int rad) {

	int x = -rad;
	int y = 0;
	int err = 2 - 2 * rad;

	smol_pixel_t color = smol_stack_back(canvas->color_stack, smol_pixel_t);
	smol_pixel_blend_func_proc blend = smol_stack_back(canvas->blend_funcs, smol_pixel_blend_func_proc);

	do {

		if(yc + y >= 0 && yc - y < (int)canvas->draw_surface.height) {
			smol_image_blendpixel(&canvas->draw_surface, xc - x, yc + y, color, blend);
			smol_image_blendpixel(&canvas->draw_surface, xc - y, yc - x, color, blend);
			smol_image_blendpixel(&canvas->draw_surface, xc - x, yc - y, color, blend);
			smol_image_blendpixel(&canvas->draw_surface, xc - y, yc + x, color, blend);
		}	

		rad = err;
		if(rad <= y) err += (++y << 1) + 1;
		if(rad > x || err > y)
			err += (++x << 1) + 1;
	} while(x < 0);

}

void smol_canvas_fill_circle(smol_canvas_t* canvas, int xc, int yc, int rad) {

	int x = -rad;
	int y = 0;
	int err = 2 - 2 * rad;

	int cw = canvas->draw_surface.width;
	int ch = canvas->draw_surface.height;

	smol_pixel_t color = smol_stack_back(canvas->color_stack, smol_pixel_t);
	smol_pixel_blend_func_proc blend = smol_stack_back(canvas->blend_funcs, smol_pixel_blend_func_proc);

	do {

		//int lx = x;
		//int hx = -x;

		//if((yc - y) < 0 || (yc + y) > surf_h) 
		//	continue;

		//if((xc + lx) < 0) lx = 0-(xc + lx);
		//if((xc + hx) > surf_w) hx = (xc + hx) - surf_w;

		if((yc + y) < (int)canvas->draw_surface.height) {
			int l = x;
			int r = -x;

			if((xc + l) < 0) l = -(xc + l);
			if((xc - x) > cw) r = (xc - x) - cw;

			for(int i = l; i < r; i++) {
				smol_image_blendpixel(&canvas->draw_surface, xc+i, yc + y, color, blend);
			}
		}
		if((yc - y) >= 0) {
			int l = x;
			int r = -x;

			if((xc + l) < 0) l = -(xc + l);
			if((xc - x) > cw) r = (xc - x) - cw;
			for(int i = l; i < r; i++) {
				smol_image_blendpixel(&canvas->draw_surface, xc+i, yc - y, color, blend);
			}
		}

		rad = err;
		if(rad <= y) err += (++y << 1) + 1;
		if(rad > x || err > y)
			err += (++x << 1) + 1;
	} while(x < 0);

}

void smol_canvas_draw_rect(smol_canvas_t* canvas, int x, int y, int w, int h) {

	smol_pixel_t color = smol_stack_back(canvas->color_stack, smol_pixel_t);
	smol_pixel_blend_func_proc blend = smol_stack_back(canvas->blend_funcs, smol_pixel_blend_func_proc);

	int cw = canvas->draw_surface.width;
	int ch = canvas->draw_surface.height;

	int left = (x < 0) ? -x : 0;
	int top = (y < 0) ? -y : 0;
	int right =  ((x + w) < cw) ? w : ((x + w) - cw);
	int bottom = ((y + h) < ch) ? h : ((y + h) - ch);

	for(int i = left; i < right; i++) {
		smol_image_blendpixel(&canvas->draw_surface, x+i, y, color, blend);
		smol_image_blendpixel(&canvas->draw_surface, x+i, y+h-1, color, blend);
	}
	for(int i = top; i < bottom - 2; i++) {
		smol_image_blendpixel(&canvas->draw_surface, x, y+i+1, color, blend);
		smol_image_blendpixel(&canvas->draw_surface, x+w-1, y+i+1, color, blend);
	}

}

void smol_canvas_fill_rect(smol_canvas_t* canvas, int x, int y, int w, int h) {

	smol_pixel_t color = smol_stack_back(canvas->color_stack, smol_pixel_t);
	smol_pixel_blend_func_proc blend = smol_stack_back(canvas->blend_funcs, smol_pixel_blend_func_proc);

	int cw = canvas->draw_surface.width;
	int ch = canvas->draw_surface.height;

	int left = (x < 0) ? -x : 0;
	int top = (y < 0) ? -y : 0;
	int right =  ((x + w) < cw) ? w : ((x + w) - cw);
	int bottom = ((y + h) < ch) ? h : ((y + h) - ch);

	for(int j = left; j < right; j++)
	for(int i = top; i < bottom; i++)
		smol_image_blendpixel(&canvas->draw_surface, x + i, y + j, color, blend);

}

void smol_canvas_fill_triangle(smol_canvas_t* canvas, int x0, int y0, int x1, int y1, int x2, int y2) {

	
	smol_pixel_t color = smol_stack_back(canvas->color_stack, smol_pixel_t);
	smol_pixel_blend_func_proc blend = smol_stack_back(canvas->blend_funcs, smol_pixel_blend_func_proc);

#define TMP_SWAP(a, b) { tmp = a; a = b; b = tmp; }

#define INIT_X(dx, sx) if(dx < 0) sx = -sx, dx = -dx
#define INIT_Y(dy, sy) if(dy > 0) sy = -sy, dy = -dy

#define ITER_X(error, error2, xc, xt, delx, dely, sx) \
	error2 = error+error; \
	if(error2 >= dely) { \
		if(xc == xt) break; \
		error += dely; \
		xc += sx; \
	}

#define ITER_Y(error, error2, yc, yt, delx, dely, sy) \
	if(error2 <= dely) { \
		if(yc == yt) break; \
		error += delx; \
		yc += sy; \
	}	

	int tmp;

	if(y0 > y1) {
		TMP_SWAP(x0, x1);
		TMP_SWAP(y0, y1);
	}

	if(y1 > y2) {
		TMP_SWAP(x1, x2);
		TMP_SWAP(y1, y2);
	}

	if(y0 > y1) {
		TMP_SWAP(x0, x1);
		TMP_SWAP(y0, y1);
	}

	{

		int px[2] = { x0, x0 };
		int py[2] = { y0, y0 };
		int dx[2] = { x1 - px[0], x2 - px[0] };
		int dy[2] = { y1 - py[0], y2 - py[0] };
		int stpx[2] = { 1, 1 };
		int stpy[2] = { -1, -1 };
		int err[2] = { dx[0] + dy[0], dx[1] + dy[1] };
		int err_dbl[2] = { err[0] + err[0], err[1] + err[1] };

		if(x1 > x2) {

			TMP_SWAP(dx[0], dx[1]);
			TMP_SWAP(dy[0], dy[1]);

			TMP_SWAP(stpx[0], stpx[1]);
			TMP_SWAP(stpy[0], stpy[1]);

			TMP_SWAP(err[0], err[1]);
			TMP_SWAP(err_dbl[0], err_dbl[1]);

		}



		INIT_X(dx[0], stpx[0]);
		INIT_Y(dy[0], stpy[0]);

		INIT_X(dx[1], stpx[1]);
		INIT_Y(dy[1], stpy[1]);





		for(;;) {

			for(int i = px[0]; i <= px[1]; i++) {
				smol_image_blendpixel(&canvas->draw_surface, i, py[0], color, blend);
			}

			ITER_X(err[0], err_dbl[0], px[0], x1, dx[0], dy[0], stpx[0])
			ITER_Y(err[0], err_dbl[0], py[0], y1, dx[0], dy[0], stpy[0])

			ITER_X(err[1], err_dbl[1], px[1], x2, dx[1], dy[1], stpx[1])
			ITER_Y(err[1], err_dbl[1], py[1], y2, dx[1], dy[1], stpy[1])
		}

	}

	{

		int px[2] = { x2, x2 };
		int py[2] = { y2, y2 };
		int dx[2] = { x0 - px[0], x1 - px[0] };
		int dy[2] = { y0 - py[0], y1 - py[0] };
		int stpx[2] = { 1, 1 };
		int stpy[2] = { -1, -1 };
		int err[2] = { dx[0] + dy[0], dx[1] + dy[1] };
		int err_dbl[2] = { err[0] + err[0], err[1] + err[1] };


		if(x0 > x1) {

			TMP_SWAP(dx[0], dx[1]);
			TMP_SWAP(dy[0], dy[1]);

			TMP_SWAP(stpx[0], stpx[1]);
			TMP_SWAP(stpy[0], stpy[1]);

			TMP_SWAP(err[0], err[1]);
			TMP_SWAP(err_dbl[0], err_dbl[1]);

		}



		INIT_X(dx[0], stpx[0]);
		INIT_Y(dy[0], stpy[0]);

		INIT_X(dx[1], stpx[1]);
		INIT_Y(dy[1], stpy[1]);


		for(;;) {

			for(int i = px[0]; i <= px[1]; i++) {
				smol_image_blendpixel(&canvas->draw_surface, i, py[0], color, blend);
			}

			ITER_X(err[0], err_dbl[0], px[0], x1, dx[0], dy[0], stpx[0])
			ITER_Y(err[0], err_dbl[0], py[0], y1, dx[0], dy[0], stpy[0])

			ITER_X(err[1], err_dbl[1], px[1], x2, dx[1], dy[1], stpx[1])
			ITER_Y(err[1], err_dbl[1], py[1], y2, dx[1], dy[1], stpy[1])
		}

	}


#undef ITER_X
#undef ITER_Y

#undef INIT_X
#undef INIT_Y

#undef TMP_SWAP
}

void smol_canvas_draw_text(smol_canvas_t* canvas, int tx, int ty, smol_font_t font, int scale, const char *text) {

	int gx = tx;
	for (size_t i = 0; *text; ++i, ++text) {

		int gy = ty;
		const char *glyph = &font.glyphs[(*text)*font.glyph_width*font.glyph_height];

		for(int dy = 0; dy < font.glyph_height; ++dy)
		for(int dx = 0; dx < font.geometry[*text].width+1; ++dx) {

			int px = gx + dx*scale;
			int py = gy + dy*scale;

			if(
				0 <= px && 
				px < (int)canvas->draw_surface.width && 0 <= py && 
				py < (int)canvas->draw_surface.height
			) {
				if (glyph[dy*font.glyph_width + dx + font.geometry[*text].offset_x]) {
					smol_canvas_fill_rect(canvas, px, py, scale, scale);
				}
			}

		}
		
		if(*text == ' ')
			gx += font.geometry['_'].width;
		else 
			gx += (int)((font.geometry[*text].width+2ull)*scale);
	}
}

void smol_canvas_present(smol_canvas_t* canvas, smol_frame_t* frame) {
	smol_frame_blit_pixels(
		frame,
		&canvas->draw_surface.pixel_data->pixel,
		canvas->draw_surface.width,
		canvas->draw_surface.height,
		0, 
		0,
		frame->width, 
		frame->height, 
		0, 
		0,
		canvas->draw_surface.width,
		canvas->draw_surface.height
	);
}

//https://qoiformat.org/qoi-specification.pdf
smol_image_t smol_load_image_qoi(const char* file_path) {

	smol_image_t result = { 0 };
	FILE* file = fopen(file_path, "rb");

	if(!file) {
		return result;
	}

	result.free_func = &free;

	/* HEADER */
	char magic[4] = { 0 };
	smol_u32 width;
	smol_u32 height;
	smol_u8  num_channels;
	smol_u8  color_space;

	fread(magic, 1, 4, file);
	if(*((smol_u32*)&magic[0]) != 'fioq')
		return result;

	fread(&width, 1, 4, file);
	fread(&height, 1, 4, file);
	fread(&num_channels, 1, 1, file);
	fread(&color_space, 1, 1, file);

#define BSWAP32(value) ( \
	((value & 0xFF000000) >> 24) | \
	((value & 0xFF0000) >> 8) | \
	((value & 0xFF00) << 8) | \
	(value << 24) \
)


	width  = BSWAP32(width);
	height = BSWAP32(height);

	smol_pixel_t* pixel_data = (smol_pixel_t*)malloc(width * height * sizeof(smol_pixel_t));
	for(smol_u32 i = 0; i < width * height; i++)
		pixel_data[i].pixel = 0xAABBCCDD;

	smol_u32 pixel_index = 0;

	{

		smol_pixel_t pixel_hashtable[64] = { 0 };
		smol_pixel_t last_pixel = SMOLC_BLACK;
#define HASH_RGBA(r, g, b, a) (r*3 + g*5 + b*7 + a*11) & 0x3F
#define STORE_PIXEL(hash, color) pixel_hashtable[hash] = color
#define HASH_STORE_PIXEL(color) STORE_PIXEL(HASH_RGBA(color.r, color.g, color.b, color.a), color)

		for(;;) {
			smol_u8 tag = fgetc(file);

			switch(tag) {
				case 0x00: {
					char byte = fgetc(file);
					if(byte == 0x1)
						goto end; //End of file
					fseek(file, -1, SEEK_CUR);
				} break;
				case 0xFE: { //RGB

					smol_u8 rgb[3];
					smol_u32 index;
					fread(rgb, 1, 3, file);

					index = HASH_RGBA(rgb[0], rgb[1], rgb[2], 255);
					smol_pixel_t new_pixel = (smol_pixel_t){ rgb[0], rgb[1], rgb[2], 255 };

					pixel_data[pixel_index++] = new_pixel;
					last_pixel = new_pixel;
					STORE_PIXEL(index, new_pixel);

				} continue;
				case 0xFF: { //RGBA
					smol_u8 rgba[4];
					smol_u32 index;
					fread(rgba, 1, 4, file);

					index = HASH_RGBA(rgba[0], rgba[1], rgba[2], rgba[3]);
					smol_pixel_t new_pixel = (smol_pixel_t){ rgba[0], rgba[1], rgba[2], rgba[3] };


					pixel_data[pixel_index++] = new_pixel;
					last_pixel = new_pixel;
					STORE_PIXEL(index, new_pixel);

				} continue;
			}
			switch((tag >> 6) & 0x3) {
				case 0: { //Index
					smol_u8 array_index = tag & 0x3F;
					smol_pixel_t new_pixel = pixel_hashtable[array_index];
					pixel_data[pixel_index++] = new_pixel;
					last_pixel = new_pixel;

				} break;
				case 1: {//Diff
					//Seems to work
					char dr = -2 + ((tag >> 4) & 3);
					char dg = -2 + ((tag >> 2) & 3);
					char db = -2 + ((tag >> 0) & 3);

					smol_pixel_t new_pixel = last_pixel;
					new_pixel.r += dr;
					new_pixel.g += dg;
					new_pixel.b += db;
					new_pixel.a = last_pixel.a;

					pixel_data[pixel_index++] = new_pixel;
					last_pixel = new_pixel;
					HASH_STORE_PIXEL(new_pixel);

				} continue;
				case 2: { //Luma
					//Seems to work
					smol_u8 next_byte = fgetc(file);
					
					char dg = (tag & 0x3F) - 32;
					char dr = dg + ((next_byte >> 4) & 0xF) - 8;
					char db = dg + ((next_byte >> 0) & 0xF) - 8;

					smol_pixel_t new_pixel = last_pixel;
					new_pixel.r += dr;
					new_pixel.g += dg;
					new_pixel.b += db;

					pixel_data[pixel_index++] = new_pixel;
					last_pixel = new_pixel;
					HASH_STORE_PIXEL(new_pixel);
				} continue;
				case 3: { //Run
					//Seems to work now
					smol_u8 run_len = tag & 0x3F;
					
					for(smol_u8 i = 0; i < run_len+1; i++) {
						pixel_data[pixel_index++] = last_pixel;
					}

				} continue;
			}

		}
	}
	end:
	
	result = smol_image_create_from_buffer(width, height, pixel_data);
	result.free_func = free;

#undef HASH_RGBA
#undef STORE_PIXEL
#undef BSWAP32

	fclose(file);

	return result;
}

#endif 