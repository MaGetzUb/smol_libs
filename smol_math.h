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

#ifndef SMOL_MATH_H
#define SMOL_MATH_H

#include <math.h>

#define SMOL_PI    3.1415926535897932384
#define SMOL_TAU   6.2831853071795864768

#define SMOL_DEG_TO_RAD(degs) ((degs)*0.0174532925199432958)
#define SMOL_RAD_TO_DEG(rads) ((rads)*57.2957795130823208768)

#ifndef SMOL_INLINE
#ifdef _MSC_VER
#define SMOL_INLINE __forceinline
#else 
#define SMOL_INLINE inline __attribute__((always_inline))
#endif 
#endif 

typedef union smol_v2_t {
	struct {
		float x, y;
	};
	struct {
		float r, g;
	};
	float v[2];
} smol_v2_t;

typedef union smol_v3_t {
	struct {
		float x, y, z;
	};
	struct {
		float r, g, b;
	};
	smol_v2_t xy;
	float v[3];
} smol_v3_t;


typedef union smol_v4_t {
	struct {
		float x, y, z, w;
	};
	struct {
		float r, g, b, a;
	};
	smol_v3_t xyz;
	float v[4];
} smol_v4_t;

typedef smol_v2_t smol_complex_t;
typedef smol_v4_t smol_quat_t;

typedef union smol_m2_t {
	struct {
		smol_v2_t x;
		smol_v2_t y;
	};
	struct {
		smol_v2_t a;
		smol_v2_t b;
	};
	smol_v2_t rows[2];
	float m[4];
} smol_m2_t;

typedef union smol_m3_t {
	struct {
		smol_v3_t x;
		smol_v3_t y;
		smol_v3_t z;
	};
	struct {
		smol_v3_t a;
		smol_v3_t b;
		smol_v3_t c;
	};
	smol_v3_t rows[3];
	float m[9];
} smol_m3_t;


typedef union smol_m4_t {
	struct {
		smol_v4_t x;
		smol_v4_t y;
		smol_v4_t z;
		smol_v4_t w;
	};
	struct {
		smol_v4_t a;
		smol_v4_t b;
		smol_v4_t c;
		smol_v4_t d;
	};
	smol_v4_t rows[4];
	float m[16];
} smol_m4_t;


static const smol_v3_t SMOL_RIGHT_VECTOR	= { 1.f, 0.f, 0.f };
static const smol_v3_t SMOL_UP_VECTOR		= { 0.f, 1.f, 0.f };
static const smol_v3_t SMOL_FORWARD_VECTOR	= { 0.f, 0.f, 1.f };
static const smol_v3_t SMOL_LEFT_VECTOR		= {-1.f, 0.f, 0.f };
static const smol_v3_t SMOL_DOWN_VECTOR		= { 0.f,-1.f, 0.f };
static const smol_v3_t SMOL_BACKWARD_VECTOR	= { 0.f, 0.f,-1.f };

//Vector math:
smol_v2_t smol_v2(float x, float y);
smol_v2_t smol_v2_neg(smol_v2_t v);
smol_v2_t smol_v2_sub(smol_v2_t a, smol_v2_t b);
smol_v2_t smol_v2_add(smol_v2_t a, smol_v2_t b);
smol_v2_t smol_v2_mul(float a, smol_v2_t b);
smol_v2_t smol_v2_hadam(smol_v2_t a, smol_v2_t b);
smol_v2_t smol_v2_div(smol_v2_t a, float b) ;
float smol_v2_dot(smol_v2_t a, smol_v2_t b);
float smol_v2_wedge(smol_v2_t a, smol_v2_t b);
float smol_v2_len_sq(smol_v2_t v);
float smol_v2_len(smol_v2_t v);
smol_v2_t smol_v2_norm(smol_v2_t v);
smol_v2_t smol_v2_mix(smol_v2_t a, smol_v2_t b, float t);

smol_v3_t smol_v3(float x, float y, float z);
smol_v3_t smol_v3_neg(smol_v3_t v);
smol_v3_t smol_v3_sub(smol_v3_t a, smol_v3_t b);
smol_v3_t smol_v3_add(smol_v3_t a, smol_v3_t b);
smol_v3_t smol_v3_mul(float a, smol_v3_t b);
smol_v3_t smol_v3_hadam(smol_v3_t a, smol_v3_t b);
smol_v3_t smol_v3_div(smol_v3_t a, float b);
float smol_v3_dot(smol_v3_t a, smol_v3_t b);
smol_v3_t smol_v3_cross(smol_v3_t a, smol_v3_t b);
float smol_v3_len_sq(smol_v3_t v);
float smol_v3_len(smol_v3_t v);
smol_v3_t smol_v3_norm(smol_v3_t v);
smol_v3_t smol_v3_mix(smol_v3_t a, smol_v3_t b, float t);

smol_v4_t smol_v4(float x, float y, float z, float w);
smol_v4_t smol_v4_from_v3(smol_v3_t xyz, float w);

smol_v4_t smol_v4_neg(smol_v4_t v);
smol_v4_t smol_v4_sub(smol_v4_t a, smol_v4_t b);
smol_v4_t smol_v4_add(smol_v4_t a, smol_v4_t b);
smol_v4_t smol_v4_mul(float a, smol_v4_t b);
smol_v4_t smol_v4_hadam(smol_v4_t a, smol_v4_t b);
smol_v4_t smol_v4_div(smol_v4_t a, float b);
float smol_v4_dot(smol_v4_t a, smol_v4_t b);
float smol_v4_len_sq(smol_v4_t v);
float smol_v4_len(smol_v4_t v);
smol_v4_t smol_v4_norm(smol_v4_t v);
smol_v4_t smol_v4_mix(smol_v4_t a, smol_v4_t b, float t);

//Quat math:
smol_quat_t smol_quat_identity();
smol_quat_t smol_quat(float r, float i, float j, float k);
smol_quat_t smol_quat_from_axis_angle(smol_v3_t axis, float angle_rad);
smol_quat_t smol_quat_mul(smol_quat_t a, smol_quat_t b);
smol_quat_t smol_quat_conjugate(smol_quat_t q);
smol_v3_t smol_quat_rotate_v3(smol_quat_t q, smol_v3_t v);

