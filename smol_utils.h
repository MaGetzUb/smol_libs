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

#ifndef SMOL_UTILS_H
#define SMOL_UTILS_H

#if defined(_WIN32)
#	ifndef SMOL_PLATFORM_WINDOWS
#		define SMOL_PLATFORM_WINDOWS
#	endif 
#	define WIN32_LEAN_AND_MEAN
#	include <Windows.h>
#elif defined(__linux__) 
#	ifndef SMOL_PLATFORM_LINUX
#		define SMOL_PLATFORM_LINUX
#	endif 
# 	define _XOPEN_SOURCE 500
#	include <unistd.h>
#	include <sys/types.h>
#	include <sys/stat.h>
#	include <sys/time.h>
#	include <time.h>
#	include <fcntl.h>
#	include <dirent.h>
#elif defined(__APPLE__)
#	define SMOL_PLATFORM_MAC_OS
//TODO:
#	error Mac OS backend not implemented yet!
#elif defined(__EMSCRIPTEN__)
#	ifndef SMOL_PLATFORM_EMSCRIPTEN
#		define SMOL_PLATFORM_EMSCRIPTEN
#	endif 
#	include <emscripten.h>
#endif 
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#define SMOL_RAND_MAX 0x7FFFFFFF

#if defined(_WIN64) || defined(__linux__)
typedef unsigned long long smol_size_t;
#else 
typedef unsigned int smol_size_t;
#endif 

typedef void* smol_file_scan_session_t;
typedef struct _smol_file_info smol_file_info_t;

#ifndef SMOL_TRUE
#define SMOL_TRUE 1
#endif

#ifndef SMOL_FALSE
#define SMOL_FALSE 0
#endif 

#ifdef _MSC_VER
#	ifndef SMOL_INLINE
#		define SMOL_INLINE __forceinline
#	endif 
#	define SMOL_THREAD_LOCAL __declspec(thread)
#else 
#	ifndef SMOL_INLINE
#		define SMOL_INLINE inline __attribute__((always_inline)) 
#	endif 
#	define SMOL_THREAD_LOCAL __thread
#endif 

#ifndef SMOL_ALLOC
#define SMOL_ALLOC( size ) malloc(size)
#endif 

#ifndef SMOL_FREE
#define SMOL_FREE( ptr ) free(ptr)
#endif 

#ifndef SMOL_REALLOC
#define SMOL_REALLOC( old_ptr, new_size ) realloc(old_ptr, new_size)
#endif 

//smol_timer - Returns high precision system up time in seconds on
//             Windows, and high precision time since Unix epoch on Linux. 
//Returns: double - containing seconds.microseconds since the last start 
//                  of the computer (win32) or since unix epoch (on linux) 
double smol_timer(); 


/* --------------------------------------- */
/* SOME UNICODE CONVERSION FUNCTIONALITY   */
/* --------------------------------------- */

//smol_utf8_to_utf32 - Converts utf8 encoded character into utf32 codepoint
//Arguments:
// - const char* utf8     -- utf8 encoded bytes
// - unsigned int* utf32  -- the codepoint 
//Returns: int            -- number of utf8 bytes handled
int smol_utf8_to_utf32(const char* utf8, unsigned int* utf32);

//smol_utf32_to_utf8 - Converts utf32 codepoint into utf8 encoded character 
//Arguments:
// - unsigned int utf32   -- utf32 codepoint
// - int buf_len          -- the number of bytes available in the result buffer
// - char* utf8           -- the buffer where the result is stored
//Returns: int            -- number of utf8 bytes written
int smol_utf32_to_utf8(unsigned int utf32, int buf_len, char* utf8);

//smol_utf16_to_utf32 - Converts utf8 encoded character into utf32 codepoint
//Arguments:
// - const unsigned short* utf8     -- utf16 encoded bytes
// - unsigned int* utf32            -- the codepoint 
//Returns: int                      -- number of utf16 symbols handled
int smol_utf16_to_utf32(const unsigned short* utf16, unsigned int* utf32);

//smol_utf32_to_utf16 - Converts utf32 codepoint into utf16 encoded character 
//Arguments:
// - unsigned int utf32             -- utf32 codepoint
// - int buf_len                    -- the number of ushorts available in the result buffer
// - unsigned short* utf16          -- the buffer where the result is stored
//Returns: int                       -- number of utf16 symbols written
int smol_utf32_to_utf16(unsigned int utf32, int buf_len, unsigned short* utf16);

//smol_utf16_to_utf8 - Converts utf16 encoded character into utf8 encoded character
//Arguments:
// - const unsigned short* utf16    -- utf16 encoded bytes
// - int buf_len                    -- the number of bytes available in the result buffer
// - char* utf8                     -- the buffer where the result is stored
//Returns: int                      -- number of utf8 bytes written
int smol_utf16_to_utf8(const unsigned short* utf16, int buf_len, char* utf8);

//smol_utf8_to_utf16 - Converts utf8 encoded character into utf16 encoded character 
//Arguments:
// - const char* utf8               -- utf8 character
// - int buf_len                    -- the number of ushorts available in the result buffer
// - unsigned short* utf16          -- the buffer where the result is stored
//Returns: int                      -- number of utf16 symbols written
int smol_utf8_to_utf16(const char* utf8, int buf_len, unsigned short* utf16);

