#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#ifndef __EMSCRIPTEN__
#define SMOL_GL_DEFENITION_HEADER "thirdparty/glbind.h"
#ifndef _MSC_VER
#define GLBIND_IMPLEMENTATION
#endif 
#include SMOL_GL_DEFENITION_HEADER
#else 
#include <emscripten.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GLES2/gl2.h>
#include <GLES3/gl3.h>
#include <GLES3/gl2ext.h>
#include <GLES3/gl32.h>
#endif 

#define SMOL_FRAME_IMPLEMENTATION
#include "smol_frame.h"

#define SMOL_MATH_IMPLEMENTATION
#include "smol_math.h"

#define SMOL_UTILS_IMPLEMENTATION
#include "smol_utils.h"

#define SMOL_INPUT_IMPLEMENTATION
#include "smol_input.h"

#define SMOL_TEXT_RENDERER_IMPLEMENTATION
#include "smol_text_renderer.h"

#include "smol_font_16x16.h"
#include "smol_font_7x8.h"

#define CASE_STRING( case_enum ) case case_enum: return #case_enum

#define smol_offset_of(Type, Field) ((void*)&(((Type*)0)->Field))
#define smol_array_size(array) (sizeof(array)/sizeof(array[0]))

#define RGBA(r, g, b, a) ( \
	(r << 0x00) | \
	(g << 0x08) | \
	(b << 0x10) | \
	(a << 0x18)	  \
)

typedef struct {
	smol_v3_t pos;
	smol_v4_t col;
} pos_col_t;

typedef struct {
	unsigned int a, b, c;
} tri_idx_t;

typedef union {
	struct {
		int x;
		int y;
		int z;
	};
	int loc[3];
	int plane;
} snek_cell_t;

typedef struct {
	smol_v4_t pos_scale;
	smol_v4_t color;
} cube_inst_t;



typedef struct _smol_frame_counter_t {
	int counter;
	int fps;
	double spf;
	double delta_accum;
} smol_frame_counter_t;

typedef struct {
	char prev_face;
	char scalar;
} dir_scalar_t;

typedef struct {
	char hor_neighbors[2];
	char ver_neighbors[2];
	char vec_idx[2];
	char positive_per_axis_dir[2];
	dir_scalar_t adj_access_axis[6];
} face_data_t;

typedef struct {
	float split_percentage;
	char is_vertical;
	union {
		char is_bottom;
		char is_right;
	};
} vp_spec_t;

typedef struct {
	int left;
	int top;
	int right;
	int bottom;
	vp_spec_t spec;
} viewport_t;

typedef struct {
	int left;
	int top;
	int right;
	int bottom;
	int num_viewports;
	viewport_t* viewports;
} view_partition_t;

typedef union {
	struct { int x, y, z; };
	int c[3];
} iv3;

//TODO: AI(?), Networking, Main menu, Sounds

typedef struct {
	snek_cell_t* cells;
	int len;
	iv3 heading;
	iv3 direction;
	smol_v4_t color;
	char plane;
	float move_step;
	float next_step;

	char player_type; 
	
	union {
		struct {
			int key_up;
			int key_down;
			int key_left;
			int key_right;
		};
	};

} snek_t;

#define SNAKE_PLAYER_PHYSICAL 0
#define SNAKE_PLAYER_ONLINE 1
#define SNAKE_PLAYER_BOT 2

void update_view_partition(view_partition_t* partition, int left, int top, int right, int bottom) {

	partition->left = left;
	partition->top = top;
	partition->right = right;
	partition->bottom = bottom;

	for(int i = 0; i < partition->num_viewports; i++) {
		vp_spec_t spec = partition->viewports[i].spec;
		if(spec.is_vertical) {
			if(spec.is_bottom) {
				partition->viewports[i].top = bottom - (float)(bottom - top) * spec.split_percentage;
				partition->viewports[i].bottom = bottom;
				bottom = partition->viewports[i].top;
			} else {
				partition->viewports[i].top = top;
				partition->viewports[i].bottom = top + (float)(bottom - top) * spec.split_percentage;
				top = partition->viewports[i].bottom;
			}
			partition->viewports[i].left = left;
			partition->viewports[i].right = right;
		}
		else {
			if(spec.is_right) {
				partition->viewports[i].left = right - (float)(right - left) * spec.split_percentage;
				partition->viewports[i].right = right;
				right = partition->viewports[i].left;
			} else {
				partition->viewports[i].left = left;
				partition->viewports[i].right = left + (float)(right - left) * spec.split_percentage;
				left = partition->viewports[i].right;
			}
			partition->viewports[i].top = top;
			partition->viewports[i].bottom = bottom;
		}
	}

}

view_partition_t create_viewport_partition(int left, int top, int right, int bottom, vp_spec_t* specs, int num_specs) {

	view_partition_t partition = { 0 };
	partition.num_viewports = num_specs;
	partition.viewports = malloc(sizeof(viewport_t) * num_specs);
	for(int i = 0; i < num_specs; i++) {
		partition.viewports[i].spec = specs[i];
	}

	update_view_partition(&partition, left, top, right, bottom);

	return partition;

}

#define VIEWPORT_BIT 2
#define SCISSOR_BIT 1

void activate_viewport(view_partition_t* partition, int index, int flags) {

	SMOL_ASSERT(index < partition->num_viewports);

	viewport_t viewport = partition->viewports[index];

	int x = viewport.left;
	int y = partition->bottom - (viewport.top + (viewport.bottom - viewport.top));
	int w = viewport.right - viewport.left;
	int h = viewport.bottom - viewport.top;

	if(flags & VIEWPORT_BIT) 
		glViewport(x, y, w, h);

	if(flags & SCISSOR_BIT) {
		glEnable(GL_SCISSOR_TEST);
		glScissor(x, y, w, h);
	}
}

void get_viewport_origin(view_partition_t* partition, int index, int* x, int* y) {

	SMOL_ASSERT(index < partition->num_viewports);
	*x = partition->viewports[index].left;
	*y = partition->viewports[index].top;

}

void get_viewport_size(view_partition_t* partition, int index, int* w, int* h) {

	SMOL_ASSERT(index < partition->num_viewports);
	*w = partition->viewports[index].right - partition->viewports[index].left;
	*h = partition->viewports[index].bottom - partition->viewports[index].top;

}

void get_viewport_position_size(view_partition_t* partition, int index, int* x, int* y, int* w, int* h) {

	SMOL_ASSERT(index < partition->num_viewports);

	*x = partition->viewports[index].left;
	*y = partition->viewports[index].top;
	*w = partition->viewports[index].right - partition->viewports[index].left;
	*h = partition->viewports[index].bottom - partition->viewports[index].top;

}