#define smol_quat_rotate_x(angle) smol_quat_from_axis_angle((smol_v3_t){1.f, 0.f, 0.f}, angle)
#define smol_quat_rotate_y(angle) smol_quat_from_axis_angle((smol_v3_t){0.f, 1.f, 0.f}, angle)
#define smol_quat_rotate_z(angle) smol_quat_from_axis_angle((smol_v3_t){0.f, 0.f, 1.f}, angle)

//Axis will be stored in the vec4's xyz components and angle in the w component.
smol_v4_t smol_quat_get_axis_angle(smol_quat_t q);

smol_v3_t smol_quat_local_x(smol_quat_t q);
smol_v3_t smol_quat_local_y(smol_quat_t q);
smol_v3_t smol_quat_local_z(smol_quat_t q);

#define smol_quat_norm(q) smol_v4_norm(q)
#define smol_quat_dot(a, b) smol_v4_dot(a, b)
#define smol_quat_neg(q) smol_v4_neg(q)

smol_quat_t smol_quat_lerp(smol_quat_t a, smol_quat_t b, float t);
smol_quat_t smol_quat_slerp(smol_quat_t a, smol_quat_t b, float t);
//Matrix math:

//Matrix 3x3
//Operators:
smol_m3_t smol_m3_transpose(smol_m3_t m);
smol_m3_t smol_m3_mul(smol_m3_t a, smol_m3_t b);
smol_m3_t smol_m3_mul_by_val(smol_m3_t a, float v);
smol_v3_t smol_m3_mul_v3(smol_m3_t a, smol_v3_t v);
float smol_m3_determinant(smol_m3_t m);
smol_m3_t smol_m3_adjugate(smol_m3_t m);
smol_m3_t smol_m3_inverse(smol_m3_t m);

smol_m3_t smol_m3_identity();
smol_m3_t smol_m3_translate_vec(smol_v2_t vec);
smol_m3_t smol_m3_translate_xy(float x, float y);
smol_m3_t smol_m3_uni_scale(float scale);

smol_m3_t smol_m3_rotate_x(float rotation_rad);
smol_m3_t smol_m3_rotate_y(float rotation_rad);
smol_m3_t smol_m3_rotate_z(float rotation_rad);

smol_m4_t smol_m4_rotate_axis_angle(smol_v3_t vec, float rotation_rad);
smol_m4_t smol_m4_rotate_quat(smol_quat_t quat);

//Matrix 4x4:
//Operations:

smol_m4_t smol_m4_transpose(smol_m4_t m);
smol_m4_t smol_m4_mul(smol_m4_t a, smol_m4_t b);
smol_m4_t smol_m4_mul_by_val(smol_m4_t a, float v);
smol_v4_t smol_m4_mul_v4(smol_m4_t m, smol_v4_t v);
float smol_m4_determinant(smol_m4_t m);
smol_m4_t smol_m4_adjugate(smol_m4_t m);
smol_m4_t smol_m4_inverse(smol_m4_t m);


//Initializations:
//Identity
smol_m4_t smol_m4_identity();

//Translate
smol_m4_t smol_m4_translate_vec(smol_v3_t vec);
smol_m4_t smol_m4_translate_xyz(float x, float y, float z);

//Scale
smol_m4_t smol_m4_scale_vec(smol_v3_t vec);
smol_m4_t smol_m4_scale_xyz(float x, float y, float z);
smol_m4_t smol_m4_uni_scale(float scale);

//Rotation:
smol_m4_t smol_m4_rotate_x(float rotation_rad);
smol_m4_t smol_m4_rotate_y(float rotation_rad);
smol_m4_t smol_m4_rotate_z(float rotation_rad);

smol_m4_t smol_m4_rotate_axis_angle(smol_v3_t vec, float rotation_rad);
smol_m4_t smol_m4_rotate_quat(smol_quat_t quat);
smol_m4_t smol_m4_look_at(smol_v3_t view_location, smol_v3_t target_location, smol_v3_t up_vector);

//Projection:
smol_m4_t smol_m4_frustum_lh(
	float left_plane, 
	float right_plane, 
	float top_plane, 
	float bottom_plane, 
	float near_plane, 
	float far_plane
);

smol_m4_t smol_m4_perspective_lh(float fov, float aspect, float near_plane, float far_plane);

smol_m4_t smol_m4_ortho_lh(
	float left_plane, 
	float right_plane, 
	float top_plane, 
	float bottom_plane, 
	float near_plane, 
	float far_plane
);

#endif 



#pragma region smol_v2_t 
SMOL_INLINE smol_v2_t smol_v2(float x, float y) {
	smol_v2_t res = { x, y };
	return res;
}

SMOL_INLINE smol_v2_t smol_v2_neg(smol_v2_t v) {
	smol_v2_t n = {
		-v.x,
		-v.y
	};
	return n;
}

SMOL_INLINE smol_v2_t smol_v2_sub(smol_v2_t a, smol_v2_t b) {
	smol_v2_t v = {
		a.x - b.x,
		a.y - b.y,
	};
	return v;
}

SMOL_INLINE smol_v2_t smol_v2_add(smol_v2_t a, smol_v2_t b) {
	smol_v2_t v = {
		a.x + b.x,
		a.y + b.y,
	};
	return v;
}

SMOL_INLINE smol_v2_t smol_v2_mul(float a, smol_v2_t b) {
	smol_v2_t v = {
		b.x * a,
		b.y * a,
	};
	return v;
}


SMOL_INLINE smol_v2_t smol_v2_hadam(smol_v2_t a, smol_v2_t b) {
	smol_v2_t v = {
		a.x * b.x,
		a.y * b.y,
	};
	return v;
}

SMOL_INLINE smol_v2_t smol_v2_div(smol_v2_t a, float b) {
	float m = 1.f/b;
	return smol_v2_mul(m, a);
}