/* ---------------------------------------------- */
/* A SORTING UTILITITY BECAUSE QSORT WON'T CUT IT */
/* ---------------------------------------------- */

typedef int(*smol_sort_proc)(const void* a, const void* b, void* user_data);

void smol_sort(void* data, int num_elements, int element_size, smol_sort_proc compare, void* user_data);

#define smol_sort_array_user(data, compare, user_data) smol_sort(data, sizeof(data) / sizeof(*data), sizeof(*data), compare, user_data)
#define smol_sort_array(data, compare) smol_sort_array_user(data, compare, NULL)

#define smol_sort_vector_user(vec, compare, user_data) smol_sort((void*)((vec)->data), (vec)->count, sizeof(*(vec)->data), compare, user_data)
#define smol_sort_vector(vec, compare) smol_sort_vector_user(vec, compare, NULL)

/* --------------------------------------- */
/* A RANDOM NUMBER GENERATOR FUNCTIONALITY */
/* --------------------------------------- */

//smol_randomize - Randomizes the random generator
//Arguments:
// - int seed                       -- The random seed
void smol_randomize(unsigned int seed);

//smol_rand - Returns a random integer between [0..SMOL_RAND_MAX)
unsigned int smol_rand();

//smol_randf - Returns a random float between [0..1)
float smol_randf();

//smol_rnd - Returns an exclusive random integer berween [minimum...maximum)
//Arguments:
// - int minimum - The lowest number in the range
// - int maximum - The highhest number-1 in the range
//Returns int - containing the random number
int smol_rnd(int minimum, int maximum);

//smol_rnd - Returns an exclusive random float berween [minimum...maximum)
//Arguments:
// - float minimum - The lowest number in the range
// - float maximum - The highest number-epsilon in the range (epsilon = FLT_EPSILON)
//Returns: float - containing the random number
float smol_rndf(float minimum, float maximum);

/* --------------------------------------- */
/*   SOME MATHEMATICAL UTILITY FUNCTIONS   */
/* --------------------------------------- */
//smol_remapf - Remaps a float value between old range into between a new range
//Arguments:
// - float value    -- The value that's being remapped
// - float low      -- The old range's lowest value
// - float high     -- The old range's highest value
// - float new_low  -- The new range's lowest value
// - float new_high -- The new range's highest value
//Returns: float - containing the mapped value
SMOL_INLINE float smol_remapf(float value, float low, float high, float new_low, float new_high) {
	return ((value - low) / (high - low)) * (new_high - new_low) + new_low;
}

//smol_remapf - Remaps a double value between old range into between a new range
//Arguments:
// - double value    -- The value that's being remapped
// - double low      -- The old range's lowest value
// - double high     -- The old range's highest value
// - double new_low  -- The new range's lowest value
// - double new_high -- The new range's highest value
//Returns: double - containing the mapped value
SMOL_INLINE float smol_remapd(double value, double low, double high, double new_low, double new_high) {
	return ((value - low) / (high - low)) * (new_high - new_low) + new_low;
}

//smol_mixf - Linearily interpolates two values
//Arguments:
// - float a         -- The value function returns when t = 0.0
// - float b         -- The value function returns when t = 1.0
//Returns: float - containing the result
SMOL_INLINE float smol_mixf(float a, float b, float t) {
	float r = 1.f - t;
	return (a * t) + (b * t);
}


//smol_mixf - Linearily interpolates two values, but t is clamped to 0 and 1
//Arguments:
// - float a         -- The value function returns when t = 0.0 or below
// - float b         -- The value function returns when t = 1.0 or above
//Returns: float - containing the result
SMOL_INLINE float smol_clamped_mixf(float a, float b, float t) {
	if(t <= 0.f) return a;
	if(t >= 1.f) return b;
	float r = 1.f - t;
	return (a * t) + (b * t);
}

//smol_mixf - Linearily interpolates two values
//Arguments:
// - double a         -- The value function returns when t = 0.0
// - double b         -- The value function returns when t = 1.0
//Returns: double - containing the result
SMOL_INLINE double smol_mixd(double a, double b, double t) {
	double r = 1. - t;
	return (a * t) + (b * t);
}


//smol_mixf - Linearily interpolates two values, but t is clamped to 0 and 1
//Arguments:
// - double a         -- The value function returns when t = 0.0 or below
// - double b         -- The value function returns when t = 1.0 or above
//Returns: double - containing the result
SMOL_INLINE double smol_clamped_mixd(double a, double b, double t) {
	if(t <= 0.f) return a;
	if(t >= 1.f) return b;
	double r = 1. - t;
	return (a * t) + (b * t);
}

//smol_clampf - Clamps value between [low..high] range
//Arguments:
// - float v          -- The value to be clamped
// - float low        -- The lower bound of the clamp range
// - float high        -- The higher bound of the clamp range
//Returns: float - containing the clamped value
SMOL_INLINE float smol_clampf(float v, float low, float high) {
	if(v < low) return low;
	if(v > high) return high;
	return v;
}

