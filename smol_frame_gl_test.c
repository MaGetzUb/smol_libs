#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SMOL_FRAME_IMPLEMENTATION
#include "smol_frame.h"

#define GLBIND_IMPLEMTATION
#include "thirdparty/glbind.h"

const char vsh[] =
	"#version 330\n"
	"#line 12\n"
	"\n"
	"layout(location = 0) in vec2 aPos;\n"
	"layout(location = 1) in vec3 aCol;\n"
	"\n"
	"out vec3 vCol;\n"
	"\n"
	"void main() {\n"
	"	vCol = aCol;\n"
	"	gl_Position = vec4(aPos, 0.f, .5f);\n"
	"}\n"
;

const char fsh[] =
	"#version 330\n"
	"#line 30\n"
	"\n"
	"layout(location = 0) out vec4 ResultColor;\n"
	"\n"
	"in vec3 vCol;\n"
	"void main() {\n"
	"	ResultColor = vec4(vCol, 1.);\n"
	"}"
;

//#define gl_assert					\
//do {								\
//	GLenum error = glGetError();	\
//	if(error != GL_NO_ERROR) {		\
//		printf("%d\n", error);		\
//		SMOL_BREAKPOINT();			\
//	}								\
//} while(0)

int main() {


	smol_frame_gl_spec_t gl_spec = {
		.major_version = 3,
		.minor_version = 3,
		.is_backward_compatible = 0,
		.is_forward_compatible = 0,
		.color_bits = 24,
		.alpha_bits = 8,
		.depth_bits = 24,
		.stencil_bits = 8,
		.is_debug = 1,
		.has_multi_sampling = 1,
		.num_multi_samples = 4
	};

	smol_frame_config_t frame_config = {
		.width = 800,
		.height = 600,
		.title = "Smol Frame :)",
		.flags = SMOL_FRAME_DEFAULT_CONFIG,
		.gl_spec = &gl_spec
	};


	GLenum result = glbInit(NULL, NULL);
	smol_frame_t* frame = smol_frame_create_advanced(&frame_config);
	


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


	GLuint program = glCreateProgram();
	glAttachShader(program, vshader);
	glAttachShader(program, fshader);
	glLinkProgram(program);

	{
		char log[1024] = { 0 };
		GLsizei length;
		glGetProgramInfoLog(program, 1024, &length, log);
		printf("Shader program info: %s\n", log);
	}

	GLuint vertexData = 0;
	glGenBuffers(1, &vertexData);
	glBindBuffer(GL_ARRAY_BUFFER, vertexData);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vbData), vbData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);


	glBindBuffer(GL_ARRAY_BUFFER, vertexData);
	glEnableVertexArrayAttrib(vao, 0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float)*0));
	glEnableVertexArrayAttrib(vao, 1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float)*2));
	glBindVertexArray(0);


	glClearColor(0.f, 0.f, 0.5f, 1.f);

	while(!smol_frame_is_closed(frame)) {
		smol_frame_update(frame);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(program);
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);
		glUseProgram(0);

		smol_frame_gl_swap_buffers(frame);
	}

	return 0;
}