SMOL_INLINE float smol_v2_dot(smol_v2_t a, smol_v2_t b) {
	return (
		a.x * b.x + 
		a.y * b.y
	);
}

SMOL_INLINE float smol_v2_wedge(smol_v2_t a, smol_v2_t b) {
	return (
		a.x * b.y - 
		a.y * b.x
	);
}

SMOL_INLINE float smol_v2_len_sq(smol_v2_t v) {
	return smol_v2_dot(v, v);
}

SMOL_INLINE float smol_v2_len(smol_v2_t v) {
	return sqrtf(smol_v2_len_sq(v));
}

SMOL_INLINE smol_v2_t smol_v2_norm(smol_v2_t v) {
	float len = smol_v2_len(v);
	return smol_v2_div(v, len);
}

SMOL_INLINE smol_v2_t smol_v2_mix(smol_v2_t a, smol_v2_t b, float t) {
	
	float it = 1.f - t;
	
	smol_v2_t res = {
		a.x * it + b.x * t,
		a.y * it + b.y * t
	};

	return res;

}

#pragma endregion

#pragma region smol_complex_t 
//...
#pragma endregion

#pragma region smol_v3_t 
SMOL_INLINE smol_v3_t smol_v3(float x, float y, float z) {
	smol_v3_t res = { x, y, z };
	return res;
}

SMOL_INLINE smol_v3_t smol_v3_neg(smol_v3_t v) {
	smol_v3_t n = {
		-v.x,
		-v.y,
		-v.z
	};
	return n;
}

SMOL_INLINE smol_v3_t smol_v3_sub(smol_v3_t a, smol_v3_t b) {
	smol_v3_t v = {
		a.x - b.x,
		a.y - b.y,
		a.z - b.z
	};
	return v;
}

SMOL_INLINE smol_v3_t smol_v3_add(smol_v3_t a, smol_v3_t b) {
	smol_v3_t v = {
		a.x + b.x,
		a.y + b.y,
		a.z + b.z
	};
	return v;
}

SMOL_INLINE smol_v3_t smol_v3_mul(float a, smol_v3_t b) {
	smol_v3_t v = {
		b.x * a,
		b.y * a,
		b.z * a
	};
	return v;
}

SMOL_INLINE smol_v3_t smol_v3_hadam(smol_v3_t a, smol_v3_t b) {
	smol_v3_t v = {
		a.x * b.x,
		a.y * b.y,
		a.z * b.z
	};
	return v;
}

SMOL_INLINE smol_v3_t smol_v3_div(smol_v3_t a, float b) {
	float m = 1.f/b;
	return smol_v3_mul(m, a);
}

SMOL_INLINE float smol_v3_dot(smol_v3_t a, smol_v3_t b) {
	return (
		a.x * b.x + 
		a.y * b.y + 
		a.z * b.z
	);
}

SMOL_INLINE smol_v3_t smol_v3_cross(smol_v3_t a, smol_v3_t b) {
	
	smol_v3_t res = {
		a.y * b.z - a.z * b.y,
		a.x * b.z - a.z * b.x,
		a.x * b.y - a.y * b.x
	};

	return res;
}

SMOL_INLINE float smol_v3_len_sq(smol_v3_t v) {
	return smol_v3_dot(v, v);
}

SMOL_INLINE float smol_v3_len(smol_v3_t v) {
	return sqrtf(smol_v3_len_sq(v));
}

SMOL_INLINE smol_v3_t smol_v3_norm(smol_v3_t v) {
	float len = smol_v3_len(v);
	return smol_v3_div(v, len);
}


SMOL_INLINE smol_v3_t smol_v3_mix(smol_v3_t a, smol_v3_t b, float t) {
	
	float it = 1.f - t;
	
	smol_v3_t res = {
		a.x * it + b.x * t,
		a.y * it + b.y * t,
		a.z * it + b.z * t
	};

	return res;

}

#pragma endregion

#pragma region smol_v4_t 

SMOL_INLINE smol_v4_t smol_v4(float x, float y, float z, float w) {
	smol_v4_t res = { x, y, z, w };
	return res;
}

SMOL_INLINE smol_v4_t smol_v4_from_v3(smol_v3_t v, float w) {
	smol_v4_t res = { v.x, v.y, v.z, w };
	return res;
}

SMOL_INLINE smol_v4_t smol_v4_neg(smol_v4_t v) {
	smol_v4_t n = {
		-v.x,
		-v.y,
		-v.z,
		-v.w
	};
	return n;
}

SMOL_INLINE smol_v4_t smol_v4_sub(smol_v4_t a, smol_v4_t b) {
	smol_v4_t v = {
		a.x - b.x,
		a.y - b.y,
		a.z - b.z,
		a.w - b.w
	};
	return v;
}

SMOL_INLINE smol_v4_t smol_v4_add(smol_v4_t a, smol_v4_t b) {
	smol_v4_t v = {
		a.x + b.x,
		a.y + b.y,
		a.z + b.z,
		a.w + b.w
	};
	return v;
}


SMOL_INLINE smol_v4_t smol_v4_mul(float a, smol_v4_t b) {
	smol_v4_t v = {
		b.x * a,
		b.y * a,
		b.z * a,
		b.w * a
	};
	return v;
}

SMOL_INLINE smol_v4_t smol_v4_hadam(smol_v4_t a, smol_v4_t b) {
	smol_v4_t v = {
		a.x * b.x,
		a.y * b.y,
		a.z * b.z,
		a.w * b.w
	};
	return v;
}

SMOL_INLINE smol_v4_t smol_v4_div(smol_v4_t a, float b) {
	float m = 1.f/b;
	return smol_v4_mul(m, a);
}

SMOL_INLINE float smol_v4_dot(smol_v4_t a, smol_v4_t b) {
	return (
		a.x * b.x + 
		a.y * b.y + 
		a.z * b.z +
		a.w * b.w
	);
}

SMOL_INLINE float smol_v4_len_sq(smol_v4_t v) {
	return smol_v4_dot(v, v);
}