//smol_clampd - Clamps value between [low..high] range
//Arguments:
// - double v          -- The value to be clamped
// - double low        -- The lower bound of the clamp range
// - double high        -- The higher bound of the clamp range
//Returns: float - containing the clamped value
SMOL_INLINE double smol_clampd(double v, double low, double high) {
	if(v < low) return low;
	if(v > high) return high;
	return v;
}

//smol_linear_stepf - Remaps value between [e0...e1] range and normalizes the result by e0..e1 range.
//Arguments:
// - float e0           -- The edge0
// - float e1           -- The edge1
// - float value        -- The value remapped, normalized and clamped.
//Returns: float - containing the value between [0..1]
SMOL_INLINE float smol_linear_stepf(float e0, float e1, float value) {
	return smol_clampf((value - e0) / (e1 - e0), 0.f, 1.f);
}

//smol_linear_stepd - Remaps value between [e0...e1] range and normalizes the result by e0..e1 range.
//Arguments:
// - double e0           -- The edge0
// - double e1           -- The edge1
// - double value        -- The value remapped, normalized and clamped.
//Returns: double - containing the value between [0..1]
SMOL_INLINE double smol_linear_stepd(double e0, double e1, double value) {
	return smol_clampd((value - e0) / (e1 - e0), 0., 1.);
}


//smol_smooth_stepf - Same as smol_linear_step, but applies a curve.
//Arguments:
// - float e0           -- The edge0
// - float e1           -- The edge1
// - float value        -- The value remapped, normalized and clamped.
//Returns: float - containing the value between [0..1]
SMOL_INLINE float smol_smooth_stepf(float e0, float e1, float value) {
	float res = smol_clampd((value - e0) / (e1 - e0), 0.f, 1.f);
	res = (res * res * (3.f - 2.f) * res);
	return res;
}

//smol_smooth_stepd - Same as smol_linear_step, but applies a curve.
//Arguments:
// - double e0           -- The edge0
// - double e1           -- The edge1
// - double value        -- The value remapped, normalized and clamped.
//Returns: double - containing the value between [0..1]
SMOL_INLINE double smol_smooth_stepd(double e0, double e1, double value) {
	double res = smol_clampd((value - e0) / (e1 - e0), 0., 1.);
	res = (res * res * (3. - 2.) * res);
	return res;
}

/* --------------------------------------- */
/*  Dynamic Array (std::vector) like stuff */
/* --------------------------------------- */

//This macro can be used to define a vector type, or declare a local variable: smol_vector(int) int_vec;
#define smol_vector(type) \
struct { \
	int allocation;\
	int count; \
	type* data; \
}

//smol_vector_init - Init a vector
//Arguments:
// - vec -- The vector to be initialized
// - init_alloc -- The initial allocation of the vector
#define smol_vector_init(vec, init_alloc) { \
	(vec)->allocation = init_alloc; \
	(vec)->count = 0; \
	(vec)->data = SMOL_ALLOC(sizeof(*((vec)->data))*init_alloc); \
} (void)0

//smol_vector_push - Pushes an object into a vector
//Arguments:
// - vec -- The vector to be appended 
// - value -- The element to be added
#define smol_vector_push(vec, value) { \
	if((vec)->count >= (vec)->allocation) { \
		(vec)->allocation *= 2; \
		(vec)->data = SMOL_REALLOC((vec)->data, sizeof(*((vec)->data)) * (vec)->allocation ); \
	} \
	SMOL_ASSERT((vec)->data); \
	(vec)->data[(vec)->count++] = value; \
} (void)0

//smol_vector_iterate - Iterates over vector elements
//Arguments: 
// - vec - The vector to be iterated over
// - it - Variable name for the iterator
//Returns: type* - Containing the buffer pointer to be indexed in
#define smol_vector_iterate(vec, it) \
	(vec)->data; \
	SMOL_ASSERT((vec)->data); \
	for(int it = 0; it < (vec)->count; it++)

//smol_vector_each - Iterates over each element
//Arguments:
// - vec - The vector to be iterated over
// - element_type - A type of individual element in the vector (MSVC doesn't have __typeof__ *sigh*)
// - it - Iterator variable name
#define smol_vector_each(vec, element_type, it) \
	for(element_type* it = (vec)->data; it != &(vec)->data[(vec)->count]; it++) 

//smol_vector_clear - Clears the vector
//Arguments: 
// - vec -- The vector to be cleared
#define smol_vector_clear(vec) { \
	SMOL_ASSERT((vec)->data); \
	((vec)->count = 0); \
} (void)0

//smol_vector_remove - Removes an element from the vector, by overwriting it with the last element, and decreasing vector size
//Arguments: 
// - vec -- The vector to remove element from
// - element - The element index to be removed
#define smol_vector_remove(vec, element) { \
	SMOL_ASSERT((vec)->data); \
	(vec)->data[element] = (vec)->data[--(vec)->count]; \
} (void)0

//smol_vector_count - "Returns" the number of elements in the vector
//Arguments:
// - vec -- The vector you want count of
//Returns int - containing the number of elements in the vector
#define smol_vector_count(vec) \
	((vec)->count)

