#ifndef SMOL_TEXTRENDERER_H
#define SMOL_TEXTRENDERER_H

#ifdef __EMSCRIPTEN__
#define GL_GLEXT_PROTOTYPES
#include <GL/glext.h>
#include <GLES2/gl2.h>
#include <GLES3/gl3.h>
#include <GLES3/gl2ext.h>
#include <GLES3/gl32.h>
#else 
#ifdef SMOL_GL_DEFENITION_HEADER
#include SMOL_GL_DEFENITION_HEADER
#endif 
#endif 

#ifndef smol_offset_of
#define smol_offset_of(Type, Field) ((void*)&(((Type*)0)->Field))
#endif 

typedef struct _smol_font_hor_geom_t smol_font_hor_geom_t;
typedef struct _smol_font_t smol_font_t;
typedef struct _smol_text_renderer_t smol_text_renderer_t;

smol_font_t smol_font_create(const char* pixels, int char_w, int char_h, smol_font_hor_geom_t* horizontal_geometry);
smol_font_t smol_font_load_pxf(const char* file_path);

smol_text_renderer_t smol_text_renderer_create(const smol_font_t* font, int max_characters);
void smol_text_renderer_set_texture_uniform_slot(smol_text_renderer_t* tr, GLuint uniform_location, GLuint texture_slot);
void smol_text_renderer_set_font(smol_text_renderer_t* tr, const smol_font_t* font);
void smol_text_renderer_begin(smol_text_renderer_t* tr);
void smol_text_renderer_end(smol_text_renderer_t* tr);
void smol_text_renderer_add_string(smol_text_renderer_t* tr, smol_v2_t pos, float scale, GLuint color, const char* fmt, ...);
void smol_text_renderer_add_char(smol_text_renderer_t* tr, smol_v2_t pos, char chr, float scale, GLuint color);
void smol_text_renderer_draw(smol_text_renderer_t* tr);

#ifdef SMOL_TEXT_RENDERER_IMPLEMENTATION

typedef struct _smol_font_hor_geom_t { 
	char offset, size; 
} smol_font_hor_geom_t;

typedef struct _smol_font_t {
	GLuint texture_id;
	int atlas_w;
	int atlas_h;
	int char_w;
	int char_h;
	smol_font_hor_geom_t* sizes;
} smol_font_t;

typedef struct _smol_char_vertex_t {
	smol_v2_t position;
	smol_v2_t texcoord;
	GLuint color;
} smol_char_vertex_t;

typedef struct _smol_text_renderer_t {
	GLuint vertex_buffer;
	GLuint index_buffer;
	GLuint vertex_attribs;
	smol_char_vertex_t* vertices;
	GLuint vertex_count;
	GLuint index_count;
	GLuint texture_slot;
	GLuint texture_uniform;
	GLenum index_type;
	const smol_font_t* font;
} smol_text_renderer_t;


smol_font_t smol_font_create(const char* pixels, int char_w, int char_h, smol_font_hor_geom_t* horizontal_geometry) {

	GLuint* memory = (GLuint*)malloc(4*1024*1024);
	smol_font_t font = { 0 };
	font.char_w = char_w;
	font.char_h = char_h;
	//16 characters per row and column
	font.atlas_w = 16 * char_w;
	font.atlas_h = 16 * char_h;
	font.sizes = horizontal_geometry;

	//We'll allocate bit more memory than we need
	if(memory) {
		memset(memory, 0, 4 * 1024 * 1024);
		for(int ty = 0; ty < 8; ty++)
		for(int tx = 0; tx < 16; tx++)
		{
			int chr = tx + ty * 16;
			for(int y = 0; y < char_w; y++) 
			for(int x = 0; x < char_h; x++) {
				int idx = 
					(ty * char_w * font.atlas_w) +
					(tx * char_w) + 
					(x + y * font.atlas_w)
				;
				memory[idx] = 0xFFFFFF | ((pixels[chr*font.char_w*font.char_h + x + y*font.char_h] > 0) * 0xFF000000);
			}
		}

		glGenTextures(1, &font.texture_id);
		glBindTexture(GL_TEXTURE_2D, font.texture_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, font.atlas_w, font.atlas_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, memory);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);
		free(memory);
	}
	
	return font;

}
#ifndef __EMSCRIPTEN__
#define INDICES_PER_QUAD 5
#else 
#define INDICES_PER_QUAD 6
#endif 
#define INDICES_PER_65K (65536 / INDICES_PER_QUAD)