SMOL_INLINE float smol_v4_len(smol_v4_t v) {
	return sqrtf(smol_v4_len_sq(v));
}

SMOL_INLINE smol_v4_t smol_v4_norm(smol_v4_t v) {
	float len = smol_v4_len(v);
	return smol_v4_div(v, len);
}

SMOL_INLINE smol_v4_t smol_v4_mix(smol_v4_t a, smol_v4_t b, float t) {
	
	float it = 1.f - t;
	
	smol_v4_t res = {
		a.x * it + b.x * t,
		a.y * it + b.y * t,
		a.z * it + b.z * t,
		a.w * it + b.w * t
	};

	return res;

}

#pragma endregion

#pragma region smol_quat_t 

SMOL_INLINE smol_quat_t smol_quat_identity() {
	smol_quat_t res = { 0.f, 0.f, 0.f, 1.f };
	return res;
}

SMOL_INLINE smol_quat_t smol_quat(float r, float i, float j, float k) {
	smol_quat_t res = { i, j, k, r };
	return res;
}

SMOL_INLINE smol_quat_t smol_quat_conjugate(smol_quat_t quat) {
	smol_quat_t res = { quat.w, -quat.x, -quat.y, -quat.z };
	return res;
}

SMOL_INLINE smol_quat_t smol_quat_mul(smol_quat_t a, smol_quat_t b) {

	smol_quat_t res = {
		.x = ( (a.w * b.x) + (a.x * b.w) + (a.y * b.z) - (a.z * b.y) ),
		.y = ( (a.w * b.y) - (a.x * b.z) + (a.y * b.w) + (a.z * b.x) ),
		.z = ( (a.w * b.z) + (a.x * b.y) - (a.y * b.x) + (a.z * b.w) ),
		.w = ( (a.w * b.w) - (a.x * b.x) - (a.y * b.y) - (a.z * b.z) )
	};

	return res;

}

SMOL_INLINE smol_quat_t smol_quat_from_axis_angle(smol_v3_t axis, float angle) {
	float ang_half = 0.5f * angle;
	float c = cosf(ang_half);
	float s = sinf(ang_half);
	return smol_quat(c, s*axis.x, s*axis.y, s*axis.z);
}


SMOL_INLINE smol_v3_t smol_quat_rotate_v3(smol_quat_t q, smol_v3_t v) {
	
	smol_v3_t lx = smol_quat_local_x(q);
	smol_v3_t ly = smol_quat_local_y(q);
	smol_v3_t lz = smol_quat_local_z(q);

	smol_v3_t res = smol_v3_add(
		smol_v3_add(
			smol_v3_mul(v.x, lx), 
			smol_v3_mul(v.y, ly)
		), smol_v3_mul(v.z, lz)
	);

	return res;

}

SMOL_INLINE smol_v3_t smol_quat_local_x(smol_quat_t q) {

	smol_v3_t res = {
		2.f * (q.w * q.w + q.x * q.x) - 1.f,
		2.f * (q.x * q.y - q.z * q.w) + 0.f,
		2.f * (q.x * q.z + q.y * q.w) + 0.f,
	};

	return res;

}

SMOL_INLINE smol_v3_t smol_quat_local_y(smol_quat_t q) {

	smol_v3_t res = {
		2.f * (q.x*q.y + q.z*q.w) + 0.f,
		2.f * (q.w*q.w + q.y*q.y) - 1.f,
		2.f * (q.y*q.z - q.x*q.w) + 0.f
	};

	return res;

}

SMOL_INLINE smol_v3_t smol_quat_local_z(smol_quat_t q) {
	
	smol_v3_t res = {
		2.f * (q.x*q.z - q.y*q.w) + 0.f,
		2.f * (q.y*q.z + q.x*q.w) + 0.f,
		2.f * (q.w*q.w + q.z*q.z) - 1.f
	};

	return res;

}

SMOL_INLINE smol_v4_t smol_quat_get_axis_angle(smol_quat_t q) {

	smol_v4_t res = { 0.f };

	float ijk_len = smol_v3_len(q.xyz);
	res.xyz = smol_v3_div(q.xyz, ijk_len);
	res.w = atan2f(ijk_len, q.w) * 2.f;

	return res;

}

SMOL_INLINE smol_quat_t smol_quat_lerp(smol_quat_t a, smol_quat_t b, float t) {
	b = smol_quat_dot(a, b) < 0.f ? smol_quat_neg(b) : b;
	return smol_v4_mix(a, b, t);
}

//https://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/slerp/index.htm
SMOL_INLINE smol_quat_t smol_quat_slerp(smol_quat_t a, smol_quat_t b, float t) {

	float cosHalfTheta = smol_v4_dot(a, b);

	if(cosHalfTheta >= 1.f)
		return a;

	float halfTheta = acosf(cosHalfTheta);
	float sinHalfTheta = sqrtf(1.f - cosHalfTheta * cosHalfTheta);

	if(sinHalfTheta < 1e-3)
		return smol_v4_mix(a, b, .5f);

	sinHalfTheta = 1.f / sinHalfTheta;

	float ratio0 = sinf((1.f - t) * cosHalfTheta) * sinHalfTheta;
	float ratio1 = sinf((0.f + t) * cosHalfTheta) * sinHalfTheta;

	return smol_v4_add(
		smol_v4_mul(ratio0, a), 
		smol_v4_mul(ratio1, b)
	);
}

#pragma endregion

#pragma region smol_m3_t 

smol_m3_t smol_m3_transpose(smol_m3_t m) {
	
	smol_m3_t res = {
		m.x.x, m.y.x, m.z.x,
		m.y.x, m.y.x, m.y.x,
		m.z.x, m.z.x, m.z.x,
	};

	return res;

}

