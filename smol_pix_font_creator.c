#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

#define SMOL_FRAME_IMPLEMENTATION
#include "smol_frame.h"

#define SMOL_INPUT_IMPLEMENTATION
#include "smol_input.h"

#define SMOL_CANVAS_IMPLEMENTATION
#include "smol_canvas.h"

#include "thirdparty/tinyfiledialogs.h"

//I build this file on Windows with these arguments:
//cl.exe /Zi /EHsc /nologo /Fo:.\build\ .\smol_pix_font_creator.c /link kernel32.lib user32.lib comdlg32.lib ole32.lib shell32.lib /OUT:.\build\smol_pix_font_creator.exe
int num_chars = 0;
int char_w, char_h;

char* pix_buffer = NULL;
char* offsets[128] = {0};
char* indexes = NULL;
char rev_indexes[128];
char char_geometry[128][2];


void load_font(const char* path);
void save_font(const char* path);
void export_c_header(const char* path);
void export_c_blob(const char* path);

BOOL SaveBitmapToFile(const char* filename, HBITMAP hBitmap);

#ifdef UNICODE
#	define tprintf wprintf
#else 
#	define tprintf printf
#endif 
#define QUICK_TEST 1

int main() {



	int stage = 0;
	int index = 0;

	puts("Type 1 (enter) to create a new font.");
	puts("Type 2 (enter) to load an existing font.");
#if _WIN32
	puts("Type 3 (enter) to create font from Windows fonts.");
#endif
	printf("How would you like to proceed: ");
#if !QUICK_TEST
	scanf("%d", &stage);
#else
	stage = 3;
#endif 
	if(stage == 1) {
		puts("Max character size is 32x32");

		printf("Char width: ");
		scanf("%d", &char_w);

		while(char_w > 32) {
			puts("Pick a width that's less  than or equal to 32 pixels!");
			printf("Char width: ");
			if(scanf("%d", &char_w) == 0) exit(0);
		}

		printf("Char height: ");
		scanf("%d", &char_h);
		while(char_h > 32) {
			puts("Pick a height that's less  than or equal to 32 pixels!");
			printf("Char height: ");
			if(scanf("%d", &char_h) == 0) exit(0);
		}

		printf("Char size will be %d x %d pixels\n", char_w, char_h);

		for(int i = 0; i < 128; i++) {
			if(isprint(i) && !isspace(i)) {
				num_chars++;
			}
		}

		pix_buffer = malloc(char_w * char_h * num_chars);
		indexes = malloc(num_chars);

		memset(pix_buffer, 0, char_w * char_h * num_chars);
		memset(indexes, 0, num_chars);	

	} else if(stage == 2) {

		const char* exts[] = { "*.pxf" };
		char* path = tinyfd_openFileDialog("Open a pix font", "", 1, exts, exts[0], 0);
		if(!path)
			exit(0);
		load_font(path);

	}
#if _WIN32
	else if(stage == 3) {

		char buf[128] = { 0 };
		char pxf_buf[128] = { 0 };
		char header_buf[128] = { 0 };
		int chars = 0;
		
		int font_loaded = 0;
		while(!font_loaded) {

		#if !QUICK_TEST
			printf("Type in type-face name or font file name: ");
			scanf("%s", buf);
			chars = strlen(buf);


			printf("Char size: ");
			scanf("%d", &char_w);

			while(char_w > 32) {
				puts("Pick a width that's less  than or equal to 32 pixels!");
				printf("Char width: ");
				if(scanf("%d", &char_w) == 0) exit(0);
			}

			char_h = char_w;

			printf("Char size will be %d x %d pixels\n", char_w, char_h);
		#else 
			strcpy(buf, "Courier");
			chars = 7;
			char_h = char_w = 12;
		#endif 
		
		#define CHARS_PER_SIDE 16

			//NONANTIALIASED_QUALITY
			int last_error = GetLastError();
	#ifdef UNICODE
			TCHAR typeface[128] = { 0 };
			MultiByteToWideChar(CP_UTF8, 0, buf, chars, typeface, 128);
	#else
			TCHAR* typeface = buf;
	#endif 
			HDC hdcScreen = GetDC(NULL);
			

			// Set the desired font attributes (not rotated)
			LOGFONT lf = { 0 };
			
			lf.lfHeight = char_h; // Font height
			lf.lfWeight = FW_THIN; // Font weight
			lf.lfCharSet = DEFAULT_CHARSET; // Character set
			lf.lfQuality = NONANTIALIASED_QUALITY;
			//tstrcpy_s(lf.lfFaceName, LF_FACESIZE, typeface); // Font face name
			memcpy(lf.lfFaceName, typeface, LF_FACESIZE * sizeof(TCHAR));

			// Create the font
			HFONT font = CreateFontIndirect(&lf);



			last_error = GetLastError();
			if(last_error) {
				TCHAR err_buf[512] = { 0 };
				FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, last_error, 0, err_buf, 511, NULL);
				tprintf(TEXT("%s\n"), err_buf);

				printf("FAILED to load font '%s'\n", buf);
				continue;
			}
			font_loaded = 1;

			// Create a device context
			HDC hdcBmp = CreateCompatibleDC(hdcScreen);

			
			HFONT old = (HFONT)SelectObject(hdcBmp, font);

			
			int cw = 0;
			int ch = 0;
			for(int i = 0; i < 128; i++) {
				char text[1] = { i };
				RECT bounds = { 0 };
				GetTextExtentPoint32(hdcBmp, text, 1, &bounds);
				int w = abs(bounds.right - bounds.left);
				int h = abs(bounds.bottom - bounds.top);
				if(w > cw) cw = w;
				if(h > ch) ch = h;
			}
			char_w = cw;
			char_h = ch;

			int atlas_w = CHARS_PER_SIDE * char_w;
			int atlas_h = (CHARS_PER_SIDE / 2) * char_h;

			BITMAPINFO bmpInfo = { 0 };
			bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmpInfo.bmiHeader.biWidth = atlas_w;
			bmpInfo.bmiHeader.biHeight = -atlas_h; // negative height for a top-down bitmap (origin at top-left)
			bmpInfo.bmiHeader.biPlanes = 1;
			bmpInfo.bmiHeader.biBitCount = 32; // 32 bits per pixel (RGBA)
			bmpInfo.bmiHeader.biCompression = BI_RGB;

			// Create the bitmap and select it into the compatible DC
			void* pBits;
			HBITMAP hBitmap = CreateDIBSection(hdcBmp, &bmpInfo, DIB_RGB_COLORS, &pBits, NULL, 0);

			HBITMAP hbmOld = (HBITMAP)SelectObject(hdcBmp, hBitmap);



			RECT rect = { 0, 0, atlas_w, atlas_h };
			FillRect(hdcBmp, &rect, GetStockObject(BLACK_BRUSH));
			SetBkMode(hdcBmp, TRANSPARENT);
			SelectObject(hdcBmp, GetStockObject(WHITE_BRUSH));
			COLORREF colorRef = RGB(255, 255, 255);
			SetTextColor(hdcBmp, colorRef);
			//SetTextAlign(hdcBmp, TA_CENTER | TA_TOP);
			
			for(int i = 0; i < 128; i++) {
				if(isprint(i) == 0 || isspace(i))
					continue;
				TCHAR text[1] = { i };

				RECT rect = { 0 };
				rect.left = ((i % CHARS_PER_SIDE) * char_w);
				rect.top =  ((i / CHARS_PER_SIDE) * char_h);
				rect.right  = (rect.left + char_w);
				rect.bottom = (rect.top  + char_h);
				
				RECT bounds = { 0 };
				GetTextExtentPoint32(hdcBmp, text, 1, &bounds);
				//printf("(%d, %d)-(%d, %d) [%d x %d]\n", bounds.left, bounds.top, bounds.right, bounds.bottom, bounds.right - bounds.left, bounds.bottom - bounds.top);

				int bw = abs(bounds.right - bounds.left);

				//TextOut(hdcBmp, rect.left, rect.top, text, 1);

				if(DrawTextA(hdcBmp, text, 1, &rect, DT_SINGLELINE | DT_CENTER) == 0) {
					printf("Error drawing text!");
				}
				num_chars++;
			}

			BITMAP bitmap = { 0 };
			GetObject(hBitmap, sizeof(BITMAP), &bitmap);


			pix_buffer = malloc(char_w * char_h * num_chars);
			indexes = malloc(num_chars);
			memset(offsets, 0, sizeof(offsets));
			memset(pix_buffer, 0, char_w * char_h * num_chars);
			memset(indexes, 0, num_chars);	

			UINT32* pixels = (UINT32*)bitmap.bmBits;
	
			for(int i = 0; i < 128; i++) {

				if(isprint(i) == 0 || isspace(i))
					continue;

				
				int x = (i % CHARS_PER_SIDE) * char_w;
				int y = (i / CHARS_PER_SIDE) * char_h;

				for(int py = 0; py < char_h; py++)
				for(int px = 0; px < char_w; px++)
				{
					int p = (pixels[(x + px) + (y + py) * atlas_w] & 0xFF);
					pix_buffer[(index * char_w * char_h) + (px + py * char_w)] = (p != 0);
				}

				index++;
			}




			int file_ext_starts = -1;
			int file_name_starts = 0;
			for(UINT32 i = chars - 1; i < chars; i--) {
				if(buf[i] == '.') {
					file_ext_starts = i;
				}
				if(buf[i] == '\\' || buf[i] == '/') {
					file_name_starts = i;
					break;
				}
			}
			

			int nchars_to_write = file_ext_starts - file_name_starts;
			


			//memcpy(pxf_buf, buf, nchars_to_write);
			//strcat(pxf_buf, ".pxf");
			//save_font(pxf_buf);

			//memcpy(header_buf, buf, nchars_to_write);
			//strcat(header_buf, ".h");
			//export_c_header(header_buf);
			SaveBitmapToFile("test.bmp", hBitmap);
		
			SelectObject(hdcBmp, hbmOld);
			DeleteDC(hdcBmp);
			ReleaseDC(NULL, hdcScreen);
		}

	}