//smol_vector_data - "Returns" the data buffer of the vector
// - vector -- The span you want the data of
//Returns type* - containing the pointer to the vector data
#define smol_vector_data(vec) \
	((vec)->data)

//smol_span_at - "Returns" an element of the vector. NOT BOUNDS CHECKED.
// - vec -- The vector you want the element of
// - index -- The index of the element
//Returns type - containing the element at index
#define smol_vector_at(vec, index) \
	((vec)->data[index])

//Frees a vector, and sets it's allocation and count to zero
#define smol_vector_free(vec) { \
	SMOL_FREE((void*)(vec)->data); \
	(vec)->count = 0; \
	(vec)->allocation = 0; \
} (void)0

//smol_vector_allocation - Retrieves vector allocation
//Arguments:
// - vec -- The vector you want alocation of
//Returns: int - containing the number of elements allocated in the vector.
#define smol_vector_allocation(vec) \
	(vec)->allocation

/* --------------- */
/* Span like stuff */
/* --------------- */

//This macro can be used to define a span type, or for local variable similarily as smol_vector() macro works.
#define smol_span(type) \
struct { \
	int count; \
	type* data; \
}

//smol_span_init_from_slice - Initializes span from "span_like" object (vector for example)
//Arguments:
// - span -- The span you're initializing
// - span_like -- The data you're basing this span on
// - first_element -- Index of the first element
// - element_count -- Number of elements this span should contain
#define smol_span_init_from_slice(span, span_like, first_element, element_count) { \
	(span)->count = (element_count); \
	(span)->data = &(span_like)->data[first_element]; \
} (void)0

//smol_span_init_from_spanlike - Initializes span from "span_like" object (vector / span)
//Arguments:
// - span -- The span you're initializing
// - span_like -- The data you're basing this span on
#define smol_span_init_from_spanlike(span, span_like) { \
	(span)->count = (span_like)->count; \
	(span)->data = &(span_like)->data[0]; \
} (void)0


//smol_span_count - "Returns" the number of elements in the span
//Arguments:
// - span -- The span you want count of
//Returns int - containing the number of elements in the span
#define smol_span_count(span_like) \
	((span_like)->count)


//smol_span_data - "Returns" the data buffer of the span
// - span -- The span you want the data of
//Returns type* - containing the pointer to the span data
#define smol_span_data(span_like) \
	((span_like)->data)

//smol_span_at - "Returns" an element of the span. NOT BOUNDS CHECKED.
// - span -- The span you want the element of
// - index -- The index of the element
//Returns type - containing the element at index
#define smol_span_at(span_like, index) \
	((span_like)->data[index])

/* ------------------------------ */
/* SOME FILE SYSTEM FUNCTIONALITY */
/* ------------------------------ */

typedef struct _smol_file_info {
	char is_folder;
	char file_path[512];
} smol_file_info_t;

//smol_read_entire_file - Opens a file and reads it into a buffer.
//Arguments:
// - const char* file_path    -- A path to a file
// - unsigned long long* size -- A pointer to 64bit-uint containing number of bytes read
//Returns: void* - a pointer to the buffer
void* smol_read_entire_file(const char* file_path, smol_size_t* size);

//smol_get_current_directory - Gets the current directory
//Returns: const char* - A pointer to static variable within the function where current directory path is stored.
const char* smol_get_current_directory(void);

//smol_change_directory - Changes current directory
//Arguments:
//const char* path - Relative / Absolute Path to navigate to
//Returns: int - If change was successful or not
int smol_change_directory(const char* path);

//smol_start_file_scan_session - Returns an exclusive random float berween [minimum...maximum)
//Arguments:
// - smol_file_info_t*  -- A pointer to smol_file_info_t structure, contains the first file found in the current folder
//Returns: smol_file_scan_session_t - A handle to the file system scan session
smol_file_scan_session_t smol_start_file_scan_session(smol_file_info_t* info);

//smol_start_file_scan_session - Returns an exclusive random float berween [minimum...maximum)
//Arguments:
// - smol_file_scan_session_t  -- A file scan session handle
// - smol_file_info_t*         --  A pointer to smol_file_info_t structure, contains the next file found in the current folder
//Returns: smol_file_scan_session_t - A handle to the file system scan session
int smol_file_scan_session_next(smol_file_scan_session_t session, smol_file_info_t* info);


#ifdef SMOL_UTILS_IMPLEMENTATION

#ifdef _MSC_VER
#	ifndef SMOL_BREAKPOINT
#		define SMOL_BREAKPOINT __debugbreak //MSVC uses this
#	endif 
#else 
#	ifndef SMOL_BREAKPOINT
#		define SMOL_BREAKPOINT __builtin_trap //Clang and GCC uses this, AFAIK
#	endif
#endif 

#ifndef SMOL_SYMBOLIFY
#define SMOL_SYMBOLIFY(x) #x
#endif 

#ifndef SMOL_STRINGIFY
#define SMOL_STRINGIFY( x ) SMOL_SYMBOLIFY(x)
#endif