smol_m3_t smol_m3_mul(smol_m3_t a, smol_m3_t b) {

	b = smol_m3_transpose(b);

	smol_m3_t res = { 
		.x = { smol_v3_dot(a.x, b.x), smol_v3_dot(a.x, b.y), smol_v3_dot(a.x, b.z) },
		.y = { smol_v3_dot(a.y, b.x), smol_v3_dot(a.y, b.y), smol_v3_dot(a.y, b.z) },
		.z = { smol_v3_dot(a.z, b.x), smol_v3_dot(a.z, b.y), smol_v3_dot(a.z, b.z) }
	};

	return res;

}

smol_m3_t smol_m3_mul_by_val(smol_m3_t a, float f) {


	smol_m3_t res = {
		.x = smol_v3_mul(f, a.x),
		.y = smol_v3_mul(f, a.y),
		.z = smol_v3_mul(f, a.z),
	};

	return res;

}

smol_v3_t smol_m3_mul_v3(smol_m3_t a, smol_v3_t v) {

	smol_v3_t res = {
		smol_v3_dot(a.x, v),
		smol_v3_dot(a.y, v),
		smol_v3_dot(a.z, v),
	};

	return res;

}

float smol_m3_determinant(smol_m3_t m) {

	float res = {
		m.x.x * (m.y.y * m.z.z - m.y.z * m.z.y) -
		m.x.y * (m.y.x * m.z.z - m.y.z * m.z.x) +
		m.x.z * (m.x.x * m.y.y - m.x.y * m.y.x)
	};

	return res;

}

smol_m3_t smol_m3_adjugate(smol_m3_t m) {

	smol_m3_t res = {
		(m.y.y * m.z.z - m.y.z * m.z.y), (m.z.x * m.y.z - m.y.x * m.z.z), (m.y.x * m.z.y - m.z.x * m.y.y),
		(m.z.y * m.x.z - m.x.y * m.z.z), (m.x.x * m.z.z - m.z.x * m.x.z), (m.x.y * m.z.x - m.x.x * m.z.y),
		(m.x.y * m.y.z - m.x.z * m.y.y), (m.x.z * m.y.x - m.x.x * m.y.z), (m.x.x * m.y.y - m.x.y * m.y.x),
	};

	return res;

}

smol_m3_t smol_m3_inverse(smol_m3_t m) {

	float det = smol_m3_determinant(m);

	if(fabsf(det) < 1e-4f) {
		fputs("Derterminat too small!", stderr);
	#ifdef _MSC_VER
		__debugbreak();
	#elif defined(_POSIX_C_SOURCE)
		sigint(SIGTRAP);
	#endif 
	}


	smol_m3_t adj = smol_m3_adjugate(m);

	return smol_m3_mul_by_val(adj, det);

}

smol_m3_t smol_m3_identity() {

	smol_m3_t res = { 
		1.f, 0.f, 0.f, 
		0.f, 1.f, 0.f, 
		0.f, 0.f, 1.f 
	};
	
	return res;

}

smol_m3_t smol_m3_translate_vec(smol_v2_t vec) {

	smol_m3_t res = { 
		1.f, 0.f, vec.x, 
		0.f, 1.f, vec.y, 
		0.f, 0.f, 1.f 
	};
	
	return res;
}

smol_m3_t smol_m3_translate_xy(float x, float y) {
	
	smol_m3_t res = { 
		1.f, 0.f, x, 
		0.f, 1.f, y, 
		0.f, 0.f, 1.f 
	};
	
	return res;

}

smol_m3_t smol_m3_uni_scale(float scale) {
	
	smol_m3_t res = { 
		scale, 0.f, 0.f, 
		0.f, scale, 0.f, 
		0.f, 0.f, scale 
	};
	
	return res;

}

smol_m3_t smol_m3_rotate_x(float rotation_rad) {

	smol_m3_t res = { 0.f };

	float r = cosf(rotation_rad);
	float i = sinf(rotation_rad);

	res.y.y = +r;
	res.z.y = -i;
	res.y.z = +i;
	res.z.z = +r;

	return res;

}

smol_m3_t smol_m3_rotate_y(float rotation_rad) {

	smol_m3_t res = { 0.f };

	float r = cosf(rotation_rad);
	float i = sinf(rotation_rad);

	res.x.x = +r;
	res.z.x = -i;
	res.x.z = +i;
	res.z.z = +r;

	return res;

}

smol_m3_t smol_m3_rotate_z(float rotation_rad) {
	
	smol_m3_t res = { 0.f };

	float r = cosf(rotation_rad);
	float i = sinf(rotation_rad);

	res.x.y = +r;
	res.y.x = -i;
	res.x.y = +i;
	res.y.z = +r;

	return res;

}

smol_m3_t smol_m3_rotate_axis_angle(smol_v3_t axis, float rotation_rad) {

	smol_m3_t res = { 0.f };

	float r = cosf(rotation_rad);
	float i = sinf(rotation_rad);
	float t = 1.f - r;

	float axx = axis.x * axis.x;
	float ayy = axis.y * axis.y;
	float azz = axis.z * axis.z;

	float axy = axis.x * axis.y;
	float axz = axis.x * axis.z;

	float ayz = axis.y * axis.z;
	
	float ayi = axis.y * i;

	res.x.x = t*axx+r;
	res.x.y = t*axy+axis.z*i;
	res.x.z = t*axz+axis.y*i;

	res.y.x = t*axy+axis.z*i;
	res.y.y = t*ayy+r;
	res.y.z = t*ayz-axis.x*r;
	
	res.z.x = t*axz-axis.y*i;
	res.z.y = t*ayz+axis.x*i;
	res.z.z = t*azz+r; 

	return res;

}

smol_m3_t smol_m3_rotate_quat(smol_quat_t q) {

	smol_m3_t res = { 0.f };

	float qxx = q.x * q.x;
	float qyy = q.y * q.y;
	float qzz = q.z * q.z;
	float qww = q.w * q.w;

	float qxy = q.x * q.y;
	float qxz = q.x * q.z;
	float qxw = q.x * q.w;

	float qyz = q.y * q.z;
	float qyw = q.y * q.w;

	float qzw = q.z * q.w;

	res.x.x = 2.f * (qww + qxx) - 1.f;
	res.x.y = 2.f * (qxy - qzw) + 0.f;
	res.x.z = 2.f * (qxz + qyw) + 0.f;

	res.y.x = 2.f * (qxy + qzw) + 0.f;
	res.y.y = 2.f * (qww + qyy) - 1.f;
	res.y.z = 2.f * (qyz - qxw) + 0.f;

	res.z.x = 2.f * (qxz - qyw) + 0.f;
	res.z.y = 2.f * (qyz + qxw) + 0.f;
	res.z.z = 2.f * (qww + qzz) - 1.f;

	return res;

}