#endif 
	else {
		exit(0);
	}


	index = 0;
	for(int i = 0; i < 128; i++) {
		if(isprint(i) && !isspace(i)) {
			indexes[index] = (char)i;
			offsets[i] = &pix_buffer[index * char_w * char_h];
			rev_indexes[i] = index;
			index++;
		}
	}

				
	const char* previews[] = {
		"!\"#$%&'()*+,-./0123456789",
		":;<=>?@",
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ",
		"[\\]^_`",
		"abcdefghijklmnopqrstuvwxyz"
		"{|}~"
	};

#ifdef _WIN32
	int h = GetSystemMetrics(SM_CYSCREEN);
	int w = GetSystemMetrics(SM_CXSCREEN);
#else
	int w = 1920;
	int h = 720;
#endif  
	//Calculate ideal window size based on the character size
	int scale = 20;
	int cmax = (char_w > char_h ? char_w : char_h);
	int limit = (w > h) ? h : w;
	limit = (limit * 3)/4;

	while((cmax * (scale*2)) < limit)
		scale *= 2;


	int width = char_w * scale;
	int height = char_h * scale + char_h*5;

	smol_frame_t* frame = smol_frame_create(width, height, "PixFont creator");

	int prev_char = -1;
	int cur_char = 0;
	unsigned* frame_buffer = malloc(sizeof(unsigned) * width * height);

	int pen_x;
	int pen_y;



	char title_buf[128] = { 0 };

	puts("-----------------------------------");
	puts("F5 = Save a pixel font file (.pfx)");
	puts("F6 = Open a pixel font file.");
	puts("F7 = Export a C header file. (.h)");
	puts("F8 = Export as C data blob. (const char[])");
	puts("Place pixels by pressimg LMB");
	puts("Erase pixels by pressing RMB");
	puts("Change to next character Right Arrow Key, or Mouse Wheel Up.");
	puts("Change to last character Left Arrow key, or Mouse Wheel Down.");
	puts("To offset image on X-axis press and hold shift, and press Left/Right arrow keys");
	puts("To offset image on Y-axis press and hold shift, and press Up/Down arrow keys");

	while(!smol_frame_is_closed(frame)) {

		smol_frame_update(frame);
		smol_inputs_flush();

		for(smol_frame_event_t ev; smol_frame_acquire_event(frame, &ev);) {
			smol_inputs_update(&ev);
		}

		//Ah macros, these poor man's lambdas :P
	#define CHAR_PIX_AT(px, py) offsets[indexes[cur_char]][(px) + (py)*char_w]
	#define FB_PIX_AT(px, py) frame_buffer[(px) + (py) * width]

		for(int y = 0; y < char_h*scale; y++)
		for(int x = 0; x < width; x++)
			FB_PIX_AT(x, y) = (y < ((1*char_h/3+0)*scale)) || (y > ((2*char_h/3+1)*scale)) ? 0xFFCCCCCC : 0xFFFFFFFF;


		pen_x = smol_mouse_x() / scale;
		pen_y = smol_mouse_y() / scale;

	
		cur_char += smol_mouse_move_z();
		if(smol_key_down(SMOLK_LSHIFT)) {

			if(smol_mouse_y() < char_h * scale) {
				if(smol_mouse_down(1)) {
					CHAR_PIX_AT(pen_x, pen_y) = 1;
				}

				if(smol_mouse_down(2)) {
					CHAR_PIX_AT(pen_x, pen_y) = 0;
				}
			}

			if(smol_key_hit(SMOLK_RIGHT)) {
				for(unsigned y = char_h-1; y < char_h; y--)
				for(unsigned x = char_w-1; x < char_w; x--) 
				{
					CHAR_PIX_AT(x, y) = CHAR_PIX_AT((char_w + x - 1) % char_w, y);
				}
			}
			
			if(smol_key_hit(SMOLK_LEFT)) {
				for(unsigned y = 0; y < char_h; y++)
				for(unsigned x = 0; x < char_w; x++) 
				{
					CHAR_PIX_AT(x, y) = CHAR_PIX_AT((char_w + x + 1) % char_w, y);
				}
			}

			if(smol_key_hit(SMOLK_DOWN)) {
				for(unsigned y = char_h-1; y < char_h; y--)
				for(unsigned x = char_w-1; x < char_w; x--) 
				{
					CHAR_PIX_AT(x, y) = CHAR_PIX_AT(x, (char_h + y - 1) % char_h);
				}
			}
			
			if(smol_key_hit(SMOLK_UP)) {
				for(unsigned y = 0; y < char_h; y++)
				for(unsigned x = 0; x < char_w; x++) 
				{
					CHAR_PIX_AT(x, y) = CHAR_PIX_AT(x, (char_h + y + 1) % char_h);
				}
			}

		} else {

			if(smol_mouse_y() < char_h * scale) {
				if(smol_mouse_hit(1)) {
					CHAR_PIX_AT(pen_x, pen_y) = 1;
				}

				if(smol_mouse_hit(2)) {
					CHAR_PIX_AT(pen_x, pen_y) = 0;
				}
			}

			if(smol_key_hit(SMOLK_RIGHT)) cur_char++;
			if(smol_key_hit(SMOLK_LEFT)) cur_char--;
		}

		if(smol_key_hit(SMOLK_F5)) {
			const char* exts[] = { "*.pxf" };
			char* path = tinyfd_saveFileDialog("Save a pix font", "", sizeof(exts)/sizeof(*exts), exts, NULL);
			if(path) save_font(path);
		}

		if(smol_key_hit(SMOLK_F6)) {
			const char* exts[] = { "*.pxf" };
			char* path = tinyfd_openFileDialog("Open a pix font", "", sizeof(exts)/sizeof(*exts), exts, NULL, 0);
			if(path) load_font(path);
		}

		if(smol_key_hit(SMOLK_F7)) {
			const char* exts[] = { "*.h", "*.c"};
			char* path = tinyfd_saveFileDialog("Export C array", "", sizeof(exts)/sizeof(*exts), exts, NULL);
			if(path) export_c_header(path);
		}

		if(smol_key_hit(SMOLK_F8)) {
			const char* exts[] = { "*.h", "*.c"};
			char* path = tinyfd_saveFileDialog("Export C array", "", sizeof(exts)/sizeof(*exts), exts, NULL);
			if(path) export_c_blob(path);
		}

		if(cur_char < 0) cur_char = 0;
		if(cur_char > (num_chars-1)) cur_char = (num_chars-1);

		//for(int y = 0; y < char_h; y++)
		//for(int x = 0; x < width; x++)
		//{
		//	FB_PIX_AT(x, y*scale) = 0xFF444444;
		//}

		//Draw hor grid
		for(int j = 0; j < char_h+1; j++)
		for(int i = 0; i < width; i++) {
			FB_PIX_AT(i, j*scale) = 0xFF888888;
			FB_PIX_AT(i, j*scale) = 0xFF888888;
		}

		//Draw ver grid
		for(int j = 0; j < char_h*scale; j++)
		for(int i = 0; i < char_w; i++) {
			FB_PIX_AT(i*scale, j) = 0xFF888888;
			FB_PIX_AT(i*scale, j) = 0xFF888888;
		}

		for(int y = 0; y < char_h*scale; y++) {
			int x = char_geometry[indexes[cur_char]][0]*scale;
			int w = char_geometry[indexes[cur_char]][1]*scale;
			FB_PIX_AT(x, y) = 0xFFFFFF00;
			FB_PIX_AT(x+w, y) = 0xFFFFFF00;
		
		}


		for(int y = 0; y < char_h; y++)
		for(int x = 0; x < char_w; x++)
		{
			if(CHAR_PIX_AT(x, y)) {
				for(int z = 0; z < scale; z++)
				for(int w = 0; w < scale; w++)
				{
					//frame_buffer[x*scale + y*scale*width + z+w*width] = 0xFF000000;
					FB_PIX_AT(x * scale + w, y * scale + z) = 0xFF000000;
				}
			}
		}
		#undef CHAR_PIX_AT

		//Render previews:

		#define CHAR_PIX_AT(px, py) offsets[c][(px) + (py)*char_w]
		
		char hover = -1;
		for(int i = 0; i < 5; i++) {
			int l = strlen(previews[i]);
			for(int j = 0; j < l; j++) {
				char c = previews[i][j];

				if(
					smol_mouse_x() >= j * char_w && smol_mouse_x() <= (j * char_w + char_w) &&
					smol_mouse_y() >= ((char_h * scale) + i * char_h) && smol_mouse_y() <= ((char_h * scale) + i * char_h + char_h)
				) {
					if(smol_mouse_hit(1)) {
						cur_char = rev_indexes[c];
					}
					hover = c;
				}

				for(int y = 0; y < char_h; y++) 
				for(int x = 0; x < char_w; x++) 
				{
					unsigned color = 0xFFAAAAAA;
					if(rev_indexes[c] == cur_char) color = 0xFF00AA00;
					if(rev_indexes[hover] == c) color = 0xFFFFAAAA;
					if(CHAR_PIX_AT(x, y)) color = 0xFF000000;
					FB_PIX_AT(j * char_w + x, (char_h * scale) + i * char_h + y) = color;
				}
			}
		}
		#undef CHAR_PIX_AT

		if(smol_mouse_y() < char_h*scale) {
			for(int x = 0; x < scale; x++) {
				FB_PIX_AT(pen_x*scale+x, pen_y*scale) = 0xFFFF0000;
				FB_PIX_AT(pen_x*scale+x, pen_y*scale+(scale-1)) = 0xFFFF0000;
				FB_PIX_AT(pen_x*scale, pen_y*scale+x) = 0xFFFF0000;
				FB_PIX_AT(pen_x*scale+(scale-1), pen_y*scale+x) = 0xFFFF0000;
			}
		}


		smol_frame_blit_pixels(frame, frame_buffer, width, height, 0, 0, width, height, 0, 0, width, height);

		

		if(prev_char != cur_char) {
			sprintf(title_buf, "PixFont Creator - Current Character: %c", indexes[cur_char]);
			smol_frame_set_title(frame, title_buf);
			prev_char = cur_char;
		}
	}

	if(frame_buffer) 
		free(frame_buffer);

	free(pix_buffer);
	free(indexes);

	return 0;
}


