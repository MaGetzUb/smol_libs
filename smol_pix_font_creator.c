#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>

#define SMOL_FRAME_IMPLEMENTATION
#include "smol_frame.h"

#define SMOL_INPUT_IMPLEMENTATION
#include "smol_input.h"

#include "thirdparty/tinyfiledialogs.h"


char* pix_buffer = NULL;
char* offsets[128] = {0};
char* indexes = NULL;
int num_chars = 0;
int char_w, char_h;

void load_font(const char* path);
void save_font(const char* path);
void export_c_header(const char* path);

int main() {

	int stage = 0;
	int index = 0;

	puts("Type 1 (enter) to create a new font.");
	puts("Type 2 (enter) to load an existing font.");
	printf("How would you like to proceed: ");
	scanf("%d", &stage);

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

		const char* exts[] = { ".pxf" };
		char* path = tinyfd_openFileDialog("Open a pix font", "", 1, exts, exts[0], 0);
		load_font(path);

	}
	else {
		exit(0);
	}



	for(int i = 0; i < 128; i++) {
		if(isprint(i) && !isspace(i)) {
			indexes[index] = (char)i;
			offsets[i] = &pix_buffer[index*char_w*char_h];
			index++;
			printf("%c", i);
		}
	}

	//Calculate ideal window size based on the character size
	int scale = 20;
	int cmax = (char_w > char_h ? char_w : char_h);
	while(cmax * scale < 640)
		scale *= 2;

	int width = char_w * scale;
	int height = char_h * scale;

	smol_frame_t* frame = smol_frame_create(width, height, "PixFont creator");

	int cur_char = 0;
	unsigned* frame_buffer = malloc(sizeof(unsigned) * width * height);

	char title_buf[128] = { 0 };

	while(!smol_frame_is_closed(frame)) {

		smol_frame_update(frame);
		smol_inputs_flush();

		for(smol_frame_event_t ev; smol_frame_acquire_event(frame, &ev);) {
			smol_inputs_update(&ev);
		}

		//Ah macros, these poor man's lambdas :P
	#define CHAR_PIX_AT(px, py) offsets[indexes[cur_char]][(px) + (py)*char_w]
	#define FB_PIX_AT(px, py) frame_buffer[(px) + (py) * width]

		for(int y = 0; y < height; y++)
		for(int x = 0; x < width; x++)
			FB_PIX_AT(x, y) = ((y / scale) < char_h/3) || ((y/scale) > ((char_h/3)*2+1)) ? 0xFFCCCCCC : 0xFFFFFFFF;


		if(smol_mouse_hit(1)) {
			int x = smol_mouse_x() / scale;
			int y = smol_mouse_y() / scale;
			CHAR_PIX_AT(x, y) = 1;
		}

		if(smol_mouse_hit(2)) {
			int x = smol_mouse_x() / scale;
			int y = smol_mouse_y() / scale;
			CHAR_PIX_AT(x, y) = 0;
		}

		cur_char += smol_mouse_move_z();
		if(smol_key_down(SMOLK_LSHIFT)) {

			if(smol_key_hit(SMOLK_RIGHT)) {
				for(unsigned y = char_h-1; y < char_h; y--)
				for(unsigned x = char_h-1; x < char_w; x--) 
				{
					CHAR_PIX_AT(x, y) = CHAR_PIX_AT((char_w + (x - 1)) % char_w, y);
				}
			}
			
			if(smol_key_hit(SMOLK_LEFT)) {
				for(unsigned y = 0; y < char_h; y++)
				for(unsigned x = 0; x < char_w; x++) 
				{
					CHAR_PIX_AT(x, y) = CHAR_PIX_AT((char_w + (x + 1)) % char_w, y);
				}
			}

		} else {
			if(smol_key_hit(SMOLK_RIGHT)) cur_char++;
			if(smol_key_hit(SMOLK_LEFT)) cur_char--;
		}

		if(smol_key_hit(SMOLK_F5)) {
			const char* exts[] = { "*.pxf" };
			char* path = tinyfd_saveFileDialog("Save a pix font", "", sizeof(exts)/sizeof(*exts), exts, NULL, 0);
			save_font(path);
		}

		if(smol_key_hit(SMOLK_F6)) {
			const char* exts[] = { "*.pxf" };
			char* path = tinyfd_openFileDialog("Open a pix font", "", sizeof(exts)/sizeof(*exts), exts, NULL, 0);
			load_font(path);
		}

		if(smol_key_hit(SMOLK_F7)) {
			const char* exts[] = { "*.h", "*.c"};
			char* path = tinyfd_saveFileDialog("Export C array", "", sizeof(exts)/sizeof(*exts), exts, NULL, 0);
			export_c_header(path);
		}

		if(cur_char < 0) cur_char = 0;
		if(cur_char > (num_chars-1)) cur_char = (num_chars-1);

		//for(int y = 0; y < char_h; y++)
		//for(int x = 0; x < width; x++)
		//{
		//	FB_PIX_AT(x, y*scale) = 0xFF444444;
		//}

		for(int i = 0; i < width; i++) {
			FB_PIX_AT(i, (1*char_h/3+0)*scale) = 0xFF888888;
			FB_PIX_AT(i, (2*char_h/3+1)*scale) = 0xFF888888;
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


		smol_frame_blit_pixels(frame, frame_buffer, width, height, 0, 0, width, height, 0, 0, width, height);


		sprintf(title_buf, "PixFont Creator - Current Character: %c", indexes[cur_char]);

		smol_frame_set_title(frame, title_buf);

		#undef CHAR_PIX_AT
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

	#undef CHAR_PIX_AT

	fclose(file);

}

void save_font(const char* path) {

	FILE* file = fopen(path, "w");
	fprintf(file, "glyph_size: %d %d\n", char_w, char_h);
	fprintf(file, "num_chars: %d\n", num_chars);
	#define CHAR_PIX_AT(px, py) offsets[indexes[i]][(px) + (py)*char_w]

	const char* next[] = { ", ", "\n" };

	for(int i = 0; i < num_chars; i++) {
		fprintf(file, "%c:\n", indexes[i]);
		for(int y = 0; y < char_h; y++) 
		for(int x = 0; x < char_w; x++) {
			char ch = (CHAR_PIX_AT(x, y) ? '1' : '0');
			fprintf(file, "%c%s", ch, next[(x + 1) >= char_w]);
		}
		
	}
	#undef CHAR_PIX_AT

	fclose(file);

}

void export_c_header(const char* path) {

	char buf[2048];
	strcpy(buf, path);
	
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

	#define CHAR_PIX_AT(px, py) offsets[indexes[i]][(px) + (py)*char_w]

	const char* next[] = { ", ", "}" };
	const char* next_char[] = {",\n", "\n"};

	fprintf(file, "static char PXF_%s_DATA[128][PXF_%s_HEIGHT][PXF_%s_WIDTH] = {\n", font_name, font_name, font_name);

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

	fprintf(file, "};\n");
	#undef CHAR_PIX_AT

	fclose(file);

}