#pragma endregion

#pragma region smol_m4_t 


SMOL_INLINE smol_m4_t smol_m4_transpose(smol_m4_t m) {
	
	smol_m4_t res = {
		m.m[0], m.m[4],  m.m[8], m.m[12],
		m.m[1], m.m[5],  m.m[9], m.m[13],
		m.m[2], m.m[6], m.m[10], m.m[14],
		m.m[3], m.m[7], m.m[11], m.m[15]
	};

	return res;

}

SMOL_INLINE smol_m4_t smol_m4_mul(smol_m4_t a, smol_m4_t b) {

	smol_m4_t t = smol_m4_transpose(b);

	smol_m4_t res = {
		.x = { smol_v4_dot(a.x, t.x), smol_v4_dot(a.x, t.y), smol_v4_dot(a.x, t.z), smol_v4_dot(a.x, t.w) },
		.y = { smol_v4_dot(a.y, t.x), smol_v4_dot(a.y, t.y), smol_v4_dot(a.y, t.z), smol_v4_dot(a.y, t.w) },
		.z = { smol_v4_dot(a.z, t.x), smol_v4_dot(a.z, t.y), smol_v4_dot(a.z, t.z), smol_v4_dot(a.z, t.w) },
		.w = { smol_v4_dot(a.w, t.x), smol_v4_dot(a.w, t.y), smol_v4_dot(a.w, t.z), smol_v4_dot(a.w, t.w) }
	};

	return res;

}

SMOL_INLINE smol_m4_t smol_m4_mul_by_val(smol_m4_t a, float f) {

	smol_m4_t res = {
		.x = smol_v4_mul(f, a.x),
		.y = smol_v4_mul(f, a.y),
		.z = smol_v4_mul(f, a.z),
		.w = smol_v4_mul(f, a.w)
	};

	return res;

}


SMOL_INLINE smol_v4_t smol_m4_mul_v4(smol_m4_t m, smol_v4_t v) {

	smol_v4_t res = { 
		smol_v4_dot(m.x, v),
		smol_v4_dot(m.y, v),
		smol_v4_dot(m.z, v),
		smol_v4_dot(m.w, v)
	};

	return res;

}


SMOL_INLINE float smol_m4_determinant(smol_m4_t m) {

	return (
		m.x.x * (m.y.y * (m.z.z * m.w.w - m.z.w * m.w.z) - m.y.z * (m.z.y * m.w.w - m.z.w * m.w.y) + m.y.w * (m.z.y * m.w.z - m.z.z * m.w.y)) -
		m.x.y * (m.y.x * (m.z.z * m.w.w - m.z.w * m.w.z) - m.y.z * (m.z.x * m.w.w - m.z.w * m.w.x) + m.y.w * (m.z.x * m.w.z - m.z.z * m.w.x)) +
		m.x.z * (m.y.x * (m.z.y * m.w.w - m.z.w * m.w.y) - m.y.y * (m.z.x * m.w.w - m.z.w * m.w.x) + m.y.w * (m.z.x * m.w.y - m.z.y * m.w.x)) - 
		m.x.w * (m.y.x * (m.z.y * m.w.z - m.z.z * m.w.y) - m.y.y * (m.z.x * m.w.z - m.z.z * m.w.x) + m.y.z * (m.z.x * m.w.y - m.z.y * m.w.x))
	);

}