void load_font(const char* path) {

	char line[4096];

	FILE* file = fopen(path, "r");

	fscanf(file, "glyph_size: %d %d\n", &char_w, &char_h);
	fscanf(file, "num_chars: %d\n", &num_chars);

	if(pix_buffer) free(pix_buffer);
	if(indexes) free(indexes);

	indexes = malloc(num_chars);
	memset(indexes, 0, num_chars);
	memset(offsets, 0, 128);

	#define CHAR_PIX_AT(px, py) offsets[indexes[i]][(px) + (py)*char_w]

	pix_buffer = malloc(num_chars * char_w * char_h);
	memset(pix_buffer, 0, num_chars * char_w * char_h);


	for(int i = 0; i < num_chars; i++) {
		fscanf(file, "%c:\n", &indexes[i]);
		offsets[indexes[i]] = &pix_buffer[i * char_w * char_h];

		for(int y = 0; y < char_h; y++) {
			fgets(line, 4096, file);
			char* tok = line;
			for(int x = 0; x < char_w; x++)
			{
				int val = 0;
				tok = strtok(x == 0 ? tok : NULL, ", ");
				sscanf(tok, "%d", &val);
				CHAR_PIX_AT(x, y) = val;
			}
		}
	}

	fgetc(file);
	
	for(int i = 0; i < num_chars; i++) {
		char c;
		int x;
		int w;
		if(fscanf(file, "%c: %d %d\n", &c, &x, &w)) {
			char_geometry[c][0] = x;
			char_geometry[c][1] = w;
		} else {
			break;
		}
	}

	#undef CHAR_PIX_AT

	fclose(file);

}

