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

#if _WIN64
typedef unsigned long long smol_size_t;
#else 
typedef unsigned int smol_size_t;
#endif 

#ifdef _MSC_VER
#define SMOL_THREAD_LOCAL __declspec(thread)
#	ifdef SMOL_INLINE
#		define SMOL_INLINE __forceinline
#	endif 
#else 
#	ifndef SMOL_INLINE
#		define SMOL_INLINE inline __attribute__((always_inline)) 
#	endif 
#define SMOL_THREAD_LOCAL __thread
#endif 
//smol_timer - Returns high precision system up time in seconds on
//             Windows, and high precision time since Unix epoch on Linux. 
//Returns: double - containing seconds.microseconds since the last start 
//                  of the computer (win32) or since unix epoch (on linux) 
double smol_timer(); 

//smol_read_entire_file - Opens a file and reads it into a buffer.
//Arguments:
// - const char* file_path    -- A path to a file
// - unsigned long long* size -- A pointer to 64bit-uint containing number of bytes read
//Returns: void* - a pointer to the buffer
void* smol_read_entire_file(const char* file_path, smol_size_t* size);

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
float smol_rndf(float minimum, float maximum);

//float SMOL_INLINE smol_map(float value, float low, float high, float new_low, float new_high) {
//	return ((value - low) / (high - low)) * (new_high - new_low) + new_low;
//}

#endif 

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


#elif defined(SMOL_PLATFORM_LINUX)

double smol_timer(void) {
	struct timeval tval;
	gettimeofday(&tval, NULL);
	return (double)tval.tv_sec + (double)tval.tv_usec * 1e-6; 
}

#elif defined(SMOL_PLATFORM_EMSCRIPTEN)

double smol_timer(void) {
	struct timespec spec;
	clock_gettime(CLOCK_MONOTONIC, &spec);
	return (double)spec.tv_sec + (double)(spec.tv_nsec) * 1e-9;
}

#endif 

void* smol_read_entire_file(const char* file_path, smol_size_t* size) {

	void* buffer = NULL;

#	ifdef _WIN32

#	ifdef UNICODE
	wchar_t path[512] = { 0 };
	MultiByteToWideChar(CP_UTF8, MB_COMPOSITE, file_path, strlen(file_path), path, 512);
	HANDLE file = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
#	else 
	HANDLE file = CreateFile(file_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
#endif 
	
	if(file == NULL) return NULL;
		
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

#endif 

#ifdef __linux__

	struct stat file_stat; 

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
#endif 

#ifdef __EMSCRIPTEN__
	FILE* file = NULL;
	if((file = fopen(file_path, "r")) != NULL) {
		
		fseek(file, 0, SEEK_END);
		
		smol_size_t size = ftell(file);
		
		fseek(file, 0, SEEK_SET);
		
		buffer = malloc(size);
		
		fread((char*)buffer, 1, size, file);

		fclose(file);
	
	}


#endif 

	return buffer;

}


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

unsigned int smol__rand_state;

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

#endif 