SMOL_INLINE smol_m4_t smol_m4_adjugate(smol_m4_t m) {

	m = smol_m4_transpose(m);

	smol_m4_t res = {
			.x = {
				 (m.y.y * m.z.z * m.w.w) + (m.y.z * m.z.w * m.w.y) + (m.y.w * m.z.y * m.w.z) - (m.y.w * m.z.z * m.w.y) - (m.y.z * m.z.y * m.w.w) - (m.y.y * m.z.w * m.w.z),
				-(m.x.y * m.z.z * m.w.w) - (m.x.z * m.z.w * m.w.y) - (m.x.w * m.z.y * m.w.z) + (m.x.w * m.z.z * m.w.y) + (m.x.z * m.z.y * m.w.w) + (m.x.y * m.z.w * m.w.z),
				 (m.x.y * m.y.z * m.w.w) + (m.x.z * m.y.w * m.w.y) + (m.x.w * m.y.y * m.w.z) - (m.x.w * m.y.z * m.w.y) - (m.x.z * m.y.y * m.w.w) - (m.x.y * m.y.w * m.w.z),
				-(m.x.y * m.y.z * m.z.w) - (m.x.z * m.y.w * m.z.y) - (m.x.w * m.y.y * m.z.z) + (m.x.w * m.y.z * m.z.y) + (m.x.z * m.y.y * m.z.w) + (m.x.y * m.y.w * m.z.z)
			}, 
			.y = {
				-(m.y.x * m.z.z * m.w.w) - (m.y.z * m.z.w * m.w.x) - (m.y.w * m.z.x * m.w.z) + (m.y.w * m.z.z * m.w.x) + (m.y.z * m.z.x * m.w.w) + (m.y.x * m.z.w * m.w.z),
				 (m.x.x * m.z.z * m.w.w) + (m.x.z * m.z.w * m.w.x) + (m.x.w * m.z.x * m.w.z) - (m.x.w * m.z.z * m.w.x) - (m.x.z * m.z.x * m.w.w) - (m.x.x * m.z.w * m.w.z),
				-(m.x.x * m.y.z * m.w.w) - (m.x.z * m.y.w * m.w.x) - (m.x.w * m.y.x * m.w.z) + (m.x.w * m.y.z * m.w.x) + (m.x.z * m.y.x * m.w.w) + (m.x.x * m.y.w * m.w.z),
				 (m.x.x * m.y.z * m.z.w) + (m.x.z * m.y.w * m.z.x) + (m.x.w * m.y.x * m.z.z) - (m.x.w * m.y.z * m.z.x) - (m.x.z * m.y.x * m.z.w) - (m.x.x * m.y.w * m.z.z)
			}, 
			.z = {
				 (m.y.x * m.z.y * m.w.w) + (m.y.y * m.z.w * m.w.x) + (m.y.w * m.z.x * m.w.y) - (m.y.w * m.z.y * m.w.x) - (m.y.y * m.z.x * m.w.w) - (m.y.x * m.z.w * m.w.y),
				-(m.x.x * m.z.y * m.w.w) - (m.x.y * m.z.w * m.w.x) - (m.x.w * m.z.x * m.w.y) + (m.x.w * m.z.y * m.w.x) + (m.x.y * m.z.x * m.w.w) + (m.x.x * m.z.w * m.w.y),
				 (m.x.x * m.y.y * m.w.w) + (m.x.y * m.y.w * m.w.x) + (m.x.w * m.y.x * m.w.y) - (m.x.w * m.y.y * m.w.x) - (m.x.y * m.y.x * m.w.w) - (m.x.x * m.y.w * m.w.y),
				-(m.x.x * m.y.y * m.z.w) - (m.x.y * m.y.w * m.z.x) - (m.x.w * m.y.x * m.z.y) + (m.x.w * m.y.y * m.z.x) + (m.x.y * m.y.x * m.z.w) + (m.x.x * m.y.w * m.z.y)
			}, 
			.w = {
				-(m.y.x * m.z.y * m.w.z) - (m.y.y * m.z.z * m.w.x) - (m.y.z * m.z.x * m.w.y) + (m.y.z * m.z.y * m.w.x) + (m.y.y * m.z.x * m.w.z) + (m.y.x * m.z.z * m.w.y),
				 (m.x.x * m.z.y * m.w.z) + (m.x.y * m.z.z * m.w.x) + (m.x.z * m.z.x * m.w.y) - (m.x.z * m.z.y * m.w.x) - (m.x.y * m.z.x * m.w.z) - (m.x.x * m.z.z * m.w.y),
				-(m.x.x * m.y.y * m.w.z) - (m.x.y * m.y.z * m.w.x) - (m.x.z * m.y.x * m.w.y) + (m.x.z * m.y.y * m.w.x) + (m.x.y * m.y.x * m.w.z) + (m.x.x * m.y.z * m.w.y),
				 (m.x.x * m.y.y * m.z.z) + (m.x.y * m.y.z * m.z.x) + (m.x.z * m.y.x * m.z.y) - (m.x.z * m.y.y * m.z.x) - (m.x.y * m.y.x * m.z.z) - (m.x.x * m.y.z * m.z.y)
			}
	};
	
	return res;

}

SMOL_INLINE smol_m4_t smol_m4_inverse(smol_m4_t m) {

	float det = smol_m4_determinant(m);

	
	if(fabsf(det) < 1e-4f) {
		fputs("Derterminat too small!", stderr);
	#ifdef _MSC_VER
		__debugbreak();
	#elif defined(_POSIX_C_SOURCE)
		sigint(SIGTRAP);
	#endif 
	}
	
	smol_m4_t adjugate = smol_m4_adjugate(m);
	smol_m4_t res = smol_m4_mul_by_val(adjugate, 1.0f / det);
	
	return res;

}

SMOL_INLINE smol_m4_t smol_m4_identity() {
	
	smol_m4_t res = {
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f,
	};
	
	return res;

}

SMOL_INLINE smol_m4_t smol_m4_translate_vec(smol_v3_t vec) {

	smol_m4_t res = smol_m4_identity();

	res.x.w = vec.x;
	res.y.w = vec.y;
	res.z.w = vec.z;

	return res;

}

SMOL_INLINE smol_m4_t smol_m4_translate_xyz(float x, float y, float z) {

	smol_m4_t res = smol_m4_identity();

	res.x.w = x;
	res.y.w = y;
	res.z.w = z;

	return res;

}

SMOL_INLINE smol_m4_t smol_m4_scale_vec(smol_v3_t vec) {
	
	smol_m4_t res = { 0.f };
	

	res.x.x = vec.x;
	res.y.y = vec.y;
	res.z.z = vec.z;
	res.w.w = 1.f;
	
	return res;

}


SMOL_INLINE smol_m4_t smol_m4_scale_xyz(float x, float y, float z) {
	
	smol_m4_t res = { 0.f };
	
	res.x.x = x;
	res.y.y = y;
	res.z.z = z;
	res.w.w = 1.f;
	
	return res;

}

SMOL_INLINE smol_m4_t smol_m4_uni_scale(float scale) {
	
	smol_m4_t res = { 0.f };
	
	res.x.x = scale;
	res.y.y = scale;
	res.z.z = scale;
	res.w.w = 1.f;
	
	return res;

}

SMOL_INLINE smol_m4_t smol_m4_rotate_x(float rotation_rad) {

	smol_m4_t res = smol_m4_identity();

	float r = cosf(rotation_rad);
	float i = sinf(rotation_rad);

	res.y.y = +r;
	res.z.y = -i;
	res.y.z = +i;
	res.z.z = +r;

	return res;

}

SMOL_INLINE smol_m4_t smol_m4_rotate_y(float rotation_rad) {

	smol_m4_t res = smol_m4_identity();

	float r = cosf(rotation_rad);
	float i = sinf(rotation_rad);

	res.x.x = +r;
	res.x.z = -i;
	res.z.x = +i;
	res.z.z = +r;

	return res;

}

