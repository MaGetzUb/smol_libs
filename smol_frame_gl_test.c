#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma GCC diagnostic ignored "-Wmissing-braces"

#define GLBIND_IMPLEMENTATION
#include "thirdparty/glbind.h"

//#define SMOL_FRAME_BACKEND_XCB
#define SMOL_FRAME_IMPLEMENTATION
#include "smol_frame.h"

#define SMOL_UTILS_IMPLEMENTATION
#include "smol_utils.h"

#include "smol_font_16x16.h"

const char vsh[] =
	"#version 330\n"
	"#line " SMOL_STRINGIFY(__LINE__) "\n"
	"\n"
	"layout(location = 0) in vec2 aPos;\n"
	"layout(location = 1) in vec3 aCol;\n"
	"\n"
	"uniform vec2 uAspect;\n"
	"uniform float uAngle;\n"
	"\n"
	"out vec3 vCol;\n"
	"\n"
	"void main() {\n"
	"	vCol = aCol;\n"
	"   mat2 rot = mat2(cos(uAngle), sin(uAngle), sin(uAngle), -cos(uAngle));\n"
	"	gl_Position = vec4((rot*aPos)*uAspect, 0.f, .5f);\n"
	"}\n"
;

const char fsh[] =
	"#version 330\n"
	"#line " SMOL_STRINGIFY(__LINE__) "\n"
	"\n"
	"layout(location = 0) out vec4 ResultColor;\n"
	"\n"
	"in vec3 vCol;\n"
	"void main() {\n"
	"	ResultColor = vec4(vCol, 1.);\n"
	"}"
;

//#define gl_assert              \
//do {                           \
//	GLenum error = glGetError(); \
//	if(error != GL_NO_ERROR) {   \
//		printf("%d\n", error);   \
//		SMOL_BREAKPOINT();       \
//	}                            \
//} while(0)

int main() {


	GLenum result = glbInit(NULL, NULL);

	smol_frame_gl_spec_t gl_spec = smol_init_gl_spec(3, 3, SMOL_TRUE, SMOL_FALSE, 8, SMOL_TRUE);

	smol_frame_config_t frame_config = {
		.width = 800,
		.height = 600,
		.title = "Smol Frame :)",
		.flags = SMOL_FRAME_CONFIG_IS_RESIZABLE | SMOL_FRAME_CONFIG_HAS_MAXIMIZE_BUTTON | SMOL_FRAME_DEFAULT_CONFIG,
		.gl_spec = &gl_spec
	};


	smol_frame_t* frame = smol_frame_create_advanced(&frame_config);


	glEnable(GL_MULTISAMPLE);
	puts(glGetString(GL_VERSION));

	int majo, mino;
	glGetIntegerv(GL_MAJOR_VERSION, &majo);
	glGetIntegerv(GL_MINOR_VERSION, &mino);
	printf("OpenGL Version: %d.%d\n", majo, mino);

	
	float vbData[][5] = {
		{ -.25f, -.25f, 1.f, 0.f, 0.f },
		{ + 0.f, +.25f, 0.f, 1.f, 0.f },
		{ +.25f, -.25f, 0.f, 0.f, 1.f },
	};
	
	puts("Creating vertex shader module...");
	GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
	{
		char log[1024] = { 0 };
		GLsizei length;

		const char* source[] = { vsh };
		int lens[] = { sizeof(vsh) };

		glShaderSource(vshader, 1, source, lens);
		glCompileShader(vshader);

		glGetShaderInfoLog(vshader, 1024, &length, log);
		if(length) printf("Vertex shader compilation info: %s\n", log);
	}
	
	puts("Creating fragment shader module...");
	GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
	{
		char log[1024] = { 0 };
		GLsizei length;

		const char* source[] = { fsh };
		int lens[] = { sizeof(fsh) };

		glShaderSource(fshader, 1, source, lens);
		glCompileShader(fshader);

		glGetShaderInfoLog(fshader, 1024, &length, log);
		if(length) printf("Fragment shader compilation info: %s\n", log);
	};

	
	puts("Creating shader program...");
	GLuint program = glCreateProgram();
	glAttachShader(program, vshader);
	glAttachShader(program, fshader);
	glLinkProgram(program);

	{
		char log[1024] = { 0 };
		GLsizei length;
		glGetProgramInfoLog(program, 1024, &length, log);
		if(length) printf("Shader program info: %s\n", log);
	}
	
	puts("Generating vbo...");
	GLuint vertexData = 0;
	glGenBuffers(1, &vertexData);
	glBindBuffer(GL_ARRAY_BUFFER, vertexData);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vbData), vbData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	puts("Generating vao...");
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);


	glBindBuffer(GL_ARRAY_BUFFER, vertexData);
	glEnableVertexArrayAttrib(vao, 0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float)*0));
	glEnableVertexArrayAttrib(vao, 1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float)*2));
	glBindVertexArray(0);
	
	
	GLuint font;
	{
		puts("Generating pixel buffer for texture...");
		GLubyte pixels[65536][4] = { 0 };
		for(int i = 0; i < 8; i++)
		for(int j = 0; j < 16; j++)
		{
			int id = j + i * 16;
			for(int y = 0; y < 16; y++) 
			for(int x = 0; x < 16; x++) {
				int idx = (j * 16 + x) + (i * 16 + y) * 256;
				pixels[idx][0] = pixels[idx][1] = pixels[idx][2] = 255;
				pixels[idx][3] = 255 * PXF_SMOL_FONT_16X16_DATA[id][y][x];
			}
		}


		puts("Creating a font texture...");
		glGenTextures(1, &font);
		glBindTexture(GL_TEXTURE_2D, font);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glViewport(0, 0, 800, 600);
	glClearColor(0.f, 0.f, 0.5f, 1.f);

	GLuint uniform_angle_location = glGetUniformLocation(program, "uAngle");
	GLuint uniform_aspect_location = glGetUniformLocation(program, "uAspect");

	puts("Mainloop begins...");

	double start_time = smol_timer();

	float aspect[] = { 1.f, 4.f / 3.f };


	while(!smol_frame_is_closed(frame)) {
		smol_frame_update(frame);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float rot = (smol_timer() - start_time)*6.283/30.;

		glUseProgram(program);
		for(smol_frame_event_t ev; smol_frame_acquire_event(frame, &ev);) {
			if(ev.type == SMOL_FRAME_EVENT_RESIZE) {
				glViewport(0, 0, ev.size.width, ev.size.height);
				aspect[0] = 1.f;
				aspect[1] = (float)ev.size.width / ev.size.height;
			}
		}

		
		glUniform1fv(uniform_angle_location, 1, &rot);
		glUniform2fv(uniform_aspect_location, 1, &aspect);
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);
		glUseProgram(0);

		smol_frame_gl_swap_buffers(frame);
	}

	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vertexData);
	glDeleteProgram(program);

	smol_frame_destroy(frame);

	return 0;
}