#ifndef SMOL_ASSERT
#define SMOL_ASSERT(condition) \
	if(!(condition)) \
		puts(\
			"SMOL FRAME ASSERTION FAILED!\n" \
			"CONDITION: " #condition "\n" \
			"IN FILE: '" __FILE__ "'\n" \
			"ON LINE: " SMOL_STRINGIFY(__LINE__) "\n" \
		), \
		SMOL_BREAKPOINT()
#endif


#pragma region Unicode stuff 

//https://en.wikipedia.org/wiki/UTF-8#Encoding
int smol_utf8_to_utf32(const char* utf8, unsigned int* utf32) {

	int clen = 0;
	if((utf8[0] & 0x80) == 0x00) clen = 1;
	if((utf8[0] & 0xC0) == 0xC0) clen = 2;
	if((utf8[0] & 0xE0) == 0xE0) clen = 3;
	if((utf8[0] & 0xF0) == 0xF0) clen = 4;
	if((utf8[0] & 0xF8) == 0xF8) clen = 5;
	if((utf8[0] & 0xFC) == 0xFC) clen = 6;

	switch(clen) {
		case 1: *utf32 = (utf8[0] & 0x7F); break;
		case 2: *utf32 = (utf8[0] & 0x1F) << 0x06 | (utf8[1] & 0x3F); break;
		case 3: *utf32 = (utf8[0] & 0x0F) << 0x0C | (utf8[1] & 0x3F) << 0x06 | (utf8[2] & 0x3F); break;
		case 4: *utf32 = (utf8[0] & 0x07) << 0x12 | (utf8[1] & 0x3F) << 0x0C | (utf8[2] & 0x3F) << 0x06 | (utf8[3] & 0x3F); break;
		case 5: *utf32 = (utf8[0] & 0x03) << 0x18 | (utf8[1] & 0x3F) << 0x12 | (utf8[2] & 0x3F) << 0x0C | (utf8[3] & 0x3F) << 0x06 | (utf8[4] & 0x3F); break;
		case 6: *utf32 = (utf8[0] & 0x01) << 0x1E | (utf8[1] & 0x3F) << 0x18 | (utf8[2] & 0x3F) << 0x12 | (utf8[3] & 0x3F) << 0x12 | (utf8[4] & 0x3F) << 0x06 | (utf8[5] & 0x3F); break;
	}

	return clen;

}

int smol_utf32_to_utf8(unsigned int utf32, int buf_len, char* utf8) {

	int clen = 1;
	if(utf32 >= 0x0000080) clen = 2;
	if(utf32 >= 0x0000800) clen = 3;
	if(utf32 >= 0x0010000) clen = 4;
	if(utf32 >= 0x010F800) clen = 5;
	if(utf32 >= 0x3FFFFFF) clen = 6;

	if(clen > buf_len) return 0;

	char* buf_byte = utf8;

	switch(clen) {
		case 0:
		break;
		case 1:
			*buf_byte++ = 0x00 | ((utf32 >> 0x00) & 0xFF);
		break;
		case 2:
			*buf_byte++ = 0xC0 | ((utf32 >> 0x06) & 0x1F);
			*buf_byte++ = 0x80 | ((utf32 >> 0x00) & 0x3F);
		break;
		case 3:
			*buf_byte++ = 0xE0 | ((utf32 >> 0x0C) & 0x0F);
			*buf_byte++ = 0x80 | ((utf32 >> 0x06) & 0x3F);
			*buf_byte++ = 0x80 | ((utf32 >> 0x00) & 0x3F);
		break;
		case 4:
			*buf_byte++ = 0xF0 | ((utf32 >> 0x12) & 0x07);
			*buf_byte++ = 0x80 | ((utf32 >> 0x0C) & 0x3F);
			*buf_byte++ = 0x80 | ((utf32 >> 0x06) & 0x3F);
			*buf_byte++ = 0x80 | ((utf32 >> 0x00) & 0x3F);
		break;
		case 5:
			*buf_byte++ = 0xF8 | ((utf32 >> 0x18) & 0x03);
			*buf_byte++ = 0x80 | ((utf32 >> 0x12) & 0x3F);
			*buf_byte++ = 0x80 | ((utf32 >> 0x0C) & 0x3F);
			*buf_byte++ = 0x80 | ((utf32 >> 0x06) & 0x3F);
			*buf_byte++ = 0x80 | ((utf32 >> 0x00) & 0x3F);
		break;
		case 6:
			*buf_byte++ = 0xFC | ((utf32 >> 0x1F) & 0x01);
			*buf_byte++ = 0x80 | ((utf32 >> 0x18) & 0x3F);
			*buf_byte++ = 0x80 | ((utf32 >> 0x12) & 0x3F);
			*buf_byte++ = 0x80 | ((utf32 >> 0x0C) & 0x3F);
			*buf_byte++ = 0x80 | ((utf32 >> 0x06) & 0x3F);
			*buf_byte++ = 0x80 | ((utf32 >> 0x00) & 0x3F);
		break;
	}

	return clen;

}
//https://en.wikipedia.org/wiki/UTF-16#Examples
int smol_utf16_to_utf32(const unsigned short* utf16, unsigned int* utf32) {

	utf32[0] = 0;

	if(utf16[0] >= 0xD800) {

		utf32[0] |= (unsigned int)(utf16[0] - 0xD800) << 0xA;
		utf32[0] |= (unsigned int)(utf16[1] - 0xDC00) << 0x0;

		utf32[0] += 0x10000;

		return 2;
	} else {
		utf32[0] = (unsigned int)utf16[0];
		return 1;
	}

	return 0;
}