SMOL_INLINE smol_m4_t smol_m4_rotate_z(float rotation_rad) {

	smol_m4_t res = smol_m4_identity();

	float r = cosf(rotation_rad);
	float i = sinf(rotation_rad);

	res.x.x = +r;
	res.x.y = -i;
	res.y.x = +i;
	res.y.y = +r;

	return res;

}

//https://www.euclideanspace.com/maths/geometry/rotations/conversions/angleToMatrix/index.htm
SMOL_INLINE smol_m4_t smol_m4_rotate_axis_angle(smol_v3_t axis, float rotation_rad) {

	smol_m4_t res = smol_m4_identity();

	float r = cosf(rotation_rad);
	float i = sinf(rotation_rad);
	float t = 1.f - r;

	float axx = axis.x * axis.x;
	float ayy = axis.y * axis.y;
	float azz = axis.z * axis.z;

	float axy = axis.x * axis.y;
	float axz = axis.x * axis.z;

	float ayz = axis.y * axis.z;
	
	float ayi = axis.y * i;

	res.x.x = t*axx+r;
	res.x.y = t*axy+axis.z*i;
	res.x.z = t*axz+axis.y*i;

	res.y.x = t*axy+axis.z*i;
	res.y.y = t*ayy+r;
	res.y.z = t*ayz-axis.x*r;
	
	res.z.x = t*axz-axis.y*i;
	res.z.y = t*ayz+axis.x*i;
	res.z.z = t*azz+r; 

	return res;

}

//https://automaticaddison.com/how-to-convert-a-quaternion-to-a-rotation-matrix/
SMOL_INLINE smol_m4_t smol_m4_rotate_quat(smol_quat_t q) {
	
	smol_m4_t res = smol_m4_identity();

	float qxx = q.x * q.x;
	float qyy = q.y * q.y;
	float qzz = q.z * q.z;
	float qww = q.w * q.w;

	float qxy = q.x * q.y;
	float qxz = q.x * q.z;
	float qxw = q.x * q.w;

	float qyz = q.y * q.z;
	float qyw = q.y * q.w;

	float qzw = q.z * q.w;

	res.x.x = 2.f * (qww + qxx) - 1.f;
	res.x.y = 2.f * (qxy - qzw) + 0.f;
	res.x.z = 2.f * (qxz + qyw) + 0.f;

	res.y.x = 2.f * (qxy + qzw) + 0.f;
	res.y.y = 2.f * (qww + qyy) - 1.f;
	res.y.z = 2.f * (qyz - qxw) + 0.f;

	res.z.x = 2.f * (qxz - qyw) + 0.f;
	res.z.y = 2.f * (qyz + qxw) + 0.f;
	res.z.z = 2.f * (qww + qzz) - 1.f;

	return smol_m4_transpose(res);

}

SMOL_INLINE smol_m4_t smol_m4_look_at(smol_v3_t view_location, smol_v3_t target_location, smol_v3_t up_vector) {

	//Forward vector
	smol_v3_t fw_vector = smol_v3_norm(smol_v3_sub(view_location, target_location));
	
	//Reorthogonalize up vector
	up_vector = smol_v3_norm(smol_v3_sub(up_vector, smol_v3_mul(smol_v3_dot(up_vector, fw_vector), fw_vector)));


	//Calculate right pointing vector
	smol_v3_t ri_vector = smol_v3_cross(up_vector, fw_vector);

	smol_m4_t rot = {
		 ri_vector.x,  ri_vector.y,  ri_vector.z, 0.f,
		 up_vector.x,  up_vector.y,  up_vector.z, 0.f,
		 fw_vector.x,  fw_vector.y,  fw_vector.z, 0.f,
		0.f, 0.f, 0.f, 1.f
	};

	//Calculate look at matrix
	smol_m4_t tra = { 
		1.f, 0.f, 0.f, -view_location.x,
		0.f, 1.f, 0.f, -view_location.y,
		0.f, 0.f, 1.f, -view_location.z,
		0.f, 0.f, 0.f, 1.f
	};

	smol_m4_t res = smol_m4_mul(rot, tra);

	return res;
}

//Projection:
SMOL_INLINE smol_m4_t smol_m4_frustum_lh(
	float left_plane, 
	float right_plane, 
	float top_plane, 
	float bottom_plane, 
	float near_plane, 
	float far_plane
) {

	smol_m4_t res = { 0.f };

	float diff_x = 1.f / (right_plane - left_plane);
	float diff_y = 1.f / (top_plane - bottom_plane);
	float diff_z = 1.f / (far_plane - near_plane);
	
	res.x.x = 2.f*near_plane * diff_x;
	res.y.y = 2.f*near_plane * diff_y;

	res.x.z = (right_plane + left_plane) * diff_x;
	res.y.z = (top_plane + bottom_plane) * diff_y;
	res.z.z = -(far_plane + near_plane) * diff_z;
	
	res.z.w = -(2.f * far_plane * near_plane) * diff_z;
	
	res.w.z = -1.f;
	res.w.w = 0.f;
	
	return res;
}

SMOL_INLINE smol_m4_t smol_m4_ortho_lh(
	float left_plane, 
	float right_plane, 
	float top_plane, 
	float bottom_plane, 
	float near_plane, 
	float far_plane
) {

	smol_m4_t res = { 0.f };

	float diff_x = right_plane - left_plane;
	float diff_y = top_plane - bottom_plane;
	float diff_z = far_plane - near_plane;

	res.x.x = 2.f / diff_x;
	res.x.w = -(right_plane + left_plane) / diff_x;
	
	res.y.y = 2.f / diff_y; 
	res.y.w = -(top_plane + bottom_plane) / diff_y;

	res.z.z = -2.f / diff_z;
	res.w.w = 1.f;

	return res;
	
}

SMOL_INLINE smol_m4_t smol_m4_perspective_lh(float fov, float aspect, float near_plane, float far_plane) {
	float ver = near_plane * tanf(fov * .5f);
	float hor = ver * aspect;
	return smol_m4_frustum_lh(-hor, hor, ver, -ver, near_plane, far_plane);
}

#pragma endregion


#ifdef SMOL_MATH_IMPLEMENTATION
#endif