void update_bounds() {
	#define CHAR_PIX_AT(px, py) offsets[indexes[i]][(px) + (py)*char_w]

	for(int i = 0; i < num_chars; i++) {

		int minX = 32;
		int maxX = 0;

		for(int y = 0; y < char_h; y++)
		for(int x = 0; x < char_w; x++) {
			char ch = (CHAR_PIX_AT(x, y) ? '1' : '0');
			if(ch == '1') {
				if(x < minX) minX = x;
				if(x > maxX) maxX = x;
			}

			if(maxX >= minX) {
				char_geometry[indexes[i]][0] = minX;
				char_geometry[indexes[i]][1] = (maxX - minX) + 1;
			}
		}
		
	}
	#undef CHAR_PIX_AT
}

void save_font(const char* path) {

	update_bounds();

	FILE* file = fopen(path, "w");
	fprintf(file, "glyph_size: %d %d\n", char_w, char_h);
	fprintf(file, "num_chars: %d\n", num_chars);
	#define CHAR_PIX_AT(px, py) offsets[indexes[i]][(px) + (py)*char_w]

	const char* next[] = { ", ", "\n" };



	for(int i = 0; i < num_chars; i++) {
		fprintf(file, "%c:\n", indexes[i]);
		
		int minX = 32;
		int maxX = 0;
		
		for(int y = 0; y < char_h; y++) 
		for(int x = 0; x < char_w; x++) {
			char ch = (CHAR_PIX_AT(x, y) ? '1' : '0');
			fprintf(file, "%c%s", ch, next[(x + 1) >= char_w]);
		}
		//printf("%c [p: %d w: %d]\n", indexes[i], minX, (maxX - minX));
		
	}
	#undef CHAR_PIX_AT
	fputc('\n', file);
	for(int i = 0; i < num_chars; i++) {
		fprintf(file, "%c: %d %d\n", indexes[i], char_geometry[indexes[i]][0], char_geometry[indexes[i]][1]);
	}

	fclose(file);

}