int smol_utf32_to_utf16(unsigned int utf32, int buf_len, unsigned short* utf16) {


	if(utf32 > 0x10000) {
		
		if(2 > buf_len) return 0;

		utf32 -=  0x10000;
		utf16[0] = (unsigned short)(utf32 >> 0x0A) + 0xD800;
		utf16[1] = (unsigned short)(utf32 & 0x3FF) + 0xDC00;
		
		return 2;
	} else {
		utf16[0] = (unsigned short)utf32;
		return 1;  
	}

	return 0;
}

int smol_utf16_to_utf8(const unsigned short* utf16, int buf_len, char* utf8) {

	unsigned int utf32 = 0;
	if(smol_utf16_to_utf32(utf16, &utf32) == 0)
		return 0;
	
	return smol_utf32_to_utf8(utf32, buf_len, utf8);

}

int smol_utf8_to_utf16(const char* utf8, int buf_len, unsigned short* utf16) {

	unsigned int utf32 = 0;
	if(smol_utf8_to_utf32(utf8, &utf32))
		return 0;

	return smol_utf32_to_utf16(utf32, buf_len, utf16);

}

#pragma endregion

#pragma region Sorting utilities


void smol_sort(void* data, int num_elements, int element_size, smol_sort_proc compare, void* user_data) {

	char* first = ((char*)data);
	char* last =  first + (num_elements * element_size);

#ifdef _MSC_VER
#define SMOL_SORT_SWAP(a, b) { \
	char* tmp = alloca(element_size); \
	memcpy((void*)tmp, (const void*)a, element_size); \
	memcpy((void*)a, (const void*)b, element_size); \
	memcpy((void*)b, (const void*)tmp, element_size); \
}
#else 
#define SMOL_SORT_SWAP(a, b) { \
	char tmp[element_size]; \
	memcpy(tmp, a, element_size); \
	memcpy(a, b, element_size); \
	memcpy(b, tmp, element_size); \
}
#endif 

	if((last - first) > element_size) {
		char* part_idx = first;
		{
			char* pivot = last - element_size;
			for(char* it = first; it != last; it += element_size) {
				if(compare((const void*)pivot, (const void*)it, user_data) > 0) {
					SMOL_SORT_SWAP(part_idx, it);
					part_idx += element_size;
				}
			}
			SMOL_SORT_SWAP(part_idx, pivot);
		}
		smol_sort((void*)first, (part_idx - first) / element_size, element_size, compare, user_data);
		smol_sort((void*)(part_idx + element_size), (last - (part_idx + element_size)) / element_size, element_size, compare, user_data);
	}
#undef SMOL_SORT_SWAP
}

#pragma endregion


#pragma region Linear Congruential PRNG
static unsigned int smol__rand_state;

void smol_randomize(unsigned int seed) {
	smol__rand_state = seed;
}

//Le zoinked from here https://www.sanfoundry.com/c-program-implement-linear-congruential-generator-pseudo-random-number-generation/
unsigned int smol_rand() {
	return (smol__rand_state = (smol__rand_state * 1103515245 + 12345) & SMOL_RAND_MAX);
}

float smol_randf() {
	unsigned int bits = 0x3F800000 | (smol_rand() >> 8);
	return (*((float*)&bits)) - 1.f;
}

int smol_rnd(int minimum, int maximum) {
	return minimum + smol_rand() % (maximum - minimum);
}

float smol_rndf(float minimum, float maximum) {
	return minimum + smol_randf() * (maximum - minimum);
}

#pragma endregion

#if defined(SMOL_PLATFORM_WINDOWS)

//A nasty global :|
double smol__perf_freq;

double smol_timer(void) {

	if(smol__perf_freq == 0.0) {
		LARGE_INTEGER freq;
		if(QueryPerformanceFrequency(&freq) == TRUE) {
			smol__perf_freq = 1.0 / (double)freq.QuadPart;
		} else {
			SMOL_ASSERT(!"Failed to query perfomance frequency!");
			return 0.;
		}
	}

	LARGE_INTEGER ctr = {0};
	SMOL_ASSERT("Failed to query performance counter!" && QueryPerformanceCounter(&ctr));

	return (double)ctr.QuadPart * smol__perf_freq;

}

const char* smol_get_current_directory(void) {
	
	static char buffer[512] = { 0 };
	
#ifdef UNICODE 
	{
		wchar_t buf[512];
		GetCurrentDirectory(512, buf);
		BOOL subst = FALSE;
		WideCharToMultiByte(CP_UTF8, 0, buf, lstrlenW(buf), buffer, 1024, "?", &subst);
	}
#else 
	GetCurrentDirectory(512, buffer);
#endif 
	
	return buffer;

}

