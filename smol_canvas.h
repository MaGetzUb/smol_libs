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

#include <string.h>
#include <stdarg.h>

#ifdef _MSC_VER
#	ifndef SMOL_INLINE
#		define SMOL_INLINE __forceinline
#	endif 
#else 
#	ifndef SMOL_INLINE
#		define SMOL_INLINE inline __attribute__((always_inline)) 
#	endif 
#endif 

//Some predefined color values
#define SMOL_RGB(R, G, B)		smol_rgba(   R,   G,   B, 255)
#define SMOLC_BLANK				smol_rgba(   0,   0,   0,   0)

#define SMOLC_WHITE				SMOL_RGB(255, 255, 255)
#define SMOLC_LIGHT_GREY		SMOL_RGB(192, 192, 192)
#define SMOLC_GREY				SMOL_RGB(128, 128, 128)
#define SMOLC_DARK_GREY			SMOL_RGB( 64,  64,  64)
#define SMOLC_DARKEST_GREY		SMOL_RGB( 32,  32,  32)
#define SMOLC_BLACK				SMOL_RGB(  0,   0,   0)

#define SMOLC_RED				SMOL_RGB(255,   0,   0)
#define SMOLC_DARKENED_RED		SMOL_RGB(192,   0,   0)
#define SMOLC_DARK_RED			SMOL_RGB(127,   0,   0)
#define SMOLC_DARKER_RED		SMOL_RGB( 64,   0,   0)
#define SMOLC_DARKEST_RED		SMOL_RGB( 32,   0,   0)

#define SMOLC_GREEN				SMOL_RGB(0,  255,    0)
#define SMOLC_DARKENED_GREEN	SMOL_RGB(0,  192,    0)
#define SMOLC_DARK_GREEN		SMOL_RGB(0,  127,    0)
#define SMOLC_DARKER_GREEN		SMOL_RGB(0,   64,    0)
#define SMOLC_DARKEST_GREEN		SMOL_RGB(0,   32,    0)

#define SMOLC_BLUE				SMOL_RGB(0,    0,  255)
#define SMOLC_DARKENED_BLUE		SMOL_RGB(0,    0,  192)
#define SMOLC_DARK_BLUE			SMOL_RGB(0,    0,  127)
#define SMOLC_DARKER_BLUE		SMOL_RGB(0,    0,   64)
#define SMOLC_DARKEST_BLUE		SMOL_RGB(0,    0,   32)


#define SMOLC_YELLOW			SMOL_RGB(255, 255,   0)
#define SMOLC_DARK_YELLOW		SMOL_RGB(127, 127,   0)

#define SMOLC_CYAN				SMOL_RGB(  0, 255, 255)
#define SMOLC_DARK_CYAN			SMOL_RGB(  0, 127, 127)

#define SMOLC_MAGENTA			SMOL_RGB(255,   0, 255)
#define SMOLC_DARK_MAGENTA		SMOL_RGB(127,   0, 127)


#define SMOLC_ORANGE			SMOL_RGB(255, 127,   0)
#define SMOLC_BROWN				SMOL_RGB(127,  63,   0)

#define SMOLC_HOTPINK			SMOL_RGB(255,   0, 127)
#define SMOLC_DARK_HOTPINK		SMOL_RGB(127,   0, 63)

#define SMOLC_AQUA				SMOL_RGB(  0, 255, 127)
#define SMOLC_DARK_AQUA			SMOL_RGB(  0, 127, 63)

#define SMOLC_LIMEGREEN			SMOL_RGB(127, 255,   0)
#define SMOLC_DARK_LIMEGREEN	SMOL_RGB(63, 127,   0)

#define SMOLC_VIOLET			SMOL_RGB(127,   0, 255)
#define SMOLC_DARK_VIOLET		SMOL_RGB(63,   0, 127)

#define SMOLC_SKYBLUE			SMOL_RGB( 0,  127, 255)
#define SMOLC_DARK_SKYBLUE		SMOL_RGB( 0,  63, 127)

//TODO:
// - Clip rects
// - Transformations 
// - Smooth drawing

//Some types that are useful
#if _WIN64 || __linux__
typedef unsigned long long smol_size_t;
#else 
typedef unsigned int smol_size_t;
#endif 
typedef char smol_i8;
typedef unsigned char smol_u8;
typedef short smol_i16;
typedef unsigned short smol_u16;
typedef int smol_i32;
typedef unsigned int smol_u32;
typedef long long smol_i64;
typedef unsigned long long smol_u64;
typedef unsigned char smol_byte;

//Forward declare the canvas
typedef struct _smol_canvas_t smol_canvas_t;

#ifndef SMOL_MATH_H
typedef union _smol_m3_t smol_m3_t;
#endif 

//The pixel structure used for color.
//The byte order on Little Endian architectures is [msb] ABGR [lsb]
typedef union _smol_pixel_t {
	struct { smol_u8  r, g, b, a; };
	smol_u32 pixel;
} smol_pixel_t;

//A structure of image
typedef struct _smol_image_t {
	smol_pixel_t* pixel_data;
	void(*free_func)(void*); //This is optional, smol_image_destroy will call this, if images it's set
	smol_u32 width;
	smol_u32 height;
} smol_image_t;

//Contains data for horizontal spacing of a glyph
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

#ifdef __cplusplus
#define smol_create_font(glyphs, glyph_width, glyph_height, geometry) smol_font_t{ glyphs, glyph_width, glyph_height, geometry }
#else 
#define smol_create_font(glyphs, glyph_width, glyph_height, geometry) (smol_font_t){ glyphs, glyph_width, glyph_height, geometry }
#endif 

//The pixel blending render_callback type
typedef smol_pixel_t(*smol_pixel_blend_func_proc)(smol_pixel_t, smol_pixel_t, smol_u32, smol_u32);

//smol_rgba - Initializes a smol_pixel_t structure
// Arguments:
// smol_byte r -- The red channel intensity (between [0..255])
// smol_byte g -- The green channel intensity (between [0..255])
// smol_byte b -- The blue channel intensity (between [0..255])
// smol_byte a -- The alpha channel intensity (between [0..255])
SMOL_INLINE smol_pixel_t smol_rgba(smol_byte r, smol_byte g, smol_byte b, smol_byte a) {
	smol_pixel_t pixel = { 0 };
	pixel.r = r;
	pixel.g = g;
	pixel.b = b;
	pixel.a = a;
	return pixel;
}

SMOL_INLINE smol_pixel_t smol_hsva(smol_u16 hue, smol_byte sat, smol_byte val, smol_byte alp) {

	hue -= ((hue / 360) * 360);

	smol_u16 red = -60 + abs(hue - 180);
	smol_u16 grn = 120 - abs(hue - 120);
	smol_u16 blu = 120 - abs(hue - 240);

	if(red < 0) red = 0;
	if(red > 60) red = 60;

	if(grn < 0) grn = 0;
	if(grn > 60) grn = 60;

	if(blu < 0) blu = 0;
	if(blu > 60) blu = 60;

	red = red * 255 / 60;
	grn = grn * 255 / 60;
	blu = blu * 255 / 60;;

	smol_u8 inv_sat = 255 - sat;

	red = ((((red * sat) / 255) + inv_sat) * val) / 255;
	grn = ((((grn * sat) / 255) + inv_sat) * val) / 255;
	blu = ((((blu * sat) / 255) + inv_sat) * val) / 255;
	
	smol_pixel_t res = { red, grn, blu, alp };

	return res;

}