void export_c_header(const char* path) {

	char buf[2048];
	strcpy(buf, path);
	
	update_bounds();

	char* file_name = NULL;
	char* font_name = NULL;
	char* tok = strtok(buf, "\\/");
	for(;tok != NULL;) {
		file_name = tok;
		tok = strtok(NULL, "\\/");
	}

	font_name = strtok(file_name, ".");
	for(int i = 0; i < strlen(font_name); i++)
		font_name[i] = toupper(font_name[i]);

	FILE* file = fopen(path, "w");
	
	fprintf(file, "#define PXF_%s_WIDTH %d\n", font_name, char_w);
	fprintf(file, "#define PXF_%s_HEIGHT %d\n", font_name, char_h);
	fprintf(file, "\n");


	const char* next[] = { ", ", "}" };
	const char* next_char[] = {",\n", "\n"};

	fprintf(file, "static char PXF_%s_DATA[128][PXF_%s_HEIGHT][PXF_%s_WIDTH] = {\n", font_name, font_name, font_name);

	#if 0
	#define CHAR_PIX_AT(px, py) offsets[indexes[i]][(px) + (py)*char_w]
	for(int i = 0; i < num_chars; i++) {
		if(indexes[i] == '\'')
			fprintf(file, "\t[\'\\\'\'] = {\n");
		else if(indexes[i] == '\\')
			fprintf(file, "\t[\'\\\\'] = {\n");
		else 
			fprintf(file, "\t[\'%c\'] = {\n", indexes[i]);
		for(int y = 0; y < char_h; y++) {
			fprintf(file, "\t\t{");
			for(int x = 0; x < char_w; x++) {
				char ch = (CHAR_PIX_AT(x, y) ? '1' : '0');
				fprintf(file, "%c%s", ch, next[(x + 1) >= char_w]);
			}
			fprintf(file, "%s", next_char[(y+1) >= char_h]);
		}
		fprintf(file, "\t}%s", next_char[(i+1) >= num_chars]);
	}
	#else 
	#define CHAR_PIX_AT(px, py) offsets[i][(px) + (py)*char_w]
	for(int i = 0; i < 128; i++) {
		if(offsets[i]) {


			fprintf(file, "\t{/* %c */\n", i);
			for(int y = 0; y < char_h; y++) {
				fprintf(file, "\t\t{");
				for(int x = 0; x < char_w; x++) {
					char ch = (CHAR_PIX_AT(x, y) ? '1' : '0');
					fprintf(file, "%c%s", ch, next[(x + 1) >= char_w]);
					//printf("%c", ch);
				}
				fprintf(file, "%s", next_char[(y+1) >= char_h]);
				//puts("");
			}

			//printf("%c [p: %d w: %d]\n", i, minX, (maxX - minX));
			fprintf(file, "\t},\n");
		}
		else {
			fprintf(file, "\t{0} /* 0x%x */ %s", i, next_char[(i+1) >= 128]);
		}

	}
	#endif 

	fprintf(file, "};\n");

	fputc('\n', file);
	fputs("//This array contains the x-offset where character's most left pixel is and the character width.\n", file);
	fprintf(file, "static char PXF_%s_OFFSET_X_WIDTH[128][2] = {\n", font_name);
#if 0
	for(int i = 0; i < num_chars; i++) {
		if(indexes[i] == '\'')
			fprintf(file, "\t[\'\\\'\'] = {%d, %d}%s", char_geometry[indexes[i]][0], char_geometry[indexes[i]][1], next_char[(i+1) >= num_chars]);
		else if(indexes[i] == '\\')
			fprintf(file, "\t[\'\\\\\'] = {%d, %d}%s", char_geometry[indexes[i]][0], char_geometry[indexes[i]][1], next_char[(i+1) >= num_chars]);
		else 
			fprintf(file, "\t[\'%c\'] = {%d, %d}%s", indexes[i], char_geometry[indexes[i]][0], char_geometry[indexes[i]][1], next_char[(i+1) >= num_chars]);
	}
#else 
	for(int i = 0; i < 128; i++) {
		if(offsets[i]) {
			fprintf(file, "/* %c */{%d, %d},\n", i, char_geometry[i][0], char_geometry[i][1]);
		}
		else {
			fprintf(file, "\t{0} /* 0x%x */ %s", i, next_char[(i+1) >= 128]);
		}
	}
#endif 
	fprintf(file, "};\n");

	#undef CHAR_PIX_AT

	fclose(file);
}