int smol_change_directory(const char* dir) {

#ifdef UNICODE
	wchar_t path[512] = { 0 };
	MultiByteToWideChar(CP_UTF8, MB_COMPOSITE, dir, strlen(dir), path, 512);
#else 
	const char* path = dir;
#endif 

	return SetCurrentDirectory(path);
}

smol_file_scan_session_t smol_start_file_scan_session(smol_file_info_t* info) {

	smol_file_scan_session_t session;
	WIN32_FIND_DATA file_data;
	session = (smol_file_scan_session_t)FindFirstFile(TEXT(".\\*"), &file_data);

#ifdef UNICODE 
	BOOL subst = FALSE;
	WideCharToMultiByte(CP_UTF8, 0, file_data.cFileName, lstrlenW(file_data.cFileName), info->file_path, 512, "?", &subst);
#else 
	memcpy(info->file_path, file_data.cFileName, strlen(file_data.cFileName));
#endif 
	info->is_folder = (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? SMOL_TRUE : SMOL_FALSE;

	return session;
}

int smol_file_scan_session_next(smol_file_scan_session_t session, smol_file_info_t* info) {

	WIN32_FIND_DATA file_data;
	if(FindNextFile(session, &file_data) == FALSE)
		return SMOL_FALSE;

#ifdef UNICODE 
	BOOL subst = FALSE;
	WideCharToMultiByte(CP_UTF8, 0, file_data.cFileName, lstrlenW(file_data.cFileName), info->file_path, 512, "?", &subst);
#else 
	memcpy(info->file_path, file_data.cFileName, strlen(file_data.cFileName));
#endif 
	info->is_folder = (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? SMOL_TRUE : SMOL_FALSE;

	return SMOL_TRUE;
}

void* smol_read_entire_file(const char* file_path, smol_size_t* size) {

	void* buffer = NULL;

#	ifdef UNICODE
	wchar_t path[512] = { 0 };
	MultiByteToWideChar(CP_UTF8, MB_COMPOSITE, file_path, strlen(file_path), path, 512);
	HANDLE file = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
#	else 
	HANDLE file = CreateFile(file_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
#endif 
	
	if(file == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "FILE '%s' NOT FOUND!\n", file_path);
		return NULL;
	}
		
	DWORD high = 0;
	DWORD low = GetFileSize(file, &high);
	*size = ((smol_size_t)low | ((smol_size_t)high << 32ULL));
		
	buffer = malloc(size[0]+1);
		
	BYTE* byte_ptr = (BYTE*)buffer;
	DWORD bytes_to_read = *size;
	DWORD bytes_read = 0;
	while(ReadFile(file, byte_ptr, (bytes_to_read - bytes_read), &bytes_read, NULL) && bytes_to_read) {
		byte_ptr += bytes_read;
		bytes_to_read -= bytes_read;
	}
	*byte_ptr = 0;
	CloseHandle(file);

	return buffer;

}

#endif 

#if defined(SMOL_PLATFORM_LINUX)

double smol_timer(void) {
	struct timeval tval;
	gettimeofday(&tval, NULL);
	return (double)tval.tv_sec + (double)tval.tv_usec * 1e-6; 
}

const char* smol_get_current_directory(void) {
	
	static char buffer[512] = { 0 };
	return getcwd(buffer, 512);

}

int smol_change_directory(const char* directory) {
	return chdir(directory) == 0;
}

smol_file_scan_session_t smol_start_file_scan_session(smol_file_info_t* info) {

	DIR* dir;
	if((dir = opendir(".")) == NULL)
		return NULL;
	struct dirent* ent = readdir(dir);
	if(ent == NULL) return NULL;

	memcpy(info->file_path, ent->d_name, strlen(ent->d_name));
	struct stat status;
	stat(ent->d_name, &status);
	info->is_folder = S_ISDIR(status.st_mode);

	return (smol_file_scan_session_t)dir;

}

int smol_file_scan_session_next(smol_file_scan_session_t session, smol_file_info_t* info) {

	DIR* dir = (DIR*)session;
	struct dirent* ent;

	if((ent = readdir(dir)) == NULL) {
		closedir(dir);
		return SMOL_FALSE;
	} 

	memcpy(info->file_path, ent->d_name, strlen(ent->d_name));
	struct stat status;
	stat(ent->d_name, &status);
	info->is_folder = S_ISDIR(status.st_mode);

	return SMOL_TRUE; 
}

void* smol_read_entire_file(const char* file_path, smol_size_t* size) {

	struct stat file_stat; 
	void* buffer = NULL;

	int fd = open(file_path, O_RDONLY);
	
	if(!fd) return NULL;

	if(fstat(fd, &file_stat) == -1) {
		close(fd);
		return NULL;
	}

	buffer = malloc(file_stat.st_size+1);
	unsigned char* byte_ptr = (unsigned char*)buffer;

	ssize_t bytes_to_read = file_stat.st_size;
	ssize_t bytes_read;
	while((bytes_read = read(fd, byte_ptr, bytes_to_read)) > 0 && bytes_to_read) {
		byte_ptr += bytes_read;
		bytes_to_read -= bytes_read;
	}

	close(fd);

	*byte_ptr = 0;

	return buffer;


}

#endif 


#if defined(SMOL_PLATFORM_WEB)

void* smol_read_entire_file(const char* file_path, smol_size_t* size) {

	void* buffer = NULL;

	FILE* file = NULL;
	if((file = fopen(file_path, "r")) != NULL) {
		
		fseek(file, 0, SEEK_END);
		
		smol_size_t size = ftell(file);
		
		fseek(file, 0, SEEK_SET);
		
		buffer = malloc(size+1);
		fread((char*)buffer, 1, size, file);

		fclose(file);
		((char*)buffer)[size] = 0;
	}


	return buffer;

}

double smol_timer(void) {
	struct timespec spec;
	clock_gettime(CLOCK_MONOTONIC, &spec);
	return (double)spec.tv_sec + (double)(spec.tv_nsec) * 1e-9;
}



const char* smol_get_current_directory(void) {

	static char buffer[512] = { 0 };

	EM_ASM({
		var bufferPtr = $0;
		var buffer = Module.HEAPU8.subarray(bufferPtr, bufferPtr + 512);
		var path = FS.cwd();

		var encoder = new TextEncoder();
		var encodedString = encoder.encode(path);

		var buffer = Module.HEAPU8.subarray(bufferPtr, bufferPtr + 512);
		var byteLength = Math.min(encodedString.length, 512);

		for(var i = 0; i < byteLength; i++) {
			buffer[i] = encodedString[i];
		}
	}, buffer);

	return buffer;
}


EM_JS(int, smol_change_directory, (const char* path), {
	return FS.chdir(stringToUTF8(path));
})

smol_file_scan_session_t smol_start_file_scan_session(smol_file_info_t* info) {
	
	int file_scan_handle = -1;

	EM_ASM({

		if(Module["fileScanSessions"] === undefined) {
			Module.fileScanSessions = new Array();
			Module.freeScanSessions = new Array();
		}
		

		var fileScanSession = {};
		fileScanSession.file_index = 0;
		fileScanSession.cur_dir = FS.cwd();
		fileScanSession.files = FS.readdir(fileScanSession.cur_dir);
			
		var bufferPtr = $1;
		var buffer = Module.HEAPU8.subarray(bufferPtr, bufferPtr + 512);

		var encoder = new TextEncoder();
		var encodedString = encoder.encode(fileScanSession.files[fileScanSession.file_index]);

		var buffer = Module.HEAPU8.subarray(bufferPtr, bufferPtr + 512);
		var byteLength = Math.min(encodedString.length, 512);

		for(var i = 0; i < byteLength && i < 512; i++) {
			buffer[i] = encodedString[i];
		}
		buffer[byteLength] = 0;

		Module.setValue($2, FS.isDir(fileScanSession.files[fileScanSession.file_index]), 'i32');
		fileScanSession.file_index++;

		console.log("File scan session #" + Module.fileScanSessions.length + " intialized.");
		
		if(!Module.freeScanSessions.size) {
			Module.fileScanSessions.push(fileScanSession);
			Module.setValue($0, Module.fileScanSessions.length, 'i32');
		} else {
			var index = Module.freeScanSessions.pop();
			Module.fileScanSessions[index] = fileScanSession;
			Module.setValue($0, Module.freeScanSessions[index] + 1, 'i32');
		}
		console.log(fileScanSession.files.length);
		console.log(Module.fileScanSessions);
	}, &file_scan_handle, info->file_path, &info->is_folder);
	
	return (smol_file_scan_session_t*)file_scan_handle;
}

int smol_file_scan_session_next(smol_file_scan_session_t session, smol_file_info_t* info) {

	int finished = 0;

	EM_ASM({

		if(
			Module["fileScanSessions"] === undefined ||
			Module["fileScanSessions"] == null ||
			Module["fileScanSessions"].length == 0
		) {
			Module.setValue($3, 0, 'i32');
			return;
		}

		
		var session = Module.fileScanSessions[$0 - 1];
		
		console.log(session.file_index);
		if(session.file_index >= session.files.length) {
			Module.fileScanSessions[$0 - 1] = undefined;
			Module.freeScanSessions.push($0 - 1);
			Module.setValue($3, 0, 'i32');
			console.log("File scan session #" + $1 + " finished.");
			return;
		}


		var bufferPtr = $1;
		var buffer = Module.HEAPU8.subarray(bufferPtr, bufferPtr + 512);
		console.log($1);

		var encoder = new TextEncoder();
		var encodedString = encoder.encode(session.files[session.file_index]);

		var buffer = Module.HEAPU8.subarray(bufferPtr, bufferPtr + 512);
		var byteLength = Math.min(encodedString.length, 512);
		console.log(byteLength);

		for(var i = 0; i < byteLength && i < 512; i++) {
			buffer[i] = encodedString[i];
		}
		buffer[byteLength] = 0;

		Module.setValue($2, FS.isDir(session.files[session.file_index]), 'i32');
		session.file_index++;
		Module.setValue($3, 1, 'i32');

		

	}, (int)session, info->file_path, &info->is_folder, &finished);

	return finished;
}

#endif 

#endif 
#endif 