//smol_image_create_advanced - Creates an image, allocates buffer if no buffer is provided. Clears also the newly allocated buffer with color.
// Arguments:
// - smol_u32 width       -- Width of the new pixel buffer, or or existing one
// - smol_u32 height      -- Height of the new pixel buffer, or existing one
// - smol_pixel_t* buffer -- Pointer to the buffer, if null, function allocates new buffer. Remember to set a free_func from the resulting field.
// - smol_pixel_t color   -- A fill color to fill the newly allocated buffer with, if buffer argument was NULL.
//Returns: smol_image_t containing the resulting image structure.
smol_image_t smol_image_create_advanced(smol_u32 width, smol_u32 height, smol_pixel_t* buffer, smol_pixel_t color);

//smol_image_destroy - Destroys an image, frees the piel buffer, if the free_func was set within the smol_image_t structure. 
// Arguments:
// - smol_image_t* image -- A pointer to the image.
void smol_image_destroy(smol_image_t* image);

//smol_image_putpixel - Puts a pixel into the image
// Arguments:
// - smol_image_t* img  -- A pointer to the image
// - smol_u32 x         -- An X-coordinate within the image
// - smol_u32 y         -- An Y-coordinate within the image
// - smol_pixel_t color -- The color of the pixel
void smol_image_putpixel(smol_image_t* img, smol_u32 x, smol_u32 y, smol_pixel_t color);

//smol_image_blend_pixel - Blends a pixel into the image
// Arguments:
// - smol_image_t* img                -- A pointer to the image
// - smol_u32 x                       -- An X-coordinate within the image
// - smol_u32 y                       -- An Y-coordinate within the image
// - smol_pixel_t pixel               -- Color of the pixel to be blended in
// - smol_pixel_blend_func_proc blend -- A pointer to the blend function
void smol_image_blend_pixel(smol_image_t* img, smol_u32 x, smol_u32 y, smol_pixel_t pixel, smol_pixel_blend_func_proc blend);

//smol_image_getpixel - Samples a pixel from the image.
// Arguments:
// - smol_image_t* img -- Pointer to the image
// - smol_u32 x        -- Coordinate X within the image
// - smol_u32 y        -- Coordinate Y within the image
//Returns: smol_pixel_t containing the color within the image
smol_pixel_t smol_image_getpixel(smol_image_t* img, smol_u32 x, smol_u32 y);

//smol_canvas_create - Creates a new canvas 
// Arguments:
// - smol_u32 width  -- Width of the canvas
// - smol_u32 height -- Height of the canvas
//Returns: smol_canvas_t a structure of the newly created canvas
smol_canvas_t smol_canvas_create(smol_u32 width, smol_u32 height);

//smol_canvas_set_color - Sets current draw color the color of the canvas
// Arguments:
// - smol_canvas_t* canvas -- Pointer to the canvas
// - smol_pixel_t color    -- A pixel color (RGBA) 
void smol_canvas_set_color(smol_canvas_t* canvas, smol_pixel_t color);

//smol_canvas_set_color_rgb - Sets current draw color the color of the canvas using RGB
// Arguments:
// - smol_canvas_t* canvas -- Pointer to the canvas
// - smol_byte r -- Red value between (0...255)
// - smol_byte g -- Green value between (0...255)
// - smol_byte b -- Blue value between (0...255)
void smol_canvas_set_color_rgb(smol_canvas_t* canvas, smol_byte r, smol_byte g, smol_byte b);

//smol_canvas_set_color_rgba - Sets current draw color the color of the canvas using RGBA
// Arguments:
// - smol_canvas_t* canvas -- Pointer to the canvas
// - smol_byte r -- Red value between (0...255)
// - smol_byte g -- Green value between (0...255)
// - smol_byte b -- Blue value between (0...255)
// - smol_byte a -- Alpha value between (0...255)
void smol_canvas_set_color_rgba(smol_canvas_t* canvas, smol_byte r, smol_byte g, smol_byte b, smol_byte a);


//smol_canvas_set_color_hsv - Sets current draw color the color of the canvas using HSV
// Arguments:
// - smol_canvas_t* canvas -- Pointer to the canvas
// - smol_u16 r  -- Hue value between (0...360)
// - smol_byte s -- Saturation value between (0...255)
// - smol_byte v -- Value(brighthness) value between (0...255)
void smol_canvas_set_color_hsv(smol_canvas_t* canvas, smol_u16 h, smol_byte s, smol_byte v);

//smol_canvas_set_color_hsv - Sets current draw color the color of the canvas using HSV
// Arguments:
// - smol_canvas_t* canvas -- Pointer to the canvas
// - smol_u16 r  -- Hue value between (0...360)
// - smol_byte s -- Saturation value between (0...255)
// - smol_byte v -- Value(brighthness) value between (0...255)
// - smol_byte a -- Alpha value between (0...255)
void smol_canvas_set_color_hsva(smol_canvas_t* canvas, smol_u16 h, smol_byte s, smol_byte v, smol_byte a);

//smol_canvas_clear - Clears the canvas with wanted color
// Arguments:
// - smol_canvas_t* canvas -- A pointer to the canvas
// - smol_pixel_t color    -- A pixel color (RGBA) 
void smol_canvas_clear(smol_canvas_t* canvas, smol_pixel_t color);

//smol_canvas_push_color - Pushes the current draw color into the color stack
// Arguments:
// - smol_canvas_t* canvas -- A pointer to the canvas
void smol_canvas_push_color(smol_canvas_t* canvas);

//smol_canvas_pop_color - Pops the draw color from the t of the color stack
// Arguments:
// - smol_canvas_t* canvas -- A pointer to the canvas
void smol_canvas_pop_color(smol_canvas_t* canvas);

//smol_canvas_set_blend - Sets the current blend function in canvas
// Arguments:
// - smol_canvas_t* canvas -- The target canvas
// - smol_pixel_blend_func_proc* blend_func -- The blend function callback
void smol_canvas_set_blend(smol_canvas_t* canvas, smol_pixel_blend_func_proc* blend_func);

//smol_canvas_push_blend - Pushes the current blending operator on t of the blend stack
// Arguments:
// - smol_canvas_t* canvas -- A pointer to the canvas
void smol_canvas_push_blend(smol_canvas_t* canvas);

//smol_canvas_pop_blend - Pops the blending operator from the t of the blend stack
// Arguments:
// - smol_canvas_t* canvas -- A pointer to the canvas
void smol_canvas_pop_blend(smol_canvas_t* canvas);

//smol_canvas_set_font - Sets the current font for the canvas
// Arguments:
// - smol_canvas_t* canvas -- The target canvas
// - smol_font_t* font -- The font to be used
void smol_canvas_set_font(smol_canvas_t* canvas, smol_font_t* font);

//smol_canvas_pop_font - Pushes the current font on the t of the font stack
// Arguments:
// - smol_canvas_t* canvas -- A pointer to the canvas
void smol_canvas_push_font(smol_canvas_t* canvas);

//smol_canvas_pop_font - Pops the current font from the t of the font stack
// Arguments:
// - smol_canvas_t* canvas -- A pointer to the canvas
void smol_canvas_pop_font(smol_canvas_t* canvas);

//smol_canvas_set_scissor - Sets the scissor rect for canvas
// Arguments:
// - smol_canvas_t* canvas -- A pointer to the canvas
// - int x
// - int y
// - int w
// - int h
void smol_canvas_set_scissor(smol_canvas_t* canvas, int x, int y, int w, int h);