smol_text_renderer_t smol_text_renderer_create(const smol_font_t* font, int max_characters) {

	smol_text_renderer_t text_renderer = { 0 };

	text_renderer.font = font;

	int index_type_size = 4;
	text_renderer.index_type = GL_UNSIGNED_INT;


	if(max_characters <= INDICES_PER_65K) {
		text_renderer.index_type = GL_UNSIGNED_SHORT;
		index_type_size = 2;
	}

	GLuint num_vbo_bytes = max_characters * 4 * sizeof(smol_char_vertex_t);
	GLuint num_ibo_bytes = max_characters * 5 * index_type_size;

#ifdef __EMSCRIPTEN__
	text_renderer.vertices = malloc(num_vbo_bytes);
	memset(text_renderer.vertices, 0, num_vbo_bytes);
#endif 
	glGenBuffers(1, &text_renderer.vertex_buffer);
	glGenBuffers(1, &text_renderer.index_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, text_renderer.vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, num_vbo_bytes, text_renderer.vertices, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, text_renderer.index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_ibo_bytes, NULL, GL_STATIC_DRAW);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	
#ifndef __EMSCRIPTEN__
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, text_renderer.index_buffer);
	void* indices = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
#else
	void* indices = malloc(num_ibo_bytes);
#endif 

	if(indices == 0) {
		const char* s = "";
		switch(glGetError()) {
			case GL_INVALID_ENUM: s = "GL_INVALID_ENUM"; break;
			case GL_INVALID_VALUE: s = "GL_INVALID_VALUE"; break;
			case GL_INVALID_OPERATION: s = "GL_INVALID_OPERATION"; break;
			case GL_INVALID_FRAMEBUFFER_OPERATION: s = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
			case GL_OUT_OF_MEMORY: s = "GL_OUT_OF_MEMORY"; break;
		}

		printf("error: %s\n", s);
	} else {
	#ifndef __EMSCRIPTEN__
		if(index_type_size < 4) {
			GLushort* shorts = (GLushort*)indices;
			for(GLuint i = 0; i < max_characters; i++) {
				shorts[i * 5 + 0] = (i * 4 + 0);
				shorts[i * 5 + 1] = (i * 4 + 1);
				shorts[i * 5 + 2] = (i * 4 + 2);
				shorts[i * 5 + 3] = (i * 4 + 3);
				shorts[i * 5 + 4] = 0xFFFF;
			}
		} else {
			GLuint* ints = (GLuint*)indices;
			for(GLuint i = 0; i < max_characters; i++) {
				ints[i * 5 + 0] = (i * 4 + 0);
				ints[i * 5 + 1] = (i * 4 + 1);
				ints[i * 5 + 2] = (i * 4 + 2);
				ints[i * 5 + 3] = (i * 4 + 3);
				ints[i * 5 + 4] = 0xFFFFFFFF;
			}
		}
	#else
		GLushort* shorts = (GLushort*)indices;
		if(index_type_size < 4) {
			for(GLuint i = 0; i < max_characters; i++) {
				shorts[i * 6 + 0] = (i * 4 + 0);
				shorts[i * 6 + 1] = (i * 4 + 1);
				shorts[i * 6 + 2] = (i * 4 + 2);
				shorts[i * 6 + 3] = (i * 4 + 1);
				shorts[i * 6 + 4] = (i * 4 + 3);
				shorts[i * 6 + 5] = (i * 4 + 2);
			}
		} else {
			GLuint* ints = (GLuint*)indices;
			for(GLuint i = 0; i < max_characters; i++) {
				ints[i * 6 + 0] = (i * 4 + 0);
				ints[i * 6 + 1] = (i * 4 + 1);
				ints[i * 6 + 2] = (i * 4 + 2);
				ints[i * 6 + 3] = (i * 4 + 1);
				ints[i * 6 + 4] = (i * 4 + 3);
				ints[i * 6 + 5] = (i * 4 + 2);
			}
		}
	#endif 

#ifndef __EMSCRIPTEN__
		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
#else 
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, text_renderer.index_buffer);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, num_ibo_bytes, indices);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#endif 

	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &text_renderer.vertex_attribs);

	glBindVertexArray(text_renderer.vertex_attribs);
	glBindBuffer(GL_ARRAY_BUFFER, text_renderer.vertex_buffer);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(smol_char_vertex_t), smol_offset_of(smol_char_vertex_t, position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(smol_char_vertex_t), smol_offset_of(smol_char_vertex_t, texcoord));
		
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(smol_char_vertex_t), smol_offset_of(smol_char_vertex_t, color));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, text_renderer.index_buffer);

	glBindVertexArray(0);

	return text_renderer;
}