void get_viewport_aspect_ratio(view_partition_t* partition, int index, float* aspect_ratio) {

	SMOL_ASSERT(index < partition->num_viewports);
	int w, h;
	get_viewport_size(partition, index, &w, &h);
	*aspect_ratio = (float)w / (float)h;

}


int smol_frame_counter_fps(smol_frame_counter_t* ctr);

void smol_frame_counter_update(smol_frame_counter_t* ctr, double dt);

#define MAX_NUM_CUBES 10000
#define ARENA_SIDE 17

#define ARENA_PLANE_LEFT 0
#define ARENA_PLANE_BOTTOM 1
#define ARENA_PLANE_FRONT 2

#define ARENA_PLANE_RIGHT 3
#define ARENA_PLANE_TOP 4
#define ARENA_PLANE_BACK 5

#define INVALID_AXIS -1
#define X_AXIS 0
#define Y_AXIS 1
#define Z_AXIS 2

GLuint create_and_compile_shader(const char* code, GLenum shader_type, const char* file_name);
GLuint create_and_compile_shader_from_file(const char* file_path, GLenum shader_type);
GLuint create_shader_program(GLuint* shaders, GLuint count);



const char* debug_message_severity_str(GLenum severity) {
	switch(severity) {
		CASE_STRING(GL_DEBUG_SEVERITY_LOW_ARB);
		CASE_STRING(GL_DEBUG_SEVERITY_MEDIUM_ARB);
		CASE_STRING(GL_DEBUG_SEVERITY_HIGH_ARB);
	}
	return "Not part of gl debug severity.";
}

void APIENTRY debug_message_callback(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam) {

	puts(debug_message_severity_str(severity));
	puts(message);
	SMOL_BREAKPOINT();

}

#define SPEED_FORMULA(n) (0.3 - fmin(pow((snake[n].len - 3) / 100., 1.25), 1.0)*0.275)

const char* plane_names[] = {
	"Top",
	"Right",
	"Back",
	"Bottom",
	"Left",
	"Front"
};