//smol_canvas_set_scissor_cascaded - Sets the scissor rect, but isolated within existing rect boundaries.
// Arguments:
// - smol_canvas_t* canvas -- A pointer to the canvas
// - int x
// - int y
// - int w
// - int h
void smol_cavas_set_scissor_cascaded(smol_canvas_t* canvas, int x, int y, int w, int h);

//smol_canvas_draw_pixel - Draws a pixel into the canvas with current color
// Arguments:
// - smol_canvas_t* canvas -- A pointer to the canvas
// - int x                 -- Location X
// - int y                 -- Location Y
void smol_canvas_draw_pixel(smol_canvas_t* canvas, int x, int y);

//smol_canvas_draw_line - Draws a line into the canvas
// Arguments:
// - smol_canvas_t* canvas -- A pointer to the canvas
// - int x0                -- Line starting point on X-Axis
// - int y0                -- Line starting point on Y-Axis
// - int x1                -- Line ending point on X-Axis
// - int y1                -- Line ending point on Y-Axis
void smol_canvas_draw_line(smol_canvas_t* canvas, int x0, int y0, int x1, int y1);

//smol_canvas_draw_image - Draws image into the canvas 
// Arguments:
// - smol_canvas_t* canvas -- A pointer to the canvas
// - smol_image_t* image   -- A pointer to the image
// - int x                 -- Image position on X-axis (t l aligned)
// - int y                 -- Image position on Y-axis (t l aligned)
void smol_canvas_draw_image(smol_canvas_t* canvas, smol_image_t* image, int x, int y);

//smol_canvas_draw_image_subrect_streched - Draws stretched subrect of an image into the canvas
// Arguments:
// - smol_canvas_t* canvas --  A pointer to the canvas
// - smol_image_t* image   --  A pointer to the image
// - int x                 -- Image position on X-axis (t l aligned)
// - int y                 -- Image position on Y-axis (t l aligned)
// - int dst_w             -- The width of the resulting rect angle where a portion of the image is drawn into
// - int dst_h             -- The height of the resulting rect angle where a portion of the image is drawn into
// - int src_x             -- The source location on X-axis within the image
// - int src_y             -- The source location on Y-axis within the image
// - int src_w             -- The horizontal number of pixels within the image
// - int src_h             -- The vertical number of pixels within the image
void smol_canvas_draw_image_subrect_streched(smol_canvas_t* canvas, smol_image_t* image, int x, int y, int dst_w, int dst_h, int src_x, int src_y, int src_w, int src_h);

//smol_canvas_draw_circle - Draw a circle into the canvas
// Arguments:
// - smol_canvas_t* canvas -- A pointer to the canvas
// - int xc                -- Circle's center on X-axis
// - int yc                -- Circle's center on Y-axis
// - int rad               -- Circle's radius
void smol_canvas_draw_circle(smol_canvas_t* canvas, int xc, int yc, int rad);

//smol_canvas_fill_circle - Fills a circle into the canvas
// Arguments:
// - smol_canvas_t* canvas -- A pointer to the canvas
// - int xc                -- Circle's center on X-axis
// - int yc                -- Circle's center on Y-axis
// - int rad               -- Circle's radius
void smol_canvas_fill_circle(smol_canvas_t* canvas, int xc, int yc, int rad);

//smol_canvas_draw_rect - Draws a rectangle into the cavas
// Arguments:
// - smol_canvas_t* canvas -- A pointer to the canvas
// - int x                 -- Rectangle's t l X location
// - int y                 -- Rectangle's t l Y location
// - int w                 -- Width of the rectangle
// - int h                 -- Height of the rectangle
void smol_canvas_draw_rect(smol_canvas_t* canvas, int x, int y, int w, int h);

//smol_canvas_fill_rect - Fills a rectangle into the canvas
// Arguments:
// - smol_canvas_t* canvas -- A pointer to the canvas
// - int x                 -- Rectangle's t l X location
// - int y                 -- Rectangle's t l Y location
// - int w                 -- Width of the rectangle
// - int h                 -- Height of the rectangle
void smol_canvas_fill_rect(smol_canvas_t* canvas, int x, int y, int w, int h);

//smol_canvas_fill_triangle - Fills a triangle into the canvas (FIXME: not working, fix ples)
// Arguments:
// - smol_canvas_t* canvas -- A pointer to the canvas  
// - int x0                -- Vertex 1's location on X-axis
// - int y0                -- Vertex 1's location on Y-axis
// - int x1                -- Vertex 2's location on X-axis
// - int y1                -- Vertex 3's location on Y-axis
// - int x2                -- Vertex 3's location on X-axis
// - int y2                -- Vertex 3's location on Y-axis
void smol_canvas_fill_triangle(smol_canvas_t* canvas, int x0, int y0, int x1, int y1, int x2, int y2);

//smol_canvas_draw_text - Draws a text into the canvas using canvas' current font
// Arguments:
// - smol_canvas_t* canvas -- A pointer to the canvas
// - int tx                -- Top l coordinate on X axis of the text 
// - int ty                -- Top l coordinate on Y axis of the text
// - int scale             -- Scale of the resulting text
// - const char* text      -- The text to be drawn
void smol_canvas_draw_text(smol_canvas_t* canvas, int tx, int ty, int scale, const char* str);

//smol_canvas_draw_text - Draws a formated text into the canvas using canvas' current font
// Arguments:
// - smol_canvas_t* canvas -- A pointer to the canvas
// - int tx                -- Top l coordinate on X axis of the text 
// - int ty                -- Top l coordinate on Y axis of the text
// - int scale             -- Scale of the resulting text
// - const char* fmt       -- The format string to be drawn
// - ...                   -- Values you want to be formated
void smol_canvas_draw_text_formated(smol_canvas_t* canvas, int tx, int ty, int scale, const char* fmt, ...);

//smol_text_size - Calculates bounding box for a text using the current font in the canvas
// Arguments: 
// - smol_canvas_t* canvas -- The canvas that's current font is being used.
// - int scale        -- Scale of the text
// - const char* str  -- Pointer to a c-string
// - int* w           -- Return value for bounding box width
// - int* h           -- Return value for bounding box height
void smol_text_size(smol_canvas_t* font, int scale, const char* str, int* w, int* h);


#ifdef SMOL_FRAME_H
//smol_canvas_present - Presents canvas in the smol frame, requires smol frame to be included in the code before this file
// Arguments:
// - smol_canvas_t* canvas -- Pointer to the canvas
// - smol_frame_t* frame   -- Pointer to the frame
void smol_canvas_present(smol_canvas_t* canvas, smol_frame_t* frame);
#endif 

//smol_load_image_qoi - Loads a qoi image from a file
// Arguments:
// - const char* file_path -- A path to the qoi image file
//Returns: smol_image_t - The result image
smol_image_t smol_load_image_qoi(const char* file_path);

//Blend functions
//smol_pixel_blend_overwrite - Over writes a pixel in the destination
// Arguments:
// - smol_pixel_t dst -- Destination pixel color
// - smol_pixel_t src -- Source pixel color
// - smol_u32 x       -- Location X
// - smol_u32 y       -- Location Y
//Returns: smol_pixel_t - The Result pixel of this blend function
smol_pixel_t smol_pixel_blend_overwrite(smol_pixel_t dst, smol_pixel_t src, smol_u32 x, smol_u32 y);