void smol_text_renderer_set_texture_uniform_slot(smol_text_renderer_t* tr, GLuint uniform_location, GLuint texture_slot) {
	tr->texture_uniform = uniform_location;
	tr->texture_slot = texture_slot;
}

void smol_text_renderer_set_font(smol_text_renderer_t* tr, const smol_font_t* font) {
	if(tr->vertices) {
		smol_text_renderer_end(tr);
		smol_text_renderer_draw(tr);
		tr->font = font;
		smol_text_renderer_begin(tr);
	} else {
		tr->font = font;
	}	
}

void smol_text_renderer_begin(smol_text_renderer_t* tr) {
	tr->vertex_count = 0;
	tr->index_count = 0;
#ifndef __EMSCRIPTEN__
	glBindBuffer(GL_ARRAY_BUFFER, tr->vertex_buffer);
	tr->vertices = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
#endif 
}

void smol_text_renderer_end(smol_text_renderer_t* tr) {
#ifdef __EMSCRIPTEN__
	glBindBuffer(GL_ARRAY_BUFFER, tr->vertex_buffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, tr->vertex_count * sizeof(smol_char_vertex_t), tr->vertices);
#else 
	glUnmapBuffer(GL_ARRAY_BUFFER);
#endif 
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	tr->vertices = NULL;
}

void smol_text_renderer_add_char(smol_text_renderer_t* tr, smol_v2_t pos, char chr, float scale, GLuint color) {

	smol_font_t* font = tr->font;

	float hor_offset = 0;
	float width = font->char_w;
	float height = font->char_h;
	
	if(font->sizes) {
		width = font->sizes[chr].size;
		hor_offset = font->sizes[chr].offset;
	}


	int x = ((chr & 0x0F) * font->char_w);
	int y = (chr >> 4) * font->char_h;

	float inv_atlas_w = 1.f / (float)font->atlas_w;
	float inv_atlas_h = 1.f / (float)font->atlas_h;

	smol_v2_t uv0 = smol_v2(
		((float)x + hor_offset) * inv_atlas_w, 
		(float)y * inv_atlas_h
	);

	smol_v2_t uv1 = smol_v2(
		((float)(x + hor_offset + (width+1.f))) * inv_atlas_w, 
		(float)(y + (height-1)) * inv_atlas_h
	);
	
	hor_offset *= scale;
	width *= scale;
	height *= scale;
	width += scale;


	tr->vertices[tr->vertex_count].position = smol_v2_add(pos, smol_v2(hor_offset, 0.f));
	tr->vertices[tr->vertex_count].texcoord = uv0;
	tr->vertices[tr->vertex_count].color = color;
	tr->vertex_count++;

	
	tr->vertices[tr->vertex_count].position = smol_v2_add(pos, smol_v2(hor_offset, height));
	tr->vertices[tr->vertex_count].texcoord = smol_v2(uv0.x, uv1.y);
	tr->vertices[tr->vertex_count].color = color;
	tr->vertex_count++;

	
	tr->vertices[tr->vertex_count].position = smol_v2_add(pos, smol_v2(hor_offset + width, 0));
	tr->vertices[tr->vertex_count].texcoord = smol_v2(uv1.x, uv0.y);
	tr->vertices[tr->vertex_count].color = color;
	tr->vertex_count++;

	
	tr->vertices[tr->vertex_count].position = smol_v2_add(pos, smol_v2(hor_offset + width, height));
	tr->vertices[tr->vertex_count].texcoord = uv1;
	tr->vertices[tr->vertex_count].color = color;
	tr->vertex_count++;

#ifdef __EMSCRIPTEN__
	tr->index_count += 6;
#else 
	tr->index_count += 5;
#endif 
}