void export_c_blob(const char* path) {

	char buf[2048];
	strcpy(buf, path);

	update_bounds();

	char* file_name = NULL;
	char* font_name = NULL;
	char* tok = strtok(buf, "\\/");
	for(; tok != NULL;) {
		file_name = tok;
		tok = strtok(NULL, "\\/");
	}

	font_name = strtok(file_name, ".");
	for(int i = 0; i < strlen(font_name); i++)
		font_name[i] = toupper(font_name[i]);

	int size = (num_chars * char_w * char_h);
	int total_size = (size / 8) + (num_chars * sizeof(char) * 2);
	char* font_buf = (char*)malloc(total_size);
	memset(font_buf, 0, total_size);
	char* geom_buf = (char*)malloc(num_chars * 2);
	memset(geom_buf, 0, num_chars * 2);
	int npixels = char_w * char_h;

	int font_idx = 0;
	int idx = 0;
	for(int i = 0; i < num_chars; i++) {

		const char* buf_ptr = &pix_buffer[i * npixels];

		for(int y = 0; y < char_h; y++)
			for(int x = 0; x < char_w; x++)
			{
				if(buf_ptr[x + y * char_w]) {
					font_buf[idx / 8] |= (0x80 >> (idx % 8));
				}
				font_idx += (idx && (idx % 8) == 0);
				idx++;
			}
		//printf("%c at index: %d saved.\n", 33+i, i*char_w*char_h);
	}


	int geom_index = 0;
	for(int i = 0; i < num_chars; i++) {
		geom_buf[geom_index++] = char_geometry[33 + i][0];
		geom_buf[geom_index++] = char_geometry[33 + i][1];
	}

	const char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


	FILE* file = fopen(path, "w");
	fprintf(file, "#define PXF_%s_WIDTH %d\n", font_name, char_w);
	fprintf(file, "#define PXF_%s_HEIGHT %d\n", font_name, char_h);
	fprintf(file, "const char PXF_%s_FONT_BLOB[] = \n\"", font_name);
	int i = 0;
	for(; i < font_idx; i += 3) {
		unsigned int b24 = (
			(unsigned char)(font_buf[i + 0]) << 16 |
			(unsigned char)(font_buf[i + 1]) << 8 |
			(unsigned char)(font_buf[i + 2]) << 0
		);

		char a = b64[(b24 >> 0x12) & 0x3FU];
		char b = b64[(b24 >> 0x0C) & 0x3FU];
		char c = b64[(b24 >> 0x06) & 0x3FU];
		char d = b64[(b24 >> 0x00) & 0x3FU];

		fprintf(file, "%c%c%c%c", a, b, c, d);
		if(i && i % (32 * 3) == 0) fprintf(file, "\"\n\"");
	}

	{
		int padding = size % 3;
		if(padding > 0) padding = 3 - padding;
		fprintf(file, "%.*s", padding + 1, "====");
	}
	fprintf(file, "\";\n");
	//
	fprintf(file, "const char* PXF_%s_GEOM_BLOB = \n\"", font_name);
	for(i = 0; i < geom_index; i += 3) {
		unsigned int b24 = (
			(unsigned char)(geom_buf[i + 0]) << 16 |
			(unsigned char)(geom_buf[i + 1]) << 8 |
			(unsigned char)(geom_buf[i + 2]) << 0
		);

		char a = b64[(b24 >> 0x12) & 0x3FU];
		char b = b64[(b24 >> 0x0C) & 0x3FU];
		char c = b64[(b24 >> 0x06) & 0x3FU];
		char d = b64[(b24 >> 0x00) & 0x3FU];

		fprintf(file, "%c%c%c%c", a, b, c, d);
		if(i && i % (32 * 3) == 0) fprintf(file, "\"\n\"");
	}
	{
		int padding = size % 3;
		if(padding > 0) padding = 3 - padding;
		fprintf(file, "%.*s", padding + 1, "====");
	}
	fprintf(file, "\";");
	/*fprintf(file, "const char PXF_%s_DATA_BLOB[] = {\n\t", font_name);
	for(int i = 0; i < size/8; i++) {
		fprintf(file,"%u,", (data_buf[i] & 0xFFU));
		if(i && (i % 32) == 0) fprintf(file, "\n\t");
	}
	fprintf(file, "\n};");*/
	fclose(file);

}