//smol_pixel_blend_add - Adds the source and the destination pixel together
// Arguments:
// - smol_pixel_t dst -- Destination pixel color
// - smol_pixel_t src -- Source pixel color
// - smol_u32 x       -- Location X
// - smol_u32 y       -- Location Y
//Returns: smol_pixel_t  - The Result pixel of this blend function
smol_pixel_t smol_pixel_blend_add(smol_pixel_t dst, smol_pixel_t src, smol_u32 x, smol_u32 y);

//smol_pixel_blend_mul - Multiplies source and destination pixel together
// Arguments:
// - smol_pixel_t dst -- Destination pixel color
// - smol_pixel_t src -- Source pixel color
// - smol_u32 x       -- Location X
// - smol_u32 y       -- Location Y
//Returns: smol_pixel_t  - The Result pixel of this blend function 
smol_pixel_t smol_pixel_blend_mul(smol_pixel_t dst, smol_pixel_t src, smol_u32 x, smol_u32 y);

//smol_pixel_blend_mix - Does linear interpolation between source and destination
// Arguments:
// - smol_pixel_t dst -- Destination pixel color
// - smol_pixel_t src -- Source pixel color
// - smol_u32 x       -- Location X
// - smol_u32 y       -- Location Y
//Returns: smol_pixel_t  - The Result pixel of this blend function
smol_pixel_t smol_pixel_blend_mix(smol_pixel_t dst, smol_pixel_t src, smol_u32 x, smol_u32 y);

//smol_pixel_blend_alpha_clip - Returns src pixel if source alpha is >= 0, otherwise dst pixel
// Arguments:
// - smol_pixel_t dst -- Destination pixel color
// - smol_pixel_t src -- Source pixel color
// - smol_u32 x       -- Location X
// - smol_u32 y       -- Location Y
//Returns: smol_pixel_t  - The Result pixel of this blend function
smol_pixel_t smol_pixel_blend_alpha_clip(smol_pixel_t dst, smol_pixel_t src, smol_u32 x, smol_u32 y);

//Create image from existing buffer
#define smol_image_create_from_buffer(width, height, buffer) smol_image_create_advanced(width, height, buffer, SMOLC_BLANK)

//Create a new empty image
#define smol_image_create(width, height) smol_image_create_advanced(width, height, NULL, SMOLC_BLANK)

//Create a new color-filled image
#define smol_image_create_filled(width, height, fill_color) smol_image_create_advanced(width, height, NULL, fill_color)

#ifdef SMOL_CANVAS_IMPLEMENTATION

#ifndef SMOL_MATH_H
typedef union _smol_m3_t {
	float m[9];
	struct { float a[3], b[3], c[3]; };
} smol_m3_t;
#endif 

typedef struct _smol_rect_t {
	int left; 
	int top;
	int right;
	int bottom;
} smol_rect_t;


smol_pixel_t smol_pixel_blend_overwrite(smol_pixel_t dst, smol_pixel_t src, smol_u32 x, smol_u32 y) {
	return src;
}

smol_pixel_t smol_pixel_blend_add(smol_pixel_t dst, smol_pixel_t src, smol_u32 x, smol_u32 y) {
	
	smol_u32 r = src.r*src.a + dst.r;
	smol_u32 g = src.g*src.a + dst.g;
	smol_u32 b = src.b*src.a + dst.b;
	smol_u32 a = src.a*src.a + dst.a;

	if(r > 255) r = 255U;
	if(g > 255) g = 255U;
	if(b > 255) b = 255U;
	if(a > 255) a = 255U;
	
	return smol_rgba((smol_u8)r, (smol_u8)g, (smol_u8)b, (smol_u8)255);

}

smol_pixel_t smol_pixel_blend_mul(smol_pixel_t dst, smol_pixel_t src, smol_u32 x, smol_u32 y) {
	
	smol_u16 r = (src.r * dst.r) / 255U;
	smol_u16 g = (src.g * dst.g) / 255U;
	smol_u16 b = (src.b * dst.b) / 255U;
	smol_u16 a = (src.a * dst.a) / 255U;

	return smol_rgba( (smol_u8)r, (smol_u8)g, (smol_u8)b, (smol_u8)255 );

}

smol_pixel_t smol_pixel_blend_mix(smol_pixel_t dst, smol_pixel_t src, smol_u32 x, smol_u32 y) {

	smol_u32 isa = 0xFF - src.a;
	smol_u32 sa =  0x00 + src.a;

	smol_u8 r = (sa * src.r + isa * dst.a) / 255U;
	smol_u8 g = (sa * src.g + isa * dst.g) / 255U;
	smol_u8 b = (sa * src.b + isa * dst.b) / 255U;
	smol_u8 a = (sa * src.a + isa * dst.a) / 255U;
	
	return smol_rgba( r, g, b, 255 );

}

smol_pixel_t smol_pixel_blend_alpha_clip(smol_pixel_t dst, smol_pixel_t src, smol_u32 x, smol_u32 y) {
	if(src.a > 127)
		return smol_rgba( src.r, src.g, src.b, 255 );
	return dst;
}

