/*
Copyright � 2023 Marko Ranta (Discord: coderunner)

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
#	include <sys/time.h>
#elif defined(__APPLE__)
#	define SMOL_PLATFORM_MAC_OS
//TODO:
#	error Mac OS backend not implemented yet!
#endif 

//smol_timer - Returns high precision system up time in seconds on
//             Windows, and high precision time since Unix epoch on Linux. 
//Returns: double - containing seconds.microseconds since the last start 
//                  of the computer (win32) or since unix epoch (on linux) 
double smol_timer(); 

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

#endif 

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

#endif 