void smol_text_renderer_add_string(smol_text_renderer_t* tr, smol_v2_t pos, float scale, GLuint color, const char* fmt, ...) {

	char buffer[2048];
	smol_font_t* font = tr->font;
	va_list args;
	va_start(args, fmt);
	int n_chars = vsnprintf(buffer, 2048, fmt, args);
	va_end(args);

	smol_v2_t cpos = pos;
	float space = font->char_w;

	if(font->sizes) {

		space = font->sizes['_'].size;

		for(int i = 0; i < n_chars; i++) {
			float offset = font->sizes[buffer[i]].offset;
			float size = font->sizes[buffer[i]].size;

			switch(buffer[i]) {
				case ' ':
				case '\t':
					cpos.x += space * scale * (buffer[i]=='\t' ? 4 : 1);
				break;
				case '\n':
					cpos.x = pos.x;
					cpos.y += scale * (float)font->char_h;
				break;
				default:
					smol_text_renderer_add_char(tr, cpos, buffer[i], scale, color);
					cpos.x += scale * offset;
					cpos.x += scale * size;
				break;
			}

		}
	} else {
		for(int i = 0; i < n_chars; i++) {

			switch(buffer[i]) {
				case ' ':
				case '\t':
					cpos.x += scale * (float)font->char_w*(buffer[i]=='\t'? 4 : 1);
				break;
				case '\n':
					cpos.x = pos.x;
					cpos.y += scale * (float)font->char_h;
				break;
				default:
					smol_text_renderer_add_char(tr, cpos, buffer[i], scale, color);
					cpos.x += scale * (float)font->char_w;
				break;
			}
		}

	}

}

void smol_text_renderer_draw(smol_text_renderer_t* tr) {

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);

	glActiveTexture(GL_TEXTURE0 + tr->texture_slot);
	glBindTexture(GL_TEXTURE_2D, tr->font->texture_id);
	glUniform1i(tr->texture_uniform, tr->texture_slot);

#ifndef __EMSCRIPTEN__
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(tr->index_type == GL_UNSIGNED_SHORT ? 0xFFFF : 0xFFFFFFFF);
	glBindVertexArray(tr->vertex_attribs);
	glDrawElements(GL_TRIANGLE_STRIP, tr->index_count, tr->index_type, NULL);
	glDisable(GL_PRIMITIVE_RESTART);
#else 
	glBindVertexArray(tr->vertex_attribs);
	glDrawElements(GL_TRIANGLES, tr->index_count, tr->index_type, NULL);
#endif 
	glBindVertexArray(0);
	glDisable(GL_BLEND);
}

smol_font_t smol_font_load_pxf(const char* file_path) {

	char line[4096];
	smol_font_t font = { 0 };
	FILE* file = fopen(file_path, "r");
	int char_w;
	int char_h;
	int has_sizes = 0;
	
	unsigned int num_chars;

	if(!file)
		return font;

	fscanf(file, "glyph_size: %d %d\n", &char_w, &char_h);
	fscanf(file, "num_chars: %d\n", &num_chars);

	
	char* pix_buffer = malloc(256 * char_w * char_h);
	char* offsets[128] = {0};
	char* indexes = malloc(num_chars);

	smol_font_hor_geom_t* sizes = malloc(sizeof(smol_font_hor_geom_t) * 256);
	

	memset(indexes, 0, num_chars);
	memset(offsets, 0, 128);

	#define CHAR_PIX_AT(px, py) offsets[indexes[i]][(px) + (py)*char_w]


	memset(pix_buffer, 0, num_chars * char_w * char_h);


	for(int i = 0; i < num_chars; i++) {
		fscanf(file, "%c:\n", &indexes[i]);
		offsets[indexes[i]] = &pix_buffer[i * font.char_w * font.char_h];

		for(int y = 0; y < font.char_h; y++) {
			fgets(line, 4096, file);
			char* tok = line;
			for(int x = 0; x < font.char_w; x++)
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
			sizes[c].offset = x;
			sizes[c].size = w;
			has_sizes = 1;
		} else {
			break;
		}
	}

	fclose(file);
	
	font = smol_font_create(pix_buffer, char_w, char_h, has_sizes ? sizes : NULL);

	if(pix_buffer) free(pix_buffer);
	if(sizes) free(sizes);

	return font;
	
}

#endif 

#endif 