smol_image_t smol_image_create_advanced(smol_u32 width, smol_u32 height, smol_pixel_t* buffer, smol_pixel_t color) {

	smol_image_t image = { 0 };

	//When buffer is zero, new is allocated.;
	if(!buffer) {
		smol_size_t num_bytes = width * height * sizeof(smol_pixel_t);
		buffer = (smol_pixel_t*)malloc(num_bytes);
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

SMOL_INLINE void smol_image_blend_pixel(smol_image_t* img, smol_u32 x, smol_u32 y, smol_pixel_t pixel, smol_pixel_blend_func_proc blend) {
	smol_image_pixel_index(img, x, y) = blend(smol_image_pixel_index(img, x, y), pixel, x, y);
}

SMOL_INLINE smol_pixel_t smol_image_getpixel(smol_image_t* img, smol_u32 x, smol_u32 y) {
	return img->pixel_data[x + y * img->width];
}

typedef struct _smol_stack_t {
	void* data;
	smol_u32 element_size;
	smol_u32 element_count;
	smol_u32 total_allocation;
} smol_stack_t;


typedef struct _smol_canvas_t {
	smol_image_t draw_surface;
	smol_stack_t color_stack;
	smol_stack_t transform_stack;
	smol_stack_t blend_funcs;
	smol_stack_t font_stack;
	smol_stack_t scissor_stack;
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
	void* dest = (char*)stack->data + (stack->element_count++ * stack->element_size);
	return memcpy(
		dest, 
		data, 
		stack->element_size
	);
}

#ifdef __cplusplus
template <typename T>
SMOL_INLINE void* smol_stack_push_immediate(smol_stack_t* stack, T value) {
	return smol_stack_push(stack, (void*)&value);
}
#else 
#define smol_stack_push_immediate(stack, immediate) smol_stack_push(stack, &(immediate))
#endif 

void smol_stack_pop(smol_stack_t* stack) {
	stack->element_count--;
}

void smol_stack_clear(smol_stack_t* stack) {
	stack->element_count = 0;
}


static smol_font_t smol__default_font = { 0 };
#define PXF_NUM_PRINTABLE_GLYPHS 94
#define SMOL_BUILTIN_FONT_WIDTH 9
#define SMOL_BUILTIN_FONT_HEIGHT 13
static char* smol__builtin_font_data;
static char* smol__builtin_geom_data;
static int smol__builtin_font_refs;

static const char SMOL_BUILTIN_FONT_BLOB[] = 
"AAACAQCAQCAQCAACAAAAAAAoFAoAAAAAAAAAAAAAAACgUPwoFAofhQKAAAAAAAQHBEIA4AhEHAQAAAAAAICiIgICAgIiKAgAAAAAAAAAAAAAAAAAAAAAAAAACAQCAAAAAAAA"
"AAAAAAAAICAQEAgEAgEAQCAIAAAEAQCAIBAIBAICAQEAAAAAABsHD+HBsAAAAAAAAAAAACAQCD+CAQCAAAAAAAAAAAAAAAAAAAQEAAAAAAAAAAAAD+AAAAAAAAAAAAAA"
"AAAAAAAAAQAAAAAAAACAgICAgICAAAAAAAAA4IhEIhEIhEIg4AAAAAACBwCAQCAQCAQPgAAAAAA4IgEAgICAgIB8AAAAAAHBEAgEDAEAhEHAAAAAAAIDAYFAoJB8BAcA"
"AAAAAPhAIBAPAEAhEHAAAAAAAYEBAIB4IhEIg4AAAAAAPhEAgIBAQCAgEAAAAAAA4IhEIg4IhEIg4AAAAAAHBEIhEHgEAgIGAAAAAAAAAAACAAAAAAAQAAAAAAAAAAAQ"
"AAAAAACAgAAAAAAAAICAgIAgCAIAAAAAAAAAAB8AB8AAAAAAAAAAAAAAgCAIAgICAgAAAAAAHBEAgEBAQCAACAAAAAAA8ISaVSqVScIA4AAAAAAGAQCAoFBEPhEdwAAA"
"AAD8IRCIR8IRCIT8AAAAAAHhCQCAQCAQBCHgAAAAAD4IhCIRCIRCIj4AAAAAAfxCIBIPBIIBCfwAAAAAD+IRAJB4JBAIDwAAAAAAHhCQCAQCOQRCHgAAAAADuIhEIh8I"
"hEIjuAAAAAAPgQCAQCAQCAQPgAAAAAA8BAIBAIBCIRBwAAAAAAcxEJBIKBwJBEcwAAAAAD4EAgEAgEAgET+AAAAAAYxENhsKhUIhEdwAAAAADOIhkMhUJhMIjkAAAAAA"
"HBEQSCQSCQREHAAAAAAD8IRCIR8IBAIDwAAAAAAHBEQSCQSCQREHA2AAAAD8IRCIR8JBIIjmAAAAAAPiCQCAPgCASCPgAAAAAD+SQQCAQCAQCA4AAAAAAdxEIhEIhEIh"
"EHAAAAAADuIhEIgoFAoCAQAAAAAAdxEIhEKhUKgoFAAAAAADuIgoFAQFAoIjuAAAAAAdxEIgoFAQCAQHAAAAAAD+QgIBAQEAgIT+AAAAAAHAgEAgEAgEAgEAgHAAAAAQ"
"BAEAQBAEAQAAAAAAAHAIBAIBAIBAIBAIHAACAoIgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA/wAAAgCAAAAAAAAAAAAAAAAAAAAAB4Ah8QiEPQAAAAADAIBAPhCIRCIT8"
"AAAAAAAAAAB8QSAQCCPgAAAAAAMAgEPiEQiEQh+AAAAAAAAAAB8QT+QCCPgAAAAAAYEAgPAgEAgEB4AAAAAAAAAAB+QiEQh8AgEPAAADAIBALBkIhEIjuAAAAAACAAAB"
"wCAQCAQPgAAAAAAIAAAPAIBAIBAIBBwAAAYBAIBMJBQOBIYwAAAAAAwCAQCAQCAQCB8AAAAAAAAAADoKhUKhUawAAAAAAAAAAbBkIhEIjuAAAAAAAAAAB8QSCQSCPgAA"
"AAAAAAAAfhCIRCIR8IDgAAAAAAAB+QiEQiEPgEBwAAAAAAAdgyEAgED4AAAAAAAAAAB8QRwBiCPgAAAAAAAEAgPAgEAgEgYAAAAAAAAAADMIhEIhMGwAAAAAAAAAAdxE"
"IgoFAQAAAAAAAAAADuIhUKgoFAAAAAAAAAAAdxEHA4IjuAAAAAAAAAADuIhEFAoCAQMAAAAAAAAfiICAgIj8AAAAAABgQCAQCBgCAQCAQBgACAQCAQCAQCAQCAQCAQAA"
"AMAQCAQCAMCAQCAQMAAAAAMSSRgAAAAAAAAA=";

static const char BUILTIN_FONT_GEOM_BLOB[] =
"BAEDAwEHAgUBBwAABAEDAwMDAQcBBwMCAQcEAQEHAgUCBQIFAgUCBQIFAgUCBQIFAgUEAQMCAgQCBQMEAgUBBwEHAQcBBwEHAQcBBwEHAQcCBQEGAQcBBwEHAQcBBwEHAQcB"
"BwEHAQcBBwEHAQcBBwEHAQcDAwEHAwMCBQAIAwIBBwEHAQcBBwEHAgQBBwEHAgUCBAEHAgUBBwEHAQcBBwEHAQcBBwIFAQcBBwEHAQcBBwEGAgUEAQIFAQf9=";


#define FIRST_VISIBLE_SYMBOL '!'

SMOL_INLINE int smol_from_base64(char c) {
	if(c >= 'A' && c <= 'Z') return (c - 'A') +  0;
	if(c >= 'a' && c <= 'z') return (c - 'a') + 26;
	if(c >= '0' && c <= '9') return (c - '0') + 52;
	if(c == '+') return 62;
	if(c == '/') return 63;
	return 0;
}

smol_canvas_t smol_canvas_create(smol_u32 width, smol_u32 height) {

	smol_canvas_t canvas = { 0 };
	canvas.draw_surface = smol_image_create(width, height);
	canvas.color_stack = smol_stack_new(smol_pixel_t, 128);
	canvas.transform_stack = smol_stack_new(smol_m3_t, 128);
	canvas.blend_funcs = smol_stack_new(smol_pixel_blend_func_proc, 128);
	canvas.font_stack = smol_stack_new(smol_font_t*, 128);
	canvas.scissor_stack = smol_stack_new(smol_rect_t, 128);

	smol_pixel_t color = SMOLC_WHITE;
	smol_stack_push_immediate(&canvas.color_stack, color);
	
	smol_pixel_blend_func_proc proc = smol_pixel_blend_overwrite;
	smol_stack_push_immediate(&canvas.blend_funcs, proc);

	smol_rect_t scissor = { 0, 0, width, height };
	smol_stack_push_immediate(&canvas.scissor_stack, scissor);

	//This default font stuff is bit hacky.. Maybe there should be some sort of base64 decoder... 
	if(smol__builtin_font_refs == 0) {

		smol__default_font.glyph_width = SMOL_BUILTIN_FONT_WIDTH;
		smol__default_font.glyph_height = SMOL_BUILTIN_FONT_HEIGHT;
		smol__default_font.geometry = NULL;
	
		int num_glyph_pixels = smol__default_font.glyph_width * smol__default_font.glyph_height;;

		int data_size = smol__default_font.glyph_width * smol__default_font.glyph_height * PXF_NUM_PRINTABLE_GLYPHS;
		int alloc_size = smol__default_font.glyph_width * smol__default_font.glyph_height * 128;

		int size = 128 * SMOL_BUILTIN_FONT_WIDTH * SMOL_BUILTIN_FONT_HEIGHT;
		smol__builtin_font_data = memset(malloc(size), 0, size);
		smol__builtin_geom_data = memset(malloc(128 * 2), 0, 128*2);

		char* glyph_data = smol__builtin_font_data;
		smol__default_font.glyphs = smol__builtin_font_data;

		

		int offset = FIRST_VISIBLE_SYMBOL * SMOL_BUILTIN_FONT_WIDTH * SMOL_BUILTIN_FONT_HEIGHT;


		const char* data = SMOL_BUILTIN_FONT_BLOB;
		int index = 0;
		smol_u32 val = 0;
		int bits = 0;

		for(int i = 0;; i++) {
			smol_u32 bits = 
				smol_from_base64(data[0]) << 0x12 |
				smol_from_base64(data[1]) << 0x0C | 
				smol_from_base64(data[2]) << 0x06 | 
				smol_from_base64(data[3]) << 0x00
			;
			for(int j = 0; j < 24 && index < data_size; j++) {
			
				int pix = (bits & (0x800000 >> j)) > 0;
	
				glyph_data[offset + index++] = pix;

				char title[2] = { FIRST_VISIBLE_SYMBOL + (index / num_glyph_pixels), 0 };
			}

			if(index >= data_size)
				break;

			data += 4;
		}

		smol_font_t* font[] = { &smol__default_font };
		smol__default_font.geometry = smol__builtin_geom_data;
		data = BUILTIN_FONT_GEOM_BLOB;
		int geoms = 0;
		for(int i = 0; i < PXF_NUM_PRINTABLE_GLYPHS; i++) {
			smol_u32 bits = 
				smol_from_base64(data[0]) << 0x12 |
				smol_from_base64(data[1]) << 0x0C | 
				smol_from_base64(data[2]) << 0x06 | 
				smol_from_base64(data[3]) << 0x00
			;
			
			for(int i = 16; i >= 0; i-=8) {
				smol__builtin_geom_data[FIRST_VISIBLE_SYMBOL*2+geoms++] = (smol_u8)(((bits) >> i) & 0xFF);
				if(geoms > PXF_NUM_PRINTABLE_GLYPHS * 2)
					goto exit;
			}
			
			data += 4;
		}
		exit:


		smol_stack_push(&canvas.font_stack, font);
		smol__builtin_font_refs++;
	}

	return canvas;
}

void smol_canvas_set_color(smol_canvas_t* canvas, smol_pixel_t color) {
	smol_stack_back(canvas->color_stack, smol_pixel_t) = color;
}

void smol_canvas_set_color_rgb(smol_canvas_t* canvas, smol_byte r, smol_byte g, smol_byte b) {
	smol_stack_back(canvas->color_stack, smol_pixel_t) = smol_rgba(r, g, b, 255);
}

void smol_canvas_set_color_rgba(smol_canvas_t* canvas, smol_byte r, smol_byte g, smol_byte b, smol_byte a) {
	smol_stack_back(canvas->color_stack, smol_pixel_t) = smol_rgba(r, g, b, a);
}

void smol_canvas_set_color_hsv(smol_canvas_t* canvas, smol_u16 h, smol_byte s, smol_byte v) {
	smol_stack_back(canvas->color_stack, smol_pixel_t) = smol_hsva(h, s, v, 255);
}

void smol_canvas_set_color_hsva(smol_canvas_t* canvas, smol_u16 h, smol_byte s, smol_byte v, smol_byte a) {
	smol_stack_back(canvas->color_stack, smol_pixel_t) = smol_hsva(h, s, v, a);
}

void smol_canvas_darken_color(smol_canvas_t* canvas, smol_u16 percentage) {
	
	smol_pixel_t color = smol_stack_back(canvas->color_stack, smol_pixel_t);
	
	smol_u8 r = ((color.r * percentage) / 100);
	smol_u8 g = ((color.g * percentage) / 100);
	smol_u8 b = ((color.a * percentage) / 100);
	
	color.r = (color.r < r) ? 0 : color.r - r;
	color.g = (color.g < g) ? 0 : color.g - g;
	color.b = (color.b < b) ? 0 : color.b - b;

	smol_stack_back(canvas->color_stack, smol_pixel_t) = smol_rgba(color.r, color.g, color.b, color.a);
}


void smol_canvas_lighten_color(smol_canvas_t* canvas, smol_u16 percentage) {
	
	smol_pixel_t color = smol_stack_back(canvas->color_stack, smol_pixel_t);
	
	smol_u16 r = ((color.r * percentage) / 100);
	smol_u16 g = ((color.g * percentage) / 100);
	smol_u16 b = ((color.b * percentage) / 100);
	
	smol_u16 cr = color.r + r;
	smol_u16 cg = color.g + g;
	smol_u16 cb = color.b + b;

	if(cr > 255) cr = 255;
	if(cg > 255) cg = 255;
	if(cb > 255) cb = 255;

	smol_stack_back(canvas->color_stack, smol_pixel_t) = smol_rgba(cr, cg, cb, color.a);
}

void smol_canvas_clear(smol_canvas_t* canvas, smol_pixel_t color) {
	for(smol_size_t i = 0; i < canvas->draw_surface.width * canvas->draw_surface.height; i++) {
		canvas->draw_surface.pixel_data[i] = color;
	}
}

void smol_canvas_push_color(smol_canvas_t* canvas) {
	smol_stack_push(&canvas->color_stack, &smol_stack_back(canvas->color_stack, smol_pixel_t));
}

void smol_canvas_pop_color(smol_canvas_t* canvas) {
	smol_stack_pop(&canvas->color_stack);
}

void smol_canvas_set_blend(smol_canvas_t* canvas, smol_pixel_blend_func_proc* blend_func) {
	smol_stack_back(canvas->blend_funcs, smol_pixel_blend_func_proc) = blend_func;
}


void smol_canvas_push_blend(smol_canvas_t* canvas) {
	smol_stack_push(&canvas->blend_funcs, &smol_stack_back(canvas->blend_funcs, smol_pixel_blend_func_proc));
}

void smol_canvas_pop_blend(smol_canvas_t* canvas) {
	smol_stack_pop(&canvas->color_stack);
}

void smol_canvas_set_font(smol_canvas_t* canvas, smol_font_t* font) {
	if(canvas->font_stack.element_count == 0)
		smol_stack_push(&canvas->font_stack, &font);
	else 
		smol_stack_back(canvas->font_stack, smol_font_t*) = font;
}

void smol_canvas_push_font(smol_canvas_t* canvas) {
	smol_stack_push(&canvas->font_stack, &smol_stack_back(canvas->blend_funcs, smol_font_t*));
}

void smol_canvas_pop_font(smol_canvas_t* canvas) {
	smol_stack_pop(&canvas->font_stack);
}

void smol_canvas_push_scissor(smol_canvas_t* canvas) {
	smol_stack_push(&canvas->scissor_stack, &smol_stack_back(canvas->color_stack, smol_pixel_t));
}

void smol_canvas_pop_scissor(smol_canvas_t* canvas) {
	smol_stack_pop(&canvas->scissor_stack);
}

void smol_canvas_set_scissor(smol_canvas_t* canvas, int x, int y, int w, int h) {

	int cw = canvas->draw_surface.width;
	int ch = canvas->draw_surface.height;

	if(x < 0) x = 0;
	if(y < 0) y = 0;
	if(x + w > cw) w = (x + w) - cw;
	if(y + h > ch) h = (y + h) - ch;

	smol_rect_t rect = { x, y, x + w, y + h };
	smol_stack_back(canvas->scissor_stack, smol_rect_t) = rect;
}

void smol_cavas_set_scissor_cascaded(smol_canvas_t* canvas, int x, int y, int w, int h) {

	if(canvas->scissor_stack.element_count == 1) {

		int cw = canvas->draw_surface.width;
		int ch = canvas->draw_surface.height;

		if(x < 0) x = 0;
		if(y < 0) y = 0;
		if(x + w > cw) w = (x + w) - cw;
		if(y + h > ch) h = (y + h) - ch;

	} else {

		smol_rect_t rct = smol_stack_back(canvas->scissor_stack, smol_rect_t);

		if(x < rct.left) x = rct.left;
		if(y < rct.top) y = rct.top;
		if(x + w >= rct.right) w = (x + w) - rct.right;
		if(y + h >= rct.right) h = (y + h) - rct.bottom;

	}

	smol_rect_t rect = { x, y, x + w, y + h };
	smol_stack_back(canvas->scissor_stack, smol_rect_t) = rect;

}


void smol_canvas_draw_pixel(smol_canvas_t* canvas, int x, int y) {
	
	int left = 0;
	int top = 0;
	int right = canvas->draw_surface.width;
	int bottom = canvas->draw_surface.height;

	if(x < left || y < top || x > right || y > bottom) 
		return;
	smol_pixel_t color = smol_stack_back(canvas->color_stack, smol_pixel_t);
	smol_pixel_blend_func_proc blendfunc = smol_stack_back(canvas->blend_funcs, smol_pixel_blend_func_proc);
	smol_image_blend_pixel(&canvas->draw_surface, x, y, color, blendfunc);

}

void smol_canvas_draw_line(smol_canvas_t* canvas, int x0, int y0, int x1, int y1) {

	smol_rect_t rect = smol_stack_back(canvas->scissor_stack, smol_rect_t);

	int left = rect.left;
	int top = rect.top;
	int right = rect.right;
	int bottom = rect.bottom;


	{
		int l =   x0 < x1 ? x0 : x1;
		int r =  x0 > x1 ? x0 : x1;
		int t =    y0 < y1 ? y0 : y1;
		int b = y0 > y1 ? y0 : y1;

		//Test if lines bounding box clips the screen rect
		if(!(l < right && t < bottom && r >= left && b >= top))
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
	//Clip against l edge
	CLIP_LINE_X(x0, y0, x1, y1, left, < );
	CLIP_LINE_Y(x0, y0, x1, y1, top, < );

	CLIP_LINE_X(x1, y1, x0, y0, left, < );
	CLIP_LINE_Y(x1, y1, x0, y0, top, < );

	//Clip against r edge
	CLIP_LINE_X(x0, y0, x1, y1, right-1, > );
	CLIP_LINE_Y(x0, y0, x1, y1, bottom-1, > );

	CLIP_LINE_X(x1, y1, x0, y0, right-1, > );
	CLIP_LINE_Y(x1, y1, x0, y0, bottom-1, > );

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
		smol_image_blend_pixel(&canvas->draw_surface, x0, y0, color, blendfunc);
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

	smol_rect_t rect = smol_stack_back(canvas->scissor_stack, smol_rect_t);

	int left = rect.left;
	int top = rect.top;
	int right = rect.right;
	int bottom = rect.bottom;

	int src_x = 0;
	int src_y = 0;
	int dst_w = image->width;
	int dst_h = image->height;

	int l = x;
	int r = l + dst_w;
	int t = y;
	int b = t + dst_h;

	if(l < left) {
		src_x += left-l;
		l += left-l;
	}

	if(t < top) {
		src_y += top-t;
		t += top-t;
	}

	if(r > (int)right) {
		r -= (r - right);
	}
	
	if(b > (int)bottom) {
		b -= (b - bottom);
	}

	
	smol_pixel_blend_func_proc blend = smol_stack_back(canvas->blend_funcs, smol_pixel_blend_func_proc);

	for(int py = t, iy = 0; py < b && iy < dst_h; py++, iy++)
	for(int px = l, ix = 0; px < r && ix < dst_w; px++, ix++) 
	{
		smol_image_blend_pixel(&canvas->draw_surface, px, py, smol_image_getpixel(image, src_x+ix, src_y+iy), blend);
	}
}

void smol_canvas_draw_image_subrect_streched(smol_canvas_t* canvas, smol_image_t* image, int x, int y, int dst_w, int dst_h, int src_x, int src_y, int src_w, int src_h) {
	
	smol_rect_t rect = smol_stack_back(canvas->scissor_stack, smol_rect_t);

	int left = rect.left;
	int top = rect.top;
	int right = rect.right;
	int bottom = rect.bottom;

	int l = x;
	int r = x + dst_w;
	int t = y;
	int b = y + dst_h;

	if(l < left) {
		src_x += left-l;
		l += left-l;
	}

	if(t < top) {
		src_y += top-t;
		t += top-t;
	}

	if(r > right) {
		//src_w -= (r - cw);
		r -= right;
	}
	
	if(b > bottom) {
		//src_h -= (b - ch);
		b -= bottom;
	}

	
	smol_pixel_blend_func_proc blend = smol_stack_back(canvas->blend_funcs, smol_pixel_blend_func_proc);

	for(int py = t, iy = 0; py < b; py++, iy++)
	for(int px = l, ix = 0; px < r; px++, ix++) 
	{
		smol_image_blend_pixel(&canvas->draw_surface, px, py, smol_image_getpixel(image, src_x+ix*src_w/dst_w, src_y+iy*src_h/dst_h), blend);
	}

}

void smol_canvas_draw_circle(smol_canvas_t* canvas, int xc, int yc, int rad) {

	smol_rect_t rect = smol_stack_back(canvas->scissor_stack, smol_rect_t);

	int left = rect.left;
	int top = rect.top;
	int right = rect.right;
	int bottom = rect.bottom;

	int x = -rad;
	int y = 0;
	int err = 2 - 2 * rad;

	smol_pixel_t color = smol_stack_back(canvas->color_stack, smol_pixel_t);
	smol_pixel_blend_func_proc blend = smol_stack_back(canvas->blend_funcs, smol_pixel_blend_func_proc);

	do {


		if((xc - y) >= left && (yc + x) > top && (xc - y) < right && (yc + x) < bottom) 
			smol_image_blend_pixel(&canvas->draw_surface, xc - y, yc + x, color, blend);

		if((xc - y) >= left && (yc - x) > top && (xc - y) < right && (yc - x) < bottom)
			smol_image_blend_pixel(&canvas->draw_surface, xc - y, yc - x, color, blend);

		if((xc - x) >= left && (yc + y) > top && (xc - x) < right && (yc + y) < bottom) 
			smol_image_blend_pixel(&canvas->draw_surface, xc - x, yc + y, color, blend);

		if((xc - x) >= left && (yc - y) > top && (xc - x) < right && (yc - y) < bottom) 
			smol_image_blend_pixel(&canvas->draw_surface, xc - x, yc - y, color, blend);
		

		rad = err;
		if(rad <= y) err += (++y << 1) + 1;
		if(rad > x || err > y)
			err += (++x << 1) + 1;
	} while(x < 0);

}

void smol_canvas_fill_circle(smol_canvas_t* canvas, int xc, int yc, int rad) {


	smol_rect_t rect = smol_stack_back(canvas->scissor_stack, smol_rect_t);

	int left = rect.left;
	int top = rect.top;
	int right = rect.right;
	int bottom = rect.bottom;

	int x = -rad;
	int y = 0;
	int err = 2 - 2 * rad;

	smol_pixel_t color = smol_stack_back(canvas->color_stack, smol_pixel_t);
	smol_pixel_blend_func_proc blend = smol_stack_back(canvas->blend_funcs, smol_pixel_blend_func_proc);

	do {

		if((yc - y) >= 0 && (yc - y) < (int)canvas->draw_surface.height) {

			int l = xc+x;
			int r = xc-x;

			if(l < left) l = left;
			if(r >= right) r = right - 1;
		
			for(int i = l; i < r; i++) {
				smol_image_blend_pixel(&canvas->draw_surface, i, yc - y, color, blend);
			}
		}

		if((yc + y) >= 0 && (yc + y) < (int)canvas->draw_surface.height) {

			int l = xc+x;
			int r = xc-x;

			if(l < left) l = left;
			if(r >= right) r = right - 1;
			
			for(int i = l; i < r; i++) {
				smol_image_blend_pixel(&canvas->draw_surface, i, yc + y, color, blend);
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
	smol_rect_t rect = smol_stack_back(canvas->scissor_stack, smol_rect_t);

	int left = rect.left;
	int top = rect.top;
	int right = rect.right;
	int bottom = rect.bottom;

	int l = (x < left) ? -x : left;
	int t = (y < top) ? -y : top;
	int r =  ((x + w) < right) ? w : ((x + w) - right);
	int b = ((y + h) < bottom) ? h : ((y + h) - bottom);

	for(int i = l; i < r; i++) {
		smol_image_blend_pixel(&canvas->draw_surface, x+i, y, color, blend);
		smol_image_blend_pixel(&canvas->draw_surface, x+i, y+h-1, color, blend);
	}
	for(int i = t; i < b - 2; i++) {
		smol_image_blend_pixel(&canvas->draw_surface, x, y+i+1, color, blend);
		smol_image_blend_pixel(&canvas->draw_surface, x+w-1, y+i+1, color, blend);
	}

}

void smol_canvas_fill_rect(smol_canvas_t* canvas, int x, int y, int w, int h) {

	smol_pixel_t color = smol_stack_back(canvas->color_stack, smol_pixel_t);
	smol_pixel_blend_func_proc blend = smol_stack_back(canvas->blend_funcs, smol_pixel_blend_func_proc);
	smol_rect_t rect = smol_stack_back(canvas->scissor_stack, smol_rect_t);

	int left = rect.left;
	int top = rect.top;
	int right = rect.right;
	int bottom = rect.bottom;

	int l = (x < left) ? left : x;
	int t = (y < top) ? top : y;
	int r = ((l + w) < right) ? x+w : l+((l + w) - right);
	int b = ((t + h) < bottom) ? y+h : t+((t + h) - bottom);

	for(int py = t; py < b; py++)
	for(int px = l; px < r; px++)
		smol_image_blend_pixel(&canvas->draw_surface, px, py, color, blend);

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
				smol_image_blend_pixel(&canvas->draw_surface, i, py[0], color, blend);
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
				smol_image_blend_pixel(&canvas->draw_surface, i, py[0], color, blend);
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

void smol_canvas_draw_text(smol_canvas_t* canvas, int tx, int ty, int scale, const char* str) {

	smol_font_t* font = smol_stack_back(canvas->font_stack, smol_font_t*);
	smol_pixel_t color = smol_stack_back(canvas->color_stack, smol_pixel_t);
	smol_pixel_blend_func_proc blend = smol_stack_back(canvas->blend_funcs, smol_pixel_blend_func_proc);
	smol_rect_t rect = smol_stack_back(canvas->scissor_stack, smol_rect_t);

	int space = font->geometry ? font->geometry['_'].width : font->glyph_width;

	int gx = tx;
	int gy = ty;
	for(; *str; ++str) {

		if(*str == ' ') {
			gx += space;
			continue;
		} else if(*str == '\t') {
			gx += space * 4;
			continue;
		} else if(*str == '\n') {
			gx = tx;
			gy += font->glyph_height * scale;
			continue;
		}

		const char* glyph = &font->glyphs[(*str) * font->glyph_width * font->glyph_height];

		int char_offset_x = 0;
		int char_w = font->glyph_width;

		if(font->geometry) {
			smol_font_hor_geometry_t geom = font->geometry[*str];
			char_w = geom.width+1;
			char_offset_x = geom.offset_x;
		}

		int l = gx; 
		int t = gy;
		int r = l + char_w * scale;
		int b = t + font->glyph_height * scale;
		
		int sx = char_offset_x*scale;
		int sy = 0;

		if(l < rect.left) 
			sx += (rect.left - l) / scale, l = rect.left;
		if(t < rect.top) 
			sy = (rect.top - t) / scale, t = rect.top;
		if(r >= rect.right) 
			r = rect.right;
		if(b >= rect.bottom) 
			b = rect.bottom;

		for(int dy = t, vy = sy; dy < b; ++dy, ++vy)
		for(int dx = l, vx = sx; dx < r; ++dx, ++vx) 
		{

			int pix_x = vx / scale;
			int pix_y = vy / scale;

			if(glyph[pix_x + pix_y * font->glyph_width]) {
				smol_image_blend_pixel(&canvas->draw_surface, dx, dy, color, blend);
			}

		}

		gx += font->geometry ? (int)((font->geometry[*str].width+2ull)*scale) : char_w*scale;

	}
}

void smol_canvas_draw_text_formated(smol_canvas_t* canvas, int tx, int ty, int scale, const char* fmt, ...) {

	static char buffer[4096] = { 0 };
	va_list list;
	va_start(list, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, list);
	va_end(list);

	smol_canvas_draw_text(canvas, tx, ty, scale, buffer);

}

void smol_text_size(smol_canvas_t* canvas, int scale, const char* str, int* w, int* h) {

	smol_font_t* font = smol_stack_back(canvas->font_stack, smol_font_t*);

	*w = 0;
	*h = 0;

	int x = 0;
	int y = 0;
	const char* s = str;

	for(;*s;s++) {
		
		switch(*s) {
			case '\n':
				x = 0;
				y += font->glyph_height * scale;
			break;
			case ' ': x += font->geometry['_'].width; break;
			case '\t': x += font->geometry['_'].width * 4; break;
			default: x += (font->geometry[*s].width + 2); break;
		}
		
		*w = (x > *w) ? x : *w;
		*h = (y > *h) ? y : *h;
	}

	*w = *w * scale;
	*h += font->glyph_height*scale;
}




#ifdef SMOL_FRAME_H
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
#endif 

//https://qoiformat.org/qoi-specification.pdf
smol_image_t smol_load_image_qoi(const char* file_path) {

	smol_image_t result = { 0 };
	FILE* file = NULL;
#ifndef _CRT_SECURE_NO_WARNINGS
	fopen_s(&file, file_path, "rb");
#else 
	file = fopen(file_path, "rb");
#endif 

	if(!file) {
		fprintf(stderr, "Unable to open file '%s'!\n", file_path);
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
					smol_pixel_t new_pixel = smol_rgba( rgb[0], rgb[1], rgb[2], 255 );

					pixel_data[pixel_index++] = new_pixel;
					last_pixel = new_pixel;
					STORE_PIXEL(index, new_pixel);

				} continue;
				case 0xFF: { //RGBA
					smol_u8 rgba[4];
					smol_u32 index;
					fread(rgba, 1, 4, file);

					index = HASH_RGBA(rgba[0], rgba[1], rgba[2], rgba[3]);
					smol_pixel_t new_pixel = smol_rgba( rgba[0], rgba[1], rgba[2], rgba[3] );


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

#endif 