int main(const int argc, const char* argv[]) {
		
	smol_randomize(1337);

	printf("%s\n", argv[0]);

	int is_compatibility_profile = SMOL_FALSE;
	smol_frame_counter_t counter = { 0 };

#ifndef __EMSCRIPTEN__
	GLenum result = glbInit(NULL, NULL);
	smol_frame_gl_spec_t gl_spec = smol_init_gl_spec(3, 3, SMOL_FALSE, SMOL_FALSE, 16, SMOL_TRUE);
#else 
	smol_frame_gl_spec_t gl_spec = smol_init_gl_spec(2, 0, SMOL_FALSE, SMOL_FALSE, 16, SMOL_TRUE);
#endif 
	
	

	smol_frame_config_t frame_config = {
		.width = 800,
		.height = 600,
		.title = "snek 3D",
		.flags = SMOL_FRAME_DEFAULT_CONFIG | SMOL_FRAME_CONFIG_IS_RESIZABLE | SMOL_FRAME_CONFIG_HAS_MAXIMIZE_BUTTON,
		.gl_spec = &gl_spec
	};



	smol_frame_t* frame = smol_frame_create_advanced(&frame_config);

	puts(glGetString(GL_VERSION));
	puts(glGetString(GL_VENDOR));

	#if !defined(__linux__) || !defined(__EMSCRIPTEN__)
	smol_gl_set_vsync(SMOL_TRUE);
	#endif 

	smol_font_t font16px = smol_font_create(
		&PXF_SMOL_FONT_16X16_DATA[0][0][0],
		PXF_SMOL_FONT_16X16_WIDTH, 
		PXF_SMOL_FONT_16X16_HEIGHT,
		&PXF_SMOL_FONT_16X16_OFFSET_X_WIDTH[0][0]
	);

	smol_font_t font7x8px = smol_font_load_pxf("smol_7x8.pxf");


	smol_text_renderer_t text_renderer = smol_text_renderer_create(&font16px, INDICES_PER_65K);

	int occupatency_map[6][ARENA_SIDE * ARENA_SIDE];
	int num_players = 2;


	view_partition_t viewports = create_viewport_partition(0, 0, 800, 600, (vp_spec_t[]) {
		(vp_spec_t) {
			.is_vertical = 1,
			.is_bottom = 1,
			.split_percentage = .2f
		},
		(vp_spec_t) {
			.is_vertical = 0,
			.is_right = 0,
			.split_percentage = num_players > 1 ? .5f : 1.f
		},
		(vp_spec_t) {
			.is_vertical = 0,
			.is_right = 0,
			.split_percentage = 1.0f
		}
	}, 1+num_players);

	//Max 4 players 
	snek_t snake[4] = { 0 };

	for(int i = 0; i < num_players; i++)
		snake[i].cells = malloc(sizeof(snek_cell_t) * 1000);

	
	face_data_t faces[6] = {
		{ //Left face
			.hor_neighbors = { ARENA_PLANE_BOTTOM, ARENA_PLANE_TOP },
			.ver_neighbors = { ARENA_PLANE_BACK, ARENA_PLANE_FRONT },
			.positive_per_axis_dir = { 1, -1 },
			.vec_idx = { 1, 2 },
			.adj_access_axis = { INVALID_AXIS }
		},
		{ //Bottom face
			.hor_neighbors = { ARENA_PLANE_LEFT, ARENA_PLANE_RIGHT },
			.ver_neighbors = { ARENA_PLANE_BACK, ARENA_PLANE_FRONT },
			.positive_per_axis_dir = { 1, 1 },
			.vec_idx = { 0, 2 },
			.adj_access_axis = { INVALID_AXIS }
		},
		{ //Front face
			.hor_neighbors = { ARENA_PLANE_LEFT, ARENA_PLANE_RIGHT },
			.ver_neighbors = { ARENA_PLANE_BOTTOM, ARENA_PLANE_TOP }, 
			.positive_per_axis_dir = { 1, 1 },
			.vec_idx = {0, 1},
			.adj_access_axis = { INVALID_AXIS }
		},
		{ //Right face
			.hor_neighbors = { ARENA_PLANE_BOTTOM, ARENA_PLANE_TOP },
			.ver_neighbors = { ARENA_PLANE_BACK, ARENA_PLANE_FRONT },
			.positive_per_axis_dir = { -1,-1 },
			.vec_idx = {1, 2},
			.adj_access_axis = { INVALID_AXIS }
		},
		{ //Top face
			.hor_neighbors = { ARENA_PLANE_LEFT, ARENA_PLANE_RIGHT },
			.ver_neighbors = { ARENA_PLANE_BACK, ARENA_PLANE_FRONT },
			.positive_per_axis_dir = { 1,-1 },
			.vec_idx = {0, 2},
			.adj_access_axis = { INVALID_AXIS }
		}, 
		{ //Back face
			.hor_neighbors = { ARENA_PLANE_LEFT, ARENA_PLANE_RIGHT },
			.ver_neighbors = { ARENA_PLANE_BOTTOM, ARENA_PLANE_TOP },
			.positive_per_axis_dir = { 1, -1 },
			.vec_idx = { 0, 1 },
			.adj_access_axis = { INVALID_AXIS }
		}
	};

	//Top plane
	faces[ARENA_PLANE_TOP].adj_access_axis[ARENA_PLANE_LEFT] = (dir_scalar_t){ X_AXIS, 1 };
	faces[ARENA_PLANE_TOP].adj_access_axis[ARENA_PLANE_RIGHT] = (dir_scalar_t){ X_AXIS, -1 };
	faces[ARENA_PLANE_TOP].adj_access_axis[ARENA_PLANE_FRONT] = (dir_scalar_t){ Z_AXIS, -1 };
	faces[ARENA_PLANE_TOP].adj_access_axis[ARENA_PLANE_BACK] = (dir_scalar_t){ Z_AXIS, 1 };

	//Right plane
	faces[ARENA_PLANE_RIGHT].adj_access_axis[ARENA_PLANE_TOP] = (dir_scalar_t){ Y_AXIS, -1 };
	faces[ARENA_PLANE_RIGHT].adj_access_axis[ARENA_PLANE_BOTTOM] = (dir_scalar_t){ Y_AXIS, 1 };
	faces[ARENA_PLANE_RIGHT].adj_access_axis[ARENA_PLANE_BACK] = (dir_scalar_t){ Z_AXIS, 1 };
	faces[ARENA_PLANE_RIGHT].adj_access_axis[ARENA_PLANE_FRONT] = (dir_scalar_t){ Z_AXIS, -1 };

	//Back plane
	faces[ARENA_PLANE_BACK].adj_access_axis[ARENA_PLANE_TOP] = (dir_scalar_t){ Y_AXIS, -1 };
	faces[ARENA_PLANE_BACK].adj_access_axis[ARENA_PLANE_BOTTOM] = (dir_scalar_t){ Y_AXIS, 1 };
	faces[ARENA_PLANE_BACK].adj_access_axis[ARENA_PLANE_LEFT] = (dir_scalar_t){ X_AXIS, 1 };
	faces[ARENA_PLANE_BACK].adj_access_axis[ARENA_PLANE_RIGHT] = (dir_scalar_t){ X_AXIS, -1 };

	//Bottom plane
	faces[ARENA_PLANE_BOTTOM].adj_access_axis[ARENA_PLANE_LEFT] = (dir_scalar_t){ X_AXIS, 1 };
	faces[ARENA_PLANE_BOTTOM].adj_access_axis[ARENA_PLANE_RIGHT] = (dir_scalar_t){ X_AXIS, -1 };
	faces[ARENA_PLANE_BOTTOM].adj_access_axis[ARENA_PLANE_FRONT] = (dir_scalar_t){ Z_AXIS, -1 };
	faces[ARENA_PLANE_BOTTOM].adj_access_axis[ARENA_PLANE_BACK] = (dir_scalar_t){ Z_AXIS, 1 };

	//Left plane
	faces[ARENA_PLANE_LEFT].adj_access_axis[ARENA_PLANE_TOP] = (dir_scalar_t){ Y_AXIS, -1};
	faces[ARENA_PLANE_LEFT].adj_access_axis[ARENA_PLANE_BOTTOM] = (dir_scalar_t){ Y_AXIS, 1};
	faces[ARENA_PLANE_LEFT].adj_access_axis[ARENA_PLANE_BACK] = (dir_scalar_t){ Z_AXIS, 1};
	faces[ARENA_PLANE_LEFT].adj_access_axis[ARENA_PLANE_FRONT] =(dir_scalar_t){  Z_AXIS, -1};

	//Front plane
	faces[ARENA_PLANE_FRONT].adj_access_axis[ARENA_PLANE_TOP] = (dir_scalar_t){ Y_AXIS, -1 };
	faces[ARENA_PLANE_FRONT].adj_access_axis[ARENA_PLANE_BOTTOM] = (dir_scalar_t){ Y_AXIS, 1 };
	faces[ARENA_PLANE_FRONT].adj_access_axis[ARENA_PLANE_LEFT] = (dir_scalar_t){ X_AXIS, 1 };
	faces[ARENA_PLANE_FRONT].adj_access_axis[ARENA_PLANE_RIGHT] = (dir_scalar_t){ X_AXIS, -1 };

	//snek_cell_t snek_pieces[5000];

	int plane_remap[3] = {0, 1, 2};
	int side_remap[3][2] = { { 0, 1 }, { 0, 1 }, { 1, 0 } };
	const float snek_scale = 1.f / (float)ARENA_SIDE;
	int failed = 0;

	for(int i = 0; i < num_players; i++) {
		int plane_index = smol_rnd(0, 3);
		int side_index = smol_rnd(0, 2);

		snake[i].len = 3;
		snake[i].plane = plane_remap[plane_index] + (3 * side_remap[plane_index][side_index]);
		snake[i].heading = (iv3){ 0, 0, 0 };
		snake[i].direction = (iv3){ 0, 0, 0 };

		snake[i].heading.c[faces[snake[i].plane].vec_idx[0]] = faces[snake[i].plane].positive_per_axis_dir[0];

		snake[i].cells[0].loc[plane_remap[(snake[i].plane + 0) % 3]] = (snake[i].plane < 3) ? -1 : ARENA_SIDE;
		snake[i].cells[0].loc[plane_remap[(snake[i].plane + 1) % 3]] = ARENA_SIDE / 2;
		snake[i].cells[0].loc[plane_remap[(snake[i].plane + 2) % 3]] = ARENA_SIDE / 2;
		snake[i].cells[0].plane = snake[i].plane;

		snake[i].next_step = SPEED_FORMULA(0);
		snake[i].move_step = snake[i].next_step;
	}

	snake[0].color = smol_v4(0.f, 0.75f, 1.f, 1.f);
	snake[0].key_up = SMOLK_W;
	snake[0].key_right = SMOLK_D;
	snake[0].key_down = SMOLK_S;
	snake[0].key_left = SMOLK_A;

	snake[1].color = smol_v4(1.f, 0.75f, 0.f, 1.f);
	snake[1].key_up = SMOLK_UP;
	snake[1].key_right = SMOLK_RIGHT;
	snake[1].key_down = SMOLK_DOWN;
	snake[1].key_left = SMOLK_LEFT;
	

	int pickup_pos[3] = {0};
	int pickup_plane = 0;
	smol_v3_t pickup_draw_coords = { 0.f };
#define RANDOM_PICKUP_POS() \
	{  \
		int plane_index = smol_rnd(0, 3); \
		int side_index = smol_rnd(0, 2); \
		 \
		pickup_pos[(plane_index + 1) % 3] = smol_rnd(0, ARENA_SIDE); \
		pickup_pos[(plane_index + 2) % 3] = smol_rnd(0, ARENA_SIDE); \
		pickup_pos[plane_index] = -1 + side_index * (ARENA_SIDE+1); \
		pickup_plane = plane_remap[plane_index] + (3*side_remap[plane_index][side_index]); \
		\
		pickup_draw_coords = smol_v3( \
			(float)pickup_pos[0] * snek_scale - snek_scale * (float)ARENA_SIDE * .5f + snek_scale*.5f, \
			(float)pickup_pos[1] * snek_scale - snek_scale * (float)ARENA_SIDE * .5f + snek_scale*.5f, \
			(float)pickup_pos[2] * snek_scale - snek_scale * (float)ARENA_SIDE * .5f + snek_scale*.5f \
		); \
	}

	RANDOM_PICKUP_POS()

	int num_cubes = 0;



	smol_v3_t verts[] = {
		{-.5f, -.5f, -.5f},
		{-.5f, +.5f, -.5f},
		{+.5f, +.5f, -.5f},
		{+.5f, -.5f, -.5f},
		{-.5f, -.5f, +.5f},
		{-.5f, +.5f, +.5f},
		{+.5f, +.5f, +.5f},
		{+.5f, -.5f, +.5f},
	};

	GLuint strip[] = {
		0, 3, 
		4, 7, 
		6, 3, 
		2, 0, 
		1, 4, 
		5, 6, 
		1, 2
	};

	
	for(int j = 0; j < num_players; j++) {
		int plane = snake[j].plane;
		int x_dir_idx = faces[plane].vec_idx[0];
		for(int i = 1; i < snake[j].len; i++) {
			//snake[0].cells[i].x = snake[0].cells[i - 1].x-1;
			//snake[0].cells[i].y = snake[0].cells[i - 1].y;
			//snake[0].cells[i].z = snake[0].cells[i - 1].z;
			snake[j].cells[i].loc[x_dir_idx] = snake[j].cells[i - 1].loc[x_dir_idx];
			int next_axis = (x_dir_idx + 1) % 3;
			snake[j].cells[i].loc[next_axis] = snake[j].cells[i - 1].loc[next_axis];
			next_axis = (x_dir_idx + 2) % 3;
			snake[j].cells[i].loc[next_axis] = snake[j].cells[i - 1].loc[next_axis];
			snake[j].cells[i].plane = snake[j].cells[i - 1].plane;
				//occupied_arena[snek_pieces[i].x + snek_pieces[i].y * ARENA_W] = 1;
		}
	}

#ifndef __EMSCRIPTEN__
	glDebugMessageCallback(debug_message_callback, NULL);
	GLuint snek_vertex_shader, snek_fragment_shader;
	snek_vertex_shader = create_and_compile_shader_from_file("shaders/snek3d.vert", GL_VERTEX_SHADER);
	snek_fragment_shader = create_and_compile_shader_from_file("shaders/snek3d.frag", GL_FRAGMENT_SHADER);

	GLuint text_vertex_shader, text_fragment_shader;
	text_vertex_shader = create_and_compile_shader_from_file("shaders/text.vert", GL_VERTEX_SHADER);
	text_fragment_shader = create_and_compile_shader_from_file("shaders/text.frag", GL_FRAGMENT_SHADER);
#else

	GLuint snek_vertex_shader, snek_fragment_shader;
	snek_vertex_shader = create_and_compile_shader_from_file("shaders/snek3d_oes.vert", GL_VERTEX_SHADER);
	SMOL_ASSERT(snek_vertex_shader != GL_INVALID_INDEX);

	snek_fragment_shader = create_and_compile_shader_from_file("shaders/snek3d_oes.frag", GL_FRAGMENT_SHADER);
	SMOL_ASSERT(snek_fragment_shader != GL_INVALID_INDEX);

	GLuint text_vertex_shader, text_fragment_shader;
	text_vertex_shader = create_and_compile_shader_from_file("shaders/text_oes.vert", GL_VERTEX_SHADER);
	SMOL_ASSERT(text_vertex_shader != GL_INVALID_INDEX);

	text_fragment_shader = create_and_compile_shader_from_file("shaders/text_oes.frag", GL_FRAGMENT_SHADER);
	SMOL_ASSERT(text_fragment_shader != GL_INVALID_INDEX);

#endif 

	GLuint snek_program = 0;
	snek_program = create_shader_program((GLuint[]) {snek_vertex_shader, snek_fragment_shader}, 2);

	GLuint text_program = 0;
	text_program = create_shader_program((GLuint[]) {text_vertex_shader, text_fragment_shader}, 2);

	smol_text_renderer_set_texture_uniform_slot(&text_renderer, glGetUniformLocation(text_program, "uTexture0"), 0);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);

	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	GLuint pos, ibo, inst;
	glGenBuffers(1, &pos);
	glGenBuffers(1, &ibo);
	glGenBuffers(1, &inst);


	glBindBuffer(GL_ARRAY_BUFFER, pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), (const void*)verts, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(strip), (const void*)strip, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, inst);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_inst_t)*MAX_NUM_CUBES, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint vao;
	glGenVertexArrays(1, &vao);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, inst);
	glEnableVertexAttribArray(0);
	glVertexAttribDivisor(0, 1);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(cube_inst_t), smol_offset_of(cube_inst_t, pos_scale));

	glEnableVertexAttribArray(1);
	glVertexAttribDivisor(1, 1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(cube_inst_t), smol_offset_of(cube_inst_t, color));

	glBindBuffer(GL_ARRAY_BUFFER, pos);
	glVertexAttribDivisor(2, 0);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(smol_v3_t), 0);
	glBindVertexArray(0);