BOOL SaveBitmapToFile(const char* filename, HBITMAP hBitmap) {
	BITMAP bitmap;
	GetObject(hBitmap, sizeof(BITMAP), &bitmap);

	BITMAPFILEHEADER bmfHeader;

	BYTE byte[sizeof(BITMAPINFOHEADER) /*+ 3*sizeof(UINT32)*/];

	BITMAPINFOHEADER* bi = byte;

	bi->biSize = sizeof(BITMAPINFOHEADER);
	bi->biWidth = bitmap.bmWidth;
	bi->biHeight = -bitmap.bmHeight;
	bi->biPlanes = 1;
	bi->biBitCount = bitmap.bmBitsPixel;
	bi->biCompression = BI_RGB;
	bi->biSizeImage = 0;
	bi->biXPelsPerMeter = 0;
	bi->biYPelsPerMeter = 0;
	bi->biClrUsed = 0;
	bi->biClrImportant = 0;

	//UINT32(*rgb)[3] = (UINT32*)&byte[sizeof(BITMAPINFOHEADER)];
	//rgb[0][0] = 0xFF0000;
	//rgb[0][1] = 0x00FF00;
	//rgb[0][2] = 0x0000FF;

	DWORD dwBmpSize = ((bitmap.bmWidth * bi->biBitCount + 31) / 32) * 4 * bitmap.bmHeight;

#ifndef UNICODE
	TCHAR* filepath = filename;
#else 
	TCHAR filepath[512] = { 0 };
	MultiByteToWideChar(CP_UTF8, 0, filename, strlen(filename), filepath, 511);
#endif

	HANDLE hFile = CreateFile(filepath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	
	if (hFile == INVALID_HANDLE_VALUE) {
		return FALSE;
	}

	DWORD dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);
	bmfHeader.bfSize = dwSizeofDIB;
	bmfHeader.bfType = 0x4D42; // BM

	DWORD dwBytesWritten;
	WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
	WriteFile(hFile, (LPSTR)byte, sizeof(byte), &dwBytesWritten, NULL);
	WriteFile(hFile, (LPSTR)bitmap.bmBits, dwBmpSize, &dwBytesWritten, NULL);

	CloseHandle(hFile);

	return TRUE;
}

#include "thirdparty/tinyfiledialogs.c"