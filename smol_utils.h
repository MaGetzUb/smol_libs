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

//smol_timer - Returns basically system up time on Windows, time since epoch on Linux. 
double smol_timer(); 

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

#ifndef SMOL_ASSERT
#define SMOL_ASSERT(condition) \
	if(!(condition)) \
		printf(\
			"SMOL FRAME ASSERTION FAILED!\n" \
			#condition "\n" \
			"IN FILE '" __FILE__ "'\n" \
			"ON LINE %d", __LINE__ \
		), \
		SMOL_BREAKPOINT()
#endif 

#if defined(SMOL_PLATFORM_WINDOWS)

//A nasty global :|
double smol__perf_freq;

double smol_timer() {

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

double smol_timer() {
	struct timeval tval;
	gettimeofday(&tval, NULL);
	return (double)tval.tv_sec + (double)tval.tv_usec * 1e-9; 
}

#endif 

#endif 