#define ADD_CUBE() cube_instances[num_cubes++]

#ifndef __EMSCRIPTEN__
	cube_inst_t* cube_instances = NULL;
#	define BEGIN_CUBES() \
	num_cubes=1; \
	glBindBuffer(GL_ARRAY_BUFFER, inst); \
	SMOL_ASSERT((cube_instances = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE)) != NULL)

#	define END_CUBES() \
		glUnmapBuffer(GL_ARRAY_BUFFER); \
		glBindBuffer(GL_ARRAY_BUFFER, 0)
#else 
	GLuint buffer_alloc = sizeof(cube_inst_t) * MAX_NUM_CUBES;
	cube_inst_t* cube_instances = malloc(buffer_alloc);

#	define BEGIN_CUBES() \
		num_cubes=1

#	define END_CUBES() \
		glBindBuffer(GL_ARRAY_BUFFER, inst); \
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(cube_inst_t) * num_cubes, cube_instances); \
		glBindBuffer(GL_ARRAY_BUFFER, 0)
#endif 


	BEGIN_CUBES();

	//glBindBuffer(GL_ARRAY_BUFFER, inst);

	cube_instances[0] = (cube_inst_t){
		.pos_scale = { 0.f, 0.f, 0.f, 1.f },
		.color = { 1.f, 1.f, 0.f, 1.f }
	};

	END_CUBES();
	

	GLuint mvp_location = glGetUniformLocation(snek_program, "uMVP");
	GLuint time_location = glGetUniformLocation(snek_program, "uTime");

	GLuint mvp_text_location = glGetUniformLocation(text_program, "uMVP");
	//glClearColor(1.f, 1.f, 1.f, 1.0f);

	float fov = 45.f;
	smol_m4_t snek_projection_mat[4];
	smol_m4_t game_overlay_2d_projection[4];
	smol_m4_t snek_view_mat = smol_m4_translate_xyz(0.f, 0.f, 2.f);
	smol_m4_t text_projection = { 0.f };
	smol_m4_t view_rot = smol_m4_identity();


	int w, h;
	get_viewport_size(&viewports, 0, &w, &h);
	text_projection = smol_m4_ortho_lh(0.f, (float)w, 0.f,  (float)h, -1.f, 1.f);	

	float aratio;
	for(int i = 0; i < num_players; i++) {
		get_viewport_aspect_ratio(&viewports, 1+i, &aratio);
		snek_projection_mat[i] = smol_m4_perspective_lh(SMOL_DEG_TO_RAD(fov), aratio, 1e-2f, 1e+3f);

		get_viewport_size(&viewports, 1+i, &w, &h);
		game_overlay_2d_projection[i] = smol_m4_ortho_lh(0.f, (float)w, 0.f, (float)h, -1.f, 1.f);
	}
#define DEBUG_CAMERA 0

	smol_quat_t cur_orient[4] = {
		smol_quat_identity(),
		smol_quat_identity(),
		smol_quat_identity(),
		smol_quat_identity()
	};
	
#if DEBUG_CAMERA
	smol_v3_t camera_dir = { 0.f };
	smol_v3_t camera_pos = smol_v3(0.f, 1.2f, 1.2f);
	smol_v3_t camera_side = smol_v3(1.f, 0.f, 0.f);
	float rot_y = 0.f;
	float rot_x = SMOL_DEG_TO_RAD(-30.f);

	camera_dir.x =-sinf(rot_y) * cosf(rot_x);
	camera_dir.z =-cosf(rot_y) * cosf(rot_x);
	camera_dir.y = sinf(rot_x);
		
	camera_side.x = cos(rot_y);
	camera_side.z =-sin(rot_y);
#endif 

	
	double loop_start = smol_timer();
	double dt = 0.;

	while(!smol_frame_is_closed(frame)) {

		double frame_start = smol_timer();
		double time = (frame_start - loop_start);

		if(dt > 0.1) dt = 0.;

		smol_frame_update(frame);
		smol_inputs_flush();

		for(smol_frame_event_t e; smol_frame_acquire_event(frame, &e);) {
			switch(e.type) {
				case SMOL_FRAME_EVENT_RESIZE: {
					//glViewport(0, 0, e.size.width, e.size.height);
					update_view_partition(&viewports, 0, 0, e.size.width, e.size.height);

					int w, h;
					get_viewport_size(&viewports, 0, &w, &h);
					text_projection = smol_m4_ortho_lh(0.f, (float)w, 0.f,  (float)h, -1.f, 1.f);

					for(int i = 0; i < num_players; i++) {

						float aratio;
						get_viewport_aspect_ratio(&viewports, 1+i, &aratio);
						snek_projection_mat[i] = smol_m4_perspective_lh(SMOL_DEG_TO_RAD(fov), aratio, 1e-2f, 1e+3f);

						get_viewport_size(&viewports, 1+i, &w, &h);
						game_overlay_2d_projection[i] = smol_m4_ortho_lh(0.f, (float)w, 0.f, (float)h, -1.f, 1.f);

					}
				} break;
				default: break;
			}
			smol_inputs_update(&e);
		}


	#if DEBUG_CAMERA
		if(smol_mouse_hit(1)) smol_frame_set_cursor_visibility(frame, 0);
		if(smol_mouse_up(1)) smol_frame_set_cursor_visibility(frame, 1);

		if(smol_mouse_down(1)) {

			rot_y -= (float)smol_mouse_move_x()*(float)SMOL_PI/180.f;
			rot_x -= (float)smol_mouse_move_y()*(float)SMOL_PI/180.f;

			camera_dir.x =-sinf(rot_y) * cosf(rot_x);
			camera_dir.z =-cosf(rot_y) * cosf(rot_x);
			camera_dir.y = sinf(rot_x);
		
			camera_side.x = cos(rot_y);
			camera_side.z =-sin(rot_y);

		}

		if(smol_key_down(SMOLK_W)) camera_pos = smol_v3_add(camera_pos, smol_v3_mul(dt*10.f, camera_dir));
		if(smol_key_down(SMOLK_S)) camera_pos = smol_v3_sub(camera_pos, smol_v3_mul(dt*10.f, camera_dir));

		if(smol_key_down(SMOLK_D)) camera_pos = smol_v3_add(camera_pos, smol_v3_mul(dt*10.f, camera_side));
		if(smol_key_down(SMOLK_A)) camera_pos = smol_v3_sub(camera_pos, smol_v3_mul(dt*10.f, camera_side));

	#endif 

		for(int i = 0; i < num_players; i++) {

			if(smol_key_hit(snake[i].key_up) && snake[i].heading.c[faces[snake[i].plane].vec_idx[0]] != 0) {
				snake[i].heading.c[faces[snake[i].plane].vec_idx[1]] = faces[snake[i].plane].positive_per_axis_dir[1];
				snake[i].heading.c[faces[snake[i].plane].vec_idx[0]] = 0;
			}

			if(smol_key_hit(snake[i].key_down) && snake[i].heading.c[faces[snake[i].plane].vec_idx[0]] != 0) {
				snake[i].heading.c[faces[snake[i].plane].vec_idx[1]] = -faces[snake[i].plane].positive_per_axis_dir[1];
				snake[i].heading.c[faces[snake[i].plane].vec_idx[0]] = 0;
			}

			if(smol_key_hit(snake[i].key_left) && snake[i].heading.c[faces[snake[i].plane].vec_idx[1]] != 0) {
				snake[i].heading.c[faces[snake[i].plane].vec_idx[1]] = 0;
				snake[i].heading.c[faces[snake[i].plane].vec_idx[0]] = -faces[snake[i].plane].positive_per_axis_dir[0];
			}

			if(smol_key_hit(snake[i].key_right) && snake[i].heading.c[faces[snake[i].plane].vec_idx[1]] != 0) {
				snake[i].heading.c[faces[snake[i].plane].vec_idx[1]] = 0;
				snake[i].heading.c[faces[snake[i].plane].vec_idx[0]] = faces[snake[i].plane].positive_per_axis_dir[0];
			}

		}


		for(int pl_idx = 0; pl_idx < num_players; pl_idx++) {
	#if 1
			if(snake[pl_idx].move_step <= 0.) {

				snake[pl_idx].direction.c[0] = snake[pl_idx].heading.c[0];
				snake[pl_idx].direction.c[1] = snake[pl_idx].heading.c[1];
				snake[pl_idx].direction.c[2] = snake[pl_idx].heading.c[2];

				//Remove occupation from tail
				
				snek_cell_t* cell = &snake[pl_idx].cells[snake[pl_idx].len-1];
				
				char* axis = faces[cell->plane].vec_idx;

				//occupatency_map[cell->plane][cell->loc[axis[0]] + cell->loc[axis[1]] * ARENA_SIDE] = 0;
				
				//printf("Starting to update player #%d cells..", pl_idx);
				for(int i = snake[pl_idx].len - 1; i > 0; i--) {
					snake[pl_idx].cells[i].x = snake[pl_idx].cells[i - 1].x;
					snake[pl_idx].cells[i].y = snake[pl_idx].cells[i - 1].y;
					snake[pl_idx].cells[i].z = snake[pl_idx].cells[i - 1].z;
					snake[pl_idx].cells[i].plane = snake[pl_idx].cells[i - 1].plane;
				}
				//printf("Ending to update player #%d cells..", pl_idx);
	

				snake[pl_idx].cells[0].loc[faces[snake[pl_idx].plane].vec_idx[0]] += snake[pl_idx].direction.c[faces[snake[pl_idx].plane].vec_idx[0]];
				snake[pl_idx].cells[0].loc[faces[snake[pl_idx].plane].vec_idx[1]] += snake[pl_idx].direction.c[faces[snake[pl_idx].plane].vec_idx[1]];

				//"Horizontally" under bounds
				if(snake[pl_idx].cells[0].loc[faces[snake[pl_idx].plane].vec_idx[0]] < 0 && snake[pl_idx].direction.c[faces[snake[pl_idx].plane].vec_idx[0]] < 0) {
					int axis = faces[snake[pl_idx].plane].vec_idx[0];
					int prev_plane = snake[pl_idx].plane;
					snake[pl_idx].direction.c[axis] = snake[pl_idx].heading.c[axis] = 0;
					snake[pl_idx].plane = faces[snake[pl_idx].plane].hor_neighbors[0];
					axis = faces[snake[pl_idx].plane].vec_idx[0];
					snake[pl_idx].direction.c[faces[snake[pl_idx].plane].adj_access_axis[prev_plane].prev_face] = 
					snake[pl_idx].heading.c[faces[snake[pl_idx].plane].adj_access_axis[prev_plane].prev_face] = faces[snake[pl_idx].plane].adj_access_axis[prev_plane].scalar;
				}

				//"Horizontally" over bounds
				if(snake[pl_idx].cells[0].loc[faces[snake[pl_idx].plane].vec_idx[0]] > (ARENA_SIDE-1) && snake[pl_idx].direction.c[faces[snake[pl_idx].plane].vec_idx[0]] > 0) {
					int axis = faces[snake[pl_idx].plane].vec_idx[0];
					int prev_plane = snake[pl_idx].plane;
					snake[pl_idx].direction.c[axis] = snake[pl_idx].heading.c[axis] = 0;
					snake[pl_idx].plane = faces[snake[pl_idx].plane].hor_neighbors[1];
					axis = faces[snake[pl_idx].plane].vec_idx[0];
					snake[pl_idx].direction.c[faces[snake[pl_idx].plane].adj_access_axis[prev_plane].prev_face] =
					snake[pl_idx].heading.c[faces[snake[pl_idx].plane].adj_access_axis[prev_plane].prev_face] = faces[snake[pl_idx].plane].adj_access_axis[prev_plane].scalar;
				}

				//"Vertically" under bounds
				if(snake[pl_idx].cells[0].loc[faces[snake[pl_idx].plane].vec_idx[1]] < 0 && snake[pl_idx].direction.c[faces[snake[pl_idx].plane].vec_idx[1]] < 0) {
					int axis = faces[snake[pl_idx].plane].vec_idx[1];
					int prev_plane = snake[pl_idx].plane;
					snake[pl_idx].direction.c[axis] = snake[pl_idx].heading.c[axis] = 0;
					snake[pl_idx].plane = faces[snake[pl_idx].plane].ver_neighbors[0];
					axis = faces[snake[pl_idx].plane].vec_idx[0];
					snake[pl_idx].direction.c[faces[snake[pl_idx].plane].adj_access_axis[prev_plane].prev_face] = 
					snake[pl_idx].heading.c[faces[snake[pl_idx].plane].adj_access_axis[prev_plane].prev_face] = faces[snake[pl_idx].plane].adj_access_axis[prev_plane].scalar;
				}

				//"Vertically" over bounds
				if(snake[pl_idx].cells[0].loc[faces[snake[pl_idx].plane].vec_idx[1]] > (ARENA_SIDE-1) && snake[pl_idx].direction.c[faces[snake[pl_idx].plane].vec_idx[1]] > 0) {
					int axis = faces[snake[pl_idx].plane].vec_idx[1];
					int prev_plane = snake[pl_idx].plane;
					snake[pl_idx].direction.c[axis] = snake[pl_idx].heading.c[axis] = 0;
					snake[pl_idx].plane = faces[snake[pl_idx].plane].ver_neighbors[1];
					axis = faces[snake[pl_idx].plane].vec_idx[0];
					snake[pl_idx].direction.c[faces[snake[pl_idx].plane].adj_access_axis[prev_plane].prev_face] = 
					snake[pl_idx].heading.c[faces[snake[pl_idx].plane].adj_access_axis[prev_plane].prev_face] = faces[snake[pl_idx].plane].adj_access_axis[prev_plane].scalar;
				}

				//Add occupation to head
				//occupied_arena[snek_pieces[0].x + snek_pieces[0].y * ARENA_W] = 1;

				for(int i = 1; i < snake[pl_idx].len; i++) {
					if(
						snake[pl_idx].cells[0].x == snake[pl_idx].cells[i].x && 
						snake[pl_idx].cells[0].y == snake[pl_idx].cells[i].y && 
						snake[pl_idx].cells[0].z == snake[pl_idx].cells[i].z
					) {
						failed = 1;
						break;
					}
				}
			
				if(
					snake[pl_idx].cells[0].x == pickup_pos[0] && 
					snake[pl_idx].cells[0].y == pickup_pos[1] &&
					snake[pl_idx].cells[0].z == pickup_pos[2]
				) {
					snake[pl_idx].cells[snake[pl_idx].len].x = snake[pl_idx].cells[snake[pl_idx].len-1].x;
					snake[pl_idx].cells[snake[pl_idx].len].y = snake[pl_idx].cells[snake[pl_idx].len-1].y;
					snake[pl_idx].cells[snake[pl_idx].len].z = snake[pl_idx].cells[snake[pl_idx].len-1].z;
				
				
					RANDOM_PICKUP_POS()
					//while(occupied_arena[pickup_x + pickup_y * ARENA_W]) {
					//	pickup_x = rand() % ARENA_W;
					//	pickup_y = rand() % ARENA_H;
					//}

					snake[pl_idx].len++;

					snake[pl_idx].next_step = SPEED_FORMULA(0);
					if(snake[pl_idx].next_step < 0.) snake[pl_idx].next_step = 0.;

				}

				cell = &snake[pl_idx].cells[0];
				//occupatency_map[cell->plane][cell->loc[axis[0]] + cell->loc[axis[1]] * ARENA_SIDE] = pl_idx;

				snake[pl_idx].move_step = snake[pl_idx].next_step;

			}
			else {
				snake[pl_idx].move_step -= dt;
			}
		#endif 

			//snek_view_mat = smol_m4_translate_vec(smol_v3_neg(camera_pos));
			//smol_m4_t view_rot = smol_m4_mul(smol_m4_rotate_x(rot_x), smol_m4_rotate_y(rot_y));
			smol_quat_t orient = { 0.f };



		
			
			switch(snake[pl_idx].plane) {
				case ARENA_PLANE_LEFT:
					orient = smol_quat_rotate_z(SMOL_PI * .5f);
				break;
				case ARENA_PLANE_BOTTOM:
					orient = smol_quat_rotate_x(-SMOL_PI);
				break;
				case ARENA_PLANE_FRONT:
					orient = smol_quat_rotate_x(SMOL_PI * .5f);
				break;
				case ARENA_PLANE_RIGHT:
					orient = smol_quat_rotate_z(-SMOL_PI * .5f);
				break;
				case ARENA_PLANE_TOP:
					orient = smol_quat_identity();
				break;
				case ARENA_PLANE_BACK:
					orient = smol_quat_rotate_x(-SMOL_PI * .5f);
				break;
			}
		
			//snek_view_mat = smol_m4_mul(orient, smol_m4_mul(view_rot, snek_view_mat));

			smol_v3_t camera_pos = smol_v3(0.f, 1.45f,  1.45f);

			cur_orient[pl_idx] = smol_quat_norm(smol_quat_lerp(cur_orient[pl_idx], orient, 2.f * dt));


			snek_view_mat = smol_m4_mul(
				smol_m4_look_at(camera_pos, smol_v3(0.f, 1.f, .5f), SMOL_UP_VECTOR), 
				smol_m4_rotate_quat(cur_orient[pl_idx])
			);

			smol_m4_t mvp_mat = smol_m4_mul(snek_projection_mat[pl_idx], snek_view_mat);

			activate_viewport(&viewports, 1+pl_idx, VIEWPORT_BIT | SCISSOR_BIT);
			glClearColor(0.f, 0.1f, 0.25f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glEnable(GL_CULL_FACE);
			glEnable(GL_DEPTH_TEST);

			{
				BEGIN_CUBES();
	
				ADD_CUBE() = (cube_inst_t){
					.pos_scale = {
						pickup_draw_coords.x,
						pickup_draw_coords.y,	
						pickup_draw_coords.z,
						snek_scale,
					},
					.color = smol_v4_mul(0.85f + cosf(time*SMOL_TAU*2.f)*0.15f, smol_v4( 1.f, .5f, 0.f, 1.f ))
				};
			
				for(int k = 0; k < num_players; k++)
				for(int i = 0; i < snake[k].len; i++) {
					ADD_CUBE() = (cube_inst_t){
						.pos_scale = { 
							(float)snake[k].cells[i].x * snek_scale - snek_scale * (float)ARENA_SIDE * .5f + snek_scale*.5f, 
							(float)snake[k].cells[i].y * snek_scale - snek_scale * (float)ARENA_SIDE * .5f + snek_scale*.5f, 
							(float)snake[k].cells[i].z * snek_scale - snek_scale * (float)ARENA_SIDE * .5f + snek_scale*.5f, 
							snek_scale
						},
						.color = i == 0 ? snake[k].color : smol_v4_mul(.75 + cosf((float)i / 8 * 2.f * SMOL_PI) * .25f, smol_v4_mul(.75f, snake[k].color))
					};
				}

				END_CUBES();
			}

			glUseProgram(snek_program);
			glUniformMatrix4fv(mvp_location, 1, GL_TRUE, (const float*)&mvp_mat);
			glUniform1f(time_location, time);

			glBindVertexArray(vao);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
			glDrawElementsInstanced(GL_TRIANGLE_STRIP, 14, GL_UNSIGNED_INT, NULL, num_cubes);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glBindVertexArray(0);

			glDisable(GL_CULL_FACE);
			glDisable(GL_DEPTH_TEST);
			

			glUseProgram(text_program);
		
			glUniformMatrix4fv(mvp_text_location, 1, GL_TRUE, (const float*)&game_overlay_2d_projection[pl_idx]);
		
			//Draw the pickup marker
			smol_text_renderer_set_font(&text_renderer, &font16px);

			smol_text_renderer_begin(&text_renderer);
			if(snake[pl_idx].plane != pickup_plane) {

				smol_v4_t coords = smol_m4_mul_v4(
					mvp_mat, 
					smol_v4_from_v3(pickup_draw_coords, 1.f)
				);

				int x, y, w, h;
				get_viewport_position_size(&viewports, 1, &x, &y, &w, &h);

				float hsw = w>>1;
				float hsh = h>>1;

				coords.x = ((x + w)>>1) + (coords.x / coords.w) * hsw;
				coords.y = ((y + h)>>1) - (coords.y / coords.w) * hsh;
			
				coords.x = fmaxf(fminf(coords.x, 2.f*hsw-16.f), 16.f);
				coords.y = fmaxf(fminf(coords.y, 2.f*hsh-16.f), 16.f);

				smol_text_renderer_add_char(&text_renderer, smol_v2(coords.x - 16.f, coords.y - 16.f), 'P', 2, RGBA(0, 255, 0, 255));

			}

			smol_text_renderer_end(&text_renderer);
			smol_text_renderer_draw(&text_renderer);
		}

	

		
		
		//Draw the "HUD"	
		activate_viewport(&viewports, 0, VIEWPORT_BIT | SCISSOR_BIT);
		glClearColor(.5f, .5f, .5f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		
		
		glUniformMatrix4fv(mvp_text_location, 1, GL_TRUE, (const float*)& text_projection);


		smol_text_renderer_begin(&text_renderer);

	

		for(int i = 0; i < num_players; i++) {
			smol_text_renderer_add_string(&text_renderer, smol_v2(11.f, 11.f+i*16), 1.f, RGBA(0, 0, 0, 128), "Player #%d points: %d", (i + 1), (snake[i].len - 3));
			smol_text_renderer_add_string(&text_renderer, smol_v2(10.f, 10.f + i * 16), 1.f, RGBA(255, 255, 255, 255), "Player #%d points: %d", (i + 1), (snake[i].len - 3));
		}
		smol_text_renderer_add_string(&text_renderer, smol_v2(11.f, 75.f), 1.f, RGBA(0, 0, 0, 128), "FPS: %d", smol_frame_counter_fps(&counter));
		smol_text_renderer_add_string(&text_renderer, smol_v2(10.f, 74.f), 1.f, RGBA(255, 0, 0, 255), "FPS: %d", smol_frame_counter_fps(&counter));

	#ifdef _DEBUG
	#if 1
		smol_text_renderer_add_string(&text_renderer, smol_v2(10.f, 32.f), .75f, RGBA(0, 255, 0, 255), "X: %d, Y: %d, Z: %d | DX: %d, DY: %d, DZ: %d", snake[0].cells[0].x, snake[0].cells[0].y, snake[0].cells[0].z, snake[0].heading.c[0], snake[0].heading.c[1], snake[0].heading.c[2]);
		smol_text_renderer_add_string(&text_renderer, smol_v2(10.f, 44.f), .75f, RGBA(0, 255, 255, 255), "Snek plane: %d \"%s\"\nPoint plane: %d", snake[0].plane, plane_names[snake[0].plane], pickup_plane, plane_names[pickup_plane]);
	#endif 
	#endif 
		smol_text_renderer_end(&text_renderer);
		smol_text_renderer_draw(&text_renderer);

		glUseProgram(0);

		smol_frame_gl_swap_buffers(frame);
		dt = smol_timer() - frame_start;
		smol_frame_counter_update(&counter, dt);

	}


	return 0;
}

const char* shader_type_name(GLenum shader_type) {
	switch(shader_type) {
		case GL_VERTEX_SHADER: return "Vertex Shader";
		case GL_FRAGMENT_SHADER: return "Fragment Shader"; 
		case GL_GEOMETRY_SHADER: return "Geometry Shader"; 
		case GL_TESS_EVALUATION_SHADER: return "Tesselation Evaluation Shader"; 
		case GL_TESS_CONTROL_SHADER: return "Tesselation Control Shader"; 
		case GL_COMPUTE_SHADER: return  "Compute Shader"; 
	}
	return "Not a shader type.";
}

GLuint create_and_compile_shader(const char* code, GLenum shader_type, const char* file_name) {


	char buffer[1024] = {0};
	GLuint shader = glCreateShader(shader_type);

	const GLint lens[] = { strlen(code) };
	const char* codes[] = { code };
	GLsizei len = 0;

	glShaderSource(shader, 1, codes, lens);
	glCompileShader(shader);
	glGetShaderInfoLog(shader, 1024, &len, buffer);

	if(len) {
		if(file_name)
			printf("%s '%s' info log:\n", shader_type_name(shader_type), file_name);
		else 
			printf("%s info log: ", shader_type_name(shader_type));
		puts(buffer);
		return GL_INVALID_INDEX;
	}
	return shader;
}

GLuint create_and_compile_shader_from_file(const char* file_path, GLenum shader_type) {
	
	smol_size_t size;
	GLuint res;
	char* buf = (char*)smol_read_entire_file(file_path, &size);
	if(buf) {
		res = create_and_compile_shader(buf, shader_type, file_path);
		if(res == GL_INVALID_INDEX) {
			puts(buf);
		}
		free(buf);
		return res;
	}
	else {
		fprintf(stderr, "Failed to read file '%s'!", file_path);
	}

	return GL_INVALID_INDEX;

}


GLuint create_shader_program(GLuint* shaders, GLuint count) {

	char buffer[1024] = {0};
	GLuint program = glCreateProgram();
	GLsizei len = 0;

	for(GLuint i = 0; i < count; i++) {
		glAttachShader(program, shaders[i]);
	}

	glLinkProgram(program);

	glGetProgramInfoLog(program, 1024, &len, buffer);

	if(len) {
		puts("Program info log: ");
		puts(buffer);
	}

	return program;

}

void smol_frame_counter_update(smol_frame_counter_t* ctr, double dt) {
	ctr->counter++;
	ctr->delta_accum += dt;
	if(ctr->delta_accum >= 1.) {
		ctr->fps = ctr->counter;
		ctr->spf = ctr->delta_accum / (double)ctr->counter;
		ctr->delta_accum = 0.;
		ctr->counter = 0;
	}
}

int smol_frame_counter_fps(smol_frame_counter_t* ctr) {
	return ctr->fps;
}
