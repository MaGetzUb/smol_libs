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

/*
NOTE: To build your program for web, remember to add these linker options for emcc: 
 -sAUDIO_WORKLET=1 -sWASM_WORKERS=1 

Also, how Emscripten implements all that, your webserver must have these response headers:

Cross-Origin-Embedder-Policy: require-corp
Cross-Origin-Opener-Policy: same-origin

The local emrun webserver has those response headers, but if you're going to 
host your application somewhere else on the internet you need to have those 
headers.

TODO: Linux backend aswell as DirectSound fallback-backend for older hardware
*/

#ifndef SMOL_AUDIO_H
#define SMOL_AUDIO_H

#ifdef _WIN32
#	ifndef SMOL_PLATFORM_WINDOWS
#		define SMOL_PLATFORM_WINDOWS
#	endif 
#	include <Windows.h>
#	if WINVER >= _WIN32_WINNT_VISTA
#		define COBJMACROS
#		define SMOL_AUDIO_BACKEND_WASAPI
#		include <mmdeviceapi.h>
#		include <audioclient.h>
#		include <initguid.h>
#		include <Functiondiscoverykeys_devpkey.h>
#	else 
#		define SMOL_AUDIOI_BACKEND_DSOUND
#		define CINTERFACE
#		include <dsound.h>
#	endif 
#endif 

#ifdef __EMSCRIPTEN__
#ifndef SMOL_PLATFORM_WEB
#	define SMOL_PLATFORM_WEB
#endif 
#	include <emscripten/emscripten.h>
#	include <emscripten/webaudio.h>
#endif 

#ifdef __linux__
#	ifndef SMOL_PLATFORM_LINUX
#		define SMOL_PLATFORM_LINUX
#	endif 
#	include <unistd.h>
#	include <alsa/asoundlib.h>
#	include <pthread.h>
#endif 

#ifdef _MSC_VER
#	ifndef SMOL_INLINE
#		define SMOL_INLINE __forceinline
#	endif 
#else 
#	ifndef SMOL_INLINE
#		define SMOL_INLINE inline __attribute__((always_inline)) 
#	endif 
#endif 

#ifdef SMOL_PLATFORM_WINDOWS
#define SMOL_ATOMIC volatile
#else 
#include <stdatomic.h>
#define SMOL_ATOMIC _Atomic
#endif 

#ifndef SMOL_ALLOC
#define SMOL_ALLOC( size ) malloc(size)
#endif 

#ifndef SMOL_FREE
#define SMOL_FREE( ptr ) free(ptr)
#endif 

#ifndef SMOL_FREE_PTR
#define SMOL_FREE_PTR free
#endif 

#ifndef SMOL_REALLOC
#define SMOL_REALLOC( old_ptr, new_size ) realloc(old_ptr, new_size)
#endif 

#if _WIN64 || __linux__
typedef unsigned long long smol_size_t;
#else 
typedef unsigned int smol_size_t;
#endif 
typedef char smol_i8;
typedef unsigned char smol_u8;
typedef short smol_i16;
typedef unsigned short smol_u16;
typedef int smol_i32;
typedef unsigned int smol_u32;
typedef long long smol_i64;
typedef unsigned long long smol_u64;
typedef unsigned char smol_byte;


#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef void smol_audio_callback_proc(
	int num_input_channels,
	int num_input_samples,
	const float** inputs,
	int num_output_channels,
	int num_output_samples,
	float** outputs,
	double sample_rate,
	double inv_sample_rate,
	void* user_data
);

typedef enum voice_state {
	SMOL_VOICE_STATE_STOPPED = 0,
	SMOL_VOICE_STATE_PLAYING,
	SMOL_VOICE_STATE_PAUSED
} voice_state;


typedef struct _smol_audiobuffer_t {
	float* samples;
	float peak;
	int sample_rate;
	int num_channels;
	int num_frames;
	int stride;
	double duration;
	void(*free_callback)(void*);
} smol_audiobuffer_t;

typedef enum smol_audio_sample_type {
	SAMPLE_TYPE_S8,
	SAMPLE_TYPE_U8,

	SAMPLE_TYPE_F32_LE,
	SAMPLE_TYPE_F32_BE,

	SAMPLE_TYPE_S16_LE,
	SAMPLE_TYPE_S24_LE,
	SAMPLE_TYPE_S32_LE,
	SAMPLE_TYPE_S16_BE,
	SAMPLE_TYPE_S24_BE,
	SAMPLE_TYPE_S32_BE,

	SAMPLE_TYPE_U16_LE,
	SAMPLE_TYPE_U24_LE,
	SAMPLE_TYPE_U32_LE,
	SAMPLE_TYPE_U16_BE,
	SAMPLE_TYPE_U24_BE,
	SAMPLE_TYPE_U32_BE,


} audio_sample_type;

typedef struct _smol_audio_dec_t {
	const smol_byte* data;
	smol_size_t data_length;
	smol_size_t data_offset;
	smol_u32 sample_rate;
	smol_u32 num_frames;
	smol_u8 num_channels;
} smol_audio_dec_t;

typedef struct _smol_wav_dec_t {
	smol_audio_dec_t decoder;
	smol_u8 bits_per_sample;
	smol_u8 render_wave_format;
} smol_wav_dec_t;

typedef struct _smol_qoa_dec_t {
	smol_audio_dec_t decoder;
	smol_u32 num_frames; //Number of qoa frames
} smol_qoa_dec_t;

//These are for now, but I think mixer and voice will need functions to 
//edit their states. Because this current method won't allow direct access
//for such things. 
typedef struct _smol_mixer_t smol_mixer_t;
typedef struct _smol_voice_t smol_voice_t;
typedef float smol_voice_sample_gen_proc(smol_mixer_t* mixer, int voice_handle, int channel, double sample_rate, double inv_sample_rate, void* user_data);

typedef struct _smol_audio_device_t {
	char name[128 - sizeof(void*)];
	void* device_handle;
} smol_audio_device_t;

//Init, shutdown and set audio engine callbacks:
int smol_audio_playback_init(int sample_rate, int num_channels);
int smol_audio_shutdown();
int smol_audio_set_playback_callback(smol_audio_callback_proc* render_callback, void* user_data);
#define smol_audio_set_mixer_as_callback(mixer_ptr) smol_audio_set_playback_callback(&smol_mixer_mix, (void*)mixer_ptr)

int smol_audio_set_capture_callback(smol_audio_callback_proc* capture_callback, void* user_data);

//Audio buffer creation, deletion and sampling
smol_audiobuffer_t smol_audiobuffer_create_from_interleaved_data(void* data, audio_sample_type sample_type, int num_frames, int num_channels, int sample_rate);
void smol_audiobuffer_destroy(smol_audiobuffer_t* audiobuffer);
float smol_audiobuffer_sample_nearest(smol_audiobuffer_t* buffer, int channel, double time_stamp_sec);
float smol_audiobuffer_sample_linear(smol_audiobuffer_t* buffer, int channel, double time_stamp_sec);
float smol_audiobuffer_sample_cubic(smol_audiobuffer_t* buffer, int channel, double time_stamp_sec);

SMOL_INLINE int smol_audiobuffer_is_valid(smol_audiobuffer_t* audiobuffer) { 
	return (audiobuffer->samples && audiobuffer->sample_rate && audiobuffer->num_channels && audiobuffer->num_channels); 
}

//Audio buffer loading / saving (works only if SMOL_UTILS_H is present, due read entire file.
#ifdef SMOL_UTILS_H
smol_audiobuffer_t smol_create_audiobuffer_from_qoa_file(const char* filepath);
smol_audiobuffer_t smol_create_audiobuffer_from_wav_file(const char* filepath);
#endif 

//QOA decoder
smol_qoa_dec_t smol_qoa_dec_init(const smol_byte* data, smol_size_t size);
void smol_qoa_dec_seek_to_frame(smol_qoa_dec_t* dec, smol_u32 frame_index);
smol_u32 smol_qoa_dec_decode_frame(smol_qoa_dec_t* decoder, float* output, smol_size_t output_size);

//WAV decoder
smol_wav_dec_t smol_wav_dec_init(const smol_byte* data, smol_size_t size);
void smol_wav_dec_seek_to_frame(smol_wav_dec_t* dec, smol_u32 frame_index);
smol_u32 smol_wav_dec_decode_frames(smol_wav_dec_t* dec, float* output, smol_size_t output_size);

//WAV saver
int smol_audiobuffer_save_wav(smol_audiobuffer_t* buffer, const char* file_path, smol_u16 bps);

//Mixer functionality

void smol_mixer_update(smol_mixer_t* mixer);
int smol_mixer_playing_voice_count(smol_mixer_t* mixer);
int smol_mixer_play_voice(smol_mixer_t* mixer, smol_voice_sample_gen_proc* render_callback, void* userdata, float gain, float balance[3]);

void smol_mixer_pause_voice(smol_mixer_t* mixer, int handle);
void smol_mixer_stop_voice(smol_mixer_t* mixer, int handle);
void smol_mixer_resume_voice(smol_mixer_t* mixer, int handle);

void smol_mixer_mix(
	int num_input_channels,
	int num_input_samples,
	const float** inputs,
	int num_output_channels,
	int num_output_samples,
	float** outputs,
	double sample_rate,
	double inv_sample_rate,
	void* user_data
);

#ifdef SMOL_AUDIO_IMPLEMENTATION

#pragma region Audio mixer stuff

typedef struct _smol_voice_t {
	double time_offset;
	double time_scale;
	float gain;
	float balance[3]; //XYZ
	volatile voice_state state;
	smol_voice_sample_gen_proc* render_callback;
	void* user_data;
} smol_voice_t;

typedef struct _smol_mixer_t {
	smol_voice_t voices[64];
	unsigned long long active_voices_mask; //Bitmask 
	smol_audio_callback_proc* post_mix_callback;
	void* post_mix_user_data;
	int num_active_voices;
} smol_mixer_t;

#ifndef SMOL_BSF_BSR
#define SMOL_BSF_BSR
SMOL_INLINE int smol_bsf(unsigned long long mask) {
#ifdef _MSC_VER
	DWORD res;
	BOOL b = BitScanForward64(&res, mask);
	return res;
#else
	return __builtin_ctzll(mask);
#endif 
}

SMOL_INLINE int smol_bsr(unsigned long long mask) {
#ifdef _MSC_VER
	DWORD res; 
	BitScanReverse64(&res, mask);
	return res;
#else
	if(mask == 0) return 0;
	return 63-__builtin_clzll(mask);
#endif
}
#endif 

int smol_mixer_playing_voice_count(smol_mixer_t* mixer) {
	return mixer->num_active_voices;
}

int smol_mixer_next_free_handle(smol_mixer_t* mixer) {
	if(mixer->active_voices_mask == 0xFFFFFFFFFFFFFFFFULL)
		return -1;
	return smol_bsf(~mixer->active_voices_mask);
}

int smol_mixer_play_voice(smol_mixer_t* mixer, smol_voice_sample_gen_proc* render_callback, void* userdata, float gain, float balance[3]) {

	if(mixer->active_voices_mask == 0xFFFFFFFFFFFFFFFFULL)
		return -1;

	unsigned int free_index = smol_mixer_next_free_handle(mixer);

	smol_voice_t* voice = &mixer->voices[free_index];
	voice->time_offset = 0.;
	voice->time_scale = 1.;
	voice->gain = gain;
	voice->balance[0] = balance[0];
	voice->balance[1] = balance[1];
	voice->balance[2] = balance[2];
	voice->state = SMOL_VOICE_STATE_PLAYING;
	voice->render_callback = render_callback;
	voice->user_data = userdata;
	mixer->active_voices_mask |= (1 << free_index);
	mixer->num_active_voices++;

	return free_index;
}

void smol_mixer_pause_voice(smol_mixer_t* mixer, int handle) {
#ifdef _WIN32
	InterlockedExchange(&mixer->voices[handle].state, SMOL_VOICE_STATE_PAUSED);
#endif 
}

void smol_mixer_stop_voice(smol_mixer_t* mixer, int handle) {
#ifdef _WIN32
	InterlockedExchange(&mixer->voices[handle].state, SMOL_VOICE_STATE_STOPPED);
#endif 
}

void smol_mixer_resume_voice(smol_mixer_t* mixer, int handle) {
#ifdef _WIN32
	InterlockedExchange(&mixer->voices[handle].state, SMOL_VOICE_STATE_PLAYING);
#endif 
}

void smol_mixer_update(smol_mixer_t* mixer) {
	int last_index = smol_bsr(mixer->active_voices_mask);

	for(int i = 0; i < last_index+1; i++) {
		if(!(mixer->active_voices_mask & (1 << i)))
			continue;
		if(mixer->voices[i].state == SMOL_VOICE_STATE_STOPPED) {
			mixer->num_active_voices--;
		#if __cplusplus
			mixer->voices[i] = smol_voice_t{ 0 };
		#else
			mixer->voices[i] = (smol_voice_t){ 0 };
		#endif 
			mixer->active_voices_mask &= ~(1 << i);
		}
	}

}

void smol_mixer_mix(
	int num_input_channels,
	int num_input_samples,
	const float** inputs,
	int num_output_channels,
	int num_output_samples,
	float** outputs,
	double sample_rate,
	double inv_sample_rate,
	void* user_data
) {
	smol_mixer_t* mixer = (smol_mixer_t*)user_data;
	int last_index = smol_bsr(mixer->active_voices_mask);

	for(int i = 0; i <= last_index; i++) {
		smol_voice_t* voice = &mixer->voices[i];
		if(voice->state != SMOL_VOICE_STATE_PLAYING)
			continue;

		float balances[2] = { 1.f };

		balances[0] = -voice->balance[0];
		balances[1] = +voice->balance[0];

		balances[0] = fminf(balances[0] * balances[0], 1.f);
		balances[1] = fminf(balances[1] * balances[1], 1.f);

		balances[0] = balances[0] * 0.5f + 0.5f;
		balances[1] = balances[1] * 0.5f + 0.5f;

		for(int j = 0; j < num_output_samples; j++) {
			for(int k = 0; k < num_output_channels; k++) {
				float sample = 0.f;
				if(voice->render_callback) sample = voice->render_callback(mixer, i, k, sample_rate, inv_sample_rate, voice->user_data);
				sample *= voice->gain * balances[k];
				outputs[k][j] += sample;
			}
			voice->time_offset += inv_sample_rate * voice->time_scale;
		}
	}

	if(mixer->post_mix_callback) {
		mixer->post_mix_callback(num_input_channels, num_input_samples, inputs, num_output_channels, num_output_samples, outputs, sample_rate, inv_sample_rate, mixer->post_mix_user_data);
	}
}


#pragma endregion 

#pragma region Audio buffer stuff

//TODO: sanity check this code.
smol_audiobuffer_t smol_audiobuffer_create_from_interleaved_data(void* data, audio_sample_type sample_type, int num_frames, int num_channels, int sample_rate) {
	smol_audiobuffer_t buffer = { 0 };
	buffer.samples = (float*)SMOL_ALLOC(sizeof(float)*num_channels*num_frames);
	buffer.sample_rate = sample_rate;
	buffer.stride = 1; //Result samples are interleaved
	buffer.duration = (double)buffer.num_frames / buffer.sample_rate;
	buffer.free_callback = SMOL_FREE_PTR;
	float* sample = buffer.samples;
	char bytes[] = { 0xAA, 0xBB };
	int is_big_endian = *((unsigned short*)bytes) == 0xAABB;

	int num_samples = num_frames * num_channels;

	switch(sample_type) {
		case SAMPLE_TYPE_S8: {
			static const float max_s8 = 1.f / 128.f;
			for(int i = 0; i < num_samples; i++) {
				*sample = (float)(((char*)data)[i] * max_s8);
				sample++;
			}
		} break;
		case SAMPLE_TYPE_U8: {
			static const float max_u8 = 1.f / 255.f;
			for(int i = 0; i < num_samples; i++) {
				*sample = (float)(((char*)data)[i] * max_u8) * 2.f - 1.f;
				sample++;
			}
		} break;
		case SAMPLE_TYPE_F32_LE:
		case SAMPLE_TYPE_F32_BE:
			if(is_big_endian != (sample_type == SAMPLE_TYPE_F32_BE)) {
				for(int i = 0; i < num_samples; i++)  {
					unsigned int raw_sample = ((unsigned int*)data)[i];

					raw_sample = (
						raw_sample >> 24 | 
						(raw_sample & 0x00FF0000) >> 8 | 
						(raw_sample & 0x0000FF00) << 8 | 
						raw_sample << 24
					);

					*sample = *(float*)raw_sample;
					sample++;
				}
			}
			else {
				for(int i = 0; i < num_samples; i++)  {
					*sample = ((float*)data)[i];
					sample++;
				}
			}
		break;
		case SAMPLE_TYPE_S16_LE:
		case SAMPLE_TYPE_S16_BE: {
			static const float inv_short_max = 1.f / 32768.f;
			if(is_big_endian != (sample_type == SAMPLE_TYPE_S32_BE)) {
				for(int i = 0; i < num_samples; i++) {
					unsigned short raw_sample = ((unsigned short*)data)[i];

					raw_sample = (
						raw_sample >> 8 |
						raw_sample << 8
					);

					*sample = ((float)raw_sample * inv_short_max);
					sample++;
				}
			}
			else {
				for(int i = 0; i < num_samples; i++) {
					*sample = ((float)(((short*)data)[i]) * inv_short_max);
					sample++;
				}
			}
		} break;
		case SAMPLE_TYPE_U16_LE:
		case SAMPLE_TYPE_U16_BE: {
			static const float inv_ushort_max = 1.f / 65536.f;
			if(is_big_endian != (sample_type == SAMPLE_TYPE_S32_BE)) {
				for(int i = 0; i < num_samples; i++) {
					unsigned int raw_sample = ((unsigned int*)data)[i];

					raw_sample = (
						raw_sample >> 8 |
						raw_sample << 8
					);

					*sample = ((float)raw_sample * inv_ushort_max) * 2.f - 1.f;
					sample++;
				}
			}
			else {
				for(int i = 0; i < num_samples; i++) {
					*sample =  (((float)((unsigned short*)data)[i]) * inv_ushort_max) * 2.f - 1.f;
					sample++;
				}
			}
		} break;
		case SAMPLE_TYPE_S24_LE:
		case SAMPLE_TYPE_S24_BE: {
			static const double inv_i24_max = 1. / 16777216.;
			if(is_big_endian != (sample_type == SAMPLE_TYPE_S32_BE)) {
				for(int i = 0; i < num_samples; i++) {
					
					int raw_sample = (
						((char*)data)[i*3+0] << 0  |
						((char*)data)[i*3+1] << 8  |
						((char*)data)[i*3+2] << 16
					);

					*sample = (float)((double)raw_sample * inv_i24_max);
					sample++;
				}
			}
			else {
				for(int i = 0; i < num_samples; i++) {
					int raw_sample = (
						((char*)data)[i*3+0] << 16 |
						((char*)data)[i*3+1] << 8  |
						((char*)data)[i*3+2] << 0
					);
					*sample = (float)((double)(raw_sample) * inv_i24_max);
					sample++;
				}
			}
		} break;
		case SAMPLE_TYPE_U24_LE:
		case SAMPLE_TYPE_U24_BE: {
			static const double inv_u24_max = 1. / 8388608.;
			if(is_big_endian != (sample_type == SAMPLE_TYPE_S32_BE)) {
				for(int i = 0; i < num_samples; i++) {
					
					unsigned int raw_sample = (
						((unsigned char*)data)[i*3+0] << 0  |
						((unsigned char*)data)[i*3+1] << 8  |
						((unsigned char*)data)[i*3+2] << 16
					);

					*sample = (float)((double)raw_sample * inv_u24_max);
					sample++;

				}
			}
			else {
				for(int i = 0; i < num_samples; i++) {
					
					unsigned int raw_sample = (
						((unsigned char*)data)[i*3+0] << 16 |
						((unsigned char*)data)[i*3+1] << 8  |
						((unsigned char*)data)[i*3+2] << 0
					);

					*sample = (float)((double)(raw_sample) * inv_u24_max);
					sample++;

				}
			}
		} break;
		case SAMPLE_TYPE_S32_LE:
		case SAMPLE_TYPE_S32_BE: {
			static const double inv_int_max = 1. / 2147483648.;
			if(is_big_endian != (sample_type == SAMPLE_TYPE_S32_BE)) {
				for(int i = 0; i < num_samples; i++) {
					unsigned int raw_sample = ((unsigned int*)data)[i];

					raw_sample = (
						raw_sample >> 24 | 
						(raw_sample & 0x00FF0000) >> 8 | 
						(raw_sample & 0x0000FF00) << 8 | 
						raw_sample << 24
					);

					*sample = (float)((double)raw_sample * inv_int_max);
					sample++;
				}
			}
			else {
				for(int i = 0; i < num_samples; i++) {
					*sample = (float)((double)(((int*)data)[i]) * inv_int_max);
					sample++;
				}
			}
		} break;
		case SAMPLE_TYPE_U32_LE:
		case SAMPLE_TYPE_U32_BE: {
			static const double inv_uint_max = 1.f / 4294967296.;
			if(is_big_endian != (sample_type == SAMPLE_TYPE_S32_BE)) {
				for(int i = 0; i < num_samples; i++) {
					unsigned int raw_sample = ((unsigned int*)data)[i];

					raw_sample = (
						raw_sample >> 24 | 
						(raw_sample & 0x00FF0000) >> 8 | 
						(raw_sample & 0x0000FF00) << 8 | 
						raw_sample << 24
					);

					*sample = (float)((double)raw_sample * inv_uint_max) * 2.f - 1.f;
					sample++;
				}
			}
			else {
				for(int i = 0; i < num_samples; i++) {
					*sample =  (float)(((double)((unsigned int*)data)[i]) * inv_uint_max) * 2.f - 1.f;
					sample++;
				}
			}
		} break;

	}

}

void smol_audiobuffer_destroy(smol_audiobuffer_t* smol_audiobuffer) {

	if(smol_audiobuffer->free_callback) {
		smol_audiobuffer->free_callback((void*)smol_audiobuffer->samples);
	}

	*smol_audiobuffer = (smol_audiobuffer_t){ 0 };
}

float smol_audiobuffer_sample_nearest(smol_audiobuffer_t* buffer, int channel, double time_stamp_sec) {
	
	double frame_index = time_stamp_sec * (double)buffer->sample_rate;
	long long integer_index = (long long)frame_index;

	int sample_step = buffer->stride;
	
	if(channel > buffer->num_channels) channel = buffer->num_channels;

	float a = 0.f;
	float b = 0.f;

	if(integer_index >= 0 && integer_index < buffer->num_frames) a = buffer->samples[integer_index++ * buffer->num_channels*sample_step + channel * sample_step];

	float result = a;

	return result;

}

float smol_audiobuffer_sample_linear(smol_audiobuffer_t* buffer, int channel, double time_stamp_sec) {
	
	double frame_index = time_stamp_sec * (double)buffer->sample_rate;
	long long integer_index = (long long)frame_index;

	double t = frame_index - integer_index;
	double r = 1.0 - t;
	
	if(channel > buffer->num_channels) channel = buffer->num_channels;

	float a = 0.f;
	float b = 0.f;

	int offset = buffer->num_channels;
	int channel_offset = buffer->stride + channel * buffer->stride;

	if(integer_index >= 0 && integer_index < buffer->num_frames) a = buffer->samples[integer_index++ * offset + channel_offset];
	if(integer_index >= 0 && integer_index < buffer->num_frames) b = buffer->samples[integer_index++ * offset + channel_offset];

	float result = r * a + t * b;

	return result;

}

float smol_audiobuffer_sample_cubic(smol_audiobuffer_t* buffer, int channel, double time_stamp_sec) {
	
	//Convert the time to frame index, and divide it by four
	double frame_index = time_stamp_sec * (double)buffer->sample_rate * .25;

	//Round the sample index down
	long long integer_index = (long long)(frame_index);

	double t = frame_index - integer_index;
	double r = 1.0 - t;
	integer_index = integer_index << 2;

	if(channel > buffer->num_channels) channel = buffer->num_channels;
	
	float a = 0.f;
	float b = 0.f;
	float c = 0.f;
	float d = 0.f;

	if(integer_index >= 0 && integer_index < buffer->num_frames) a = buffer->samples[integer_index++ * buffer->num_channels*buffer->stride + channel * buffer->stride];
	if(integer_index >= 0 && integer_index < buffer->num_frames) b = buffer->samples[integer_index++ * buffer->num_channels*buffer->stride + channel * buffer->stride];
	if(integer_index >= 0 && integer_index < buffer->num_frames) c = buffer->samples[integer_index++ * buffer->num_channels*buffer->stride + channel * buffer->stride];
	if(integer_index >= 0 && integer_index < buffer->num_frames) d = buffer->samples[integer_index++ * buffer->num_channels*buffer->stride + channel * buffer->stride];

	float result = (
		(1.f * r * r * r * a) + 
		(3.f * r * r * t * b) + 
		(3.f * r * t * t * c) + 
		(1.f * t * t * t * d)
	);

	return result;

}

float smol_audiobuffer_sample_linear_monomix(smol_audiobuffer_t* buffer, double time_stamp_sec) {
	
	double frame_index = time_stamp_sec * (double)buffer->sample_rate;
	long long integer_index = (long long)frame_index;
	double t = frame_index - integer_index;
	double r = 1.0 - t;
	int sample_step = buffer->stride * buffer->num_channels;
	

	float a = 0.f;
	float b = 0.f;

	if(integer_index >= 0 && integer_index < buffer->num_frames) {
		for(int i = 0; i < buffer->num_channels; i++) {
			a += buffer->samples[integer_index++ * buffer->num_channels * buffer->stride + i * buffer->stride];
		}
	}

	if(integer_index >= 0 && integer_index < buffer->num_frames) {
		for(int i = 0; i < buffer->num_channels; i++) {
			b += buffer->samples[integer_index++ * buffer->num_channels * buffer->stride + i * buffer->stride];
		}
	}

	float result = r * a + t * b;

	return result;

}

#pragma endregion 

#pragma region Audio buffer loading / saving and streaming etc.
//https://github.com/phoboslab/qoa
//https://qoaformat.org/qoa-specification.pdf
static const int smol_qoa_dequant_table[16][8] = {
	{   1,    -1,    3,    -3,    5,    -5,     7,     -7},
	{   5,    -5,   18,   -18,   32,   -32,    49,    -49},
	{  16,   -16,   53,   -53,   95,   -95,   147,   -147},
	{  34,   -34,  113,  -113,  203,  -203,   315,   -315},
	{  63,   -63,  210,  -210,  378,  -378,   588,   -588},
	{ 104,  -104,  345,  -345,  621,  -621,   966,   -966},
	{ 158,  -158,  528,  -528,  950,  -950,  1477,  -1477},
	{ 228,  -228,  760,  -760, 1368, -1368,  2128,  -2128},
	{ 316,  -316, 1053, -1053, 1895, -1895,  2947,  -2947},
	{ 422,  -422, 1405, -1405, 2529, -2529,  3934,  -3934},
	{ 548,  -548, 1828, -1828, 3290, -3290,  5117,  -5117},
	{ 696,  -696, 2320, -2320, 4176, -4176,  6496,  -6496},
	{ 868,  -868, 2893, -2893, 5207, -5207,  8099,  -8099},
	{1064, -1064, 3548, -3548, 6386, -6386,  9933,  -9933},
	{1286, -1286, 4288, -4288, 7718, -7718, 12005, -12005},
	{1536, -1536, 5120, -5120, 9216, -9216, 14336, -14336},
};


smol_size_t smol_audio_dec_skip(smol_audio_dec_t* dec, smol_size_t bytes) {
	if((dec->data_offset + bytes) < dec->data_length) {
		dec->data_offset += bytes;
		return bytes;
	}
	return dec->data_length - dec->data_offset;
}

smol_u64 smol_audio_dec_peek_u64(smol_audio_dec_t* dec) {
	smol_u64 value = *(smol_u64*)(dec->data + dec->data_offset);
	return value;
}

smol_u64 smol_audio_dec_read_u64(smol_audio_dec_t* dec) {

	if(dec->data_offset >= dec->data_length)
		return 0;

	smol_u64 value = *(smol_u64*)(dec->data + dec->data_offset);
	dec->data_offset += 8;
	
	return value;
}

smol_u32 smol_audio_dec_peek_u32(smol_audio_dec_t* dec) {

	if(dec->data_offset >= dec->data_length)
		return 0;

	return *((smol_u32*)(dec->data + dec->data_offset));
}

smol_u32 smol_audio_dec_read_u32(smol_audio_dec_t* dec) {

	if(dec->data_offset >= dec->data_length)
		return 0;

	smol_u32 value = smol_audio_dec_peek_u32(dec);
	dec->data_offset += 4;

	return value;
}

float smol_audio_dec_peek_f32(smol_audio_dec_t* dec) {

	if(dec->data_offset >= dec->data_length)
		return 0;

	return *((float*)(dec->data + dec->data_offset));
}

float smol_audio_dec_read_f32(smol_audio_dec_t* dec) {

	if(dec->data_offset >= dec->data_length)
		return 0;

	float value = smol_audio_dec_peek_u32(dec);
	dec->data_offset += 4;

	return value;
}

smol_u16 smol_audio_dec_peek_u16(smol_audio_dec_t* dec) {

	if(dec->data_offset >= dec->data_length)
		return 0;

	return *((smol_u16*)(dec->data + dec->data_offset));
}

smol_u16 smol_audio_dec_read_u16(smol_audio_dec_t* dec) {

	if(dec->data_offset >= dec->data_length)
		return 0;

	smol_u16 value = smol_audio_dec_peek_u16(dec);
	dec->data_offset += 2;

	return value;
}

smol_u8 smol_audio_dec_peek_u8(smol_audio_dec_t* dec) {
	return *(dec->data + dec->data_offset);
}

smol_u8 smol_audio_dec_read_u8(smol_audio_dec_t* dec) {

	if(dec->data_offset >= dec->data_length)
		return 0;

	smol_u8 value = smol_audio_dec_peek_u16(dec);
	dec->data_offset++;
	return value;
}

smol_u64 smol_qoa_dec_peek_u64(smol_audio_dec_t* dec) {
	
	static const smol_i32 is_big_endian = (((int)'\xAA\xBB\xCC\xDD') == ((int)0xAABBCCDD));
	smol_u64 value = smol_audio_dec_peek_u64(dec);

	if(is_big_endian == 0) {

		return (
			(value & 0xFF00000000000000ULL) >> 0x38 |
			(value & 0x00FF000000000000ULL) >> 0x28 |
			(value & 0x0000FF0000000000ULL) >> 0x18 |
			(value & 0x000000FF00000000ULL) >> 0x08 |
			(value & 0x00000000FF000000ULL) << 0x08 |
			(value & 0x0000000000FF0000ULL) << 0x18 |
			(value & 0x000000000000FF00ULL) << 0x28 |
			(value & 0x00000000000000FFULL) << 0x38
		);
	
	}

	return value;

}

smol_u64 smol_qoa_dec_read_u64(smol_audio_dec_t* dec) {

	if(dec->data_offset >= dec->data_length)
		return 0;

	smol_u64 result = smol_qoa_dec_peek_u64(dec);
	dec->data_offset += 8;
	
	return result;

}

// https://qoaformat.org/qoa-specification.pdf
smol_qoa_dec_t smol_qoa_dec_init(const smol_byte* data, smol_size_t size) {

	smol_qoa_dec_t result = { 0 };
	smol_audio_dec_t* dec = &result.decoder;
	dec->data = data;
	dec->data_length = size;
	dec->data_offset = 0;
	
	smol_u64 header = smol_qoa_dec_read_u64(&result);
	smol_u32 num_samples = header & 0xFFFFFFFFU;

	if(header && ((header >> 32) != 'qoaf'))
		goto failed;

	smol_size_t qoa_frame_size = (256 * 8 * dec->num_channels + 8);

	smol_u64 frame_header = smol_qoa_dec_peek_u64(&result);
	dec->num_channels = (frame_header >> 56) & 0x0000FFU;
	dec->sample_rate =  (frame_header >> 32) & 0xFFFFFFU;
	dec->num_frames = num_samples;
	result.num_frames = (size - 8) / qoa_frame_size;

	if(!dec->num_channels || !dec->sample_rate)
		goto failed;

	return result;

failed:

#if !__cplusplus
	result = (smol_qoa_dec_t){ 0 };
#else 
	result = _smol_qoa_dec_t{ 0 };
#endif 

	return result;

}

void smol_qoa_dec_seek_to_frame(smol_qoa_dec_t* dec, smol_u32 frame_index) {
	dec->decoder.data_offset = 8 + frame_index * (8 + 2048*dec->decoder.num_channels);
}

smol_u32 smol_qoa_dec_decode_frame(smol_qoa_dec_t* decoder, float* output, smol_size_t output_size) {

	smol_audio_dec_t* dec = &decoder->decoder;

	if(dec->data_offset >= dec->data_length)
		return 0;

	static const float inv_sample_max = 1.f/32768.f;
	smol_u64 bytes = smol_qoa_dec_read_u64(dec);

	smol_u32 num_channels = (bytes >> 56) & 0x000000FF;
	smol_u32 sample_rate  = (bytes >> 32) & 0x00FFFFFF;
	smol_u16 num_samples  = (bytes >> 16) & 0x0000FFFF;
	smol_u16 frame_size   = (bytes >>  0) & 0x0000FFFF;
	smol_u32 samples_read = 0;


	short history[8][4] = { 0 };
	short weights[8][4] = { 0 };

	SMOL_ASSERT(num_channels == dec->num_channels);
	SMOL_ASSERT(num_channels <= 8);

	for(smol_u16 chn = 0; chn < num_channels; chn++) {

		smol_u64 hist = smol_qoa_dec_read_u64(dec);
		smol_u64 wght = smol_qoa_dec_read_u64(dec);

		for(int idx = 0; idx < 4; idx++) {

			history[chn][idx] = (smol_i16)(hist >> 48) & 0xFFFF;
			hist <<= 16;

			weights[chn][idx] = (smol_i16)(wght >> 48) & 0xFFFF;
			wght <<= 16;

		}

	}

	smol_u32 num_bundled_samples = num_channels * 20;

	for(smol_u32 smp = 0; smp < (num_samples * num_channels) && smp < output_size; smp += num_bundled_samples) {
		for(smol_u32 chn = 0; chn < num_channels; chn++) {
		
			smol_u64 slice = smol_qoa_dec_read_u64(dec);
			smol_i16 scale_factor = slice >> 60;
			
			for(smol_u32 s = 0; s < 20; s++) {
				smol_i32 residual = (slice >> 57) & 0x7;
				slice <<= 3;

				smol_i32 dequantized = smol_qoa_dequant_table[scale_factor][residual];
				
				//Predict
				smol_i32 prediction = 0;
				{
						
					for(int i = 0; i < 4; i++)
						prediction += history[chn][i] * weights[chn][i];

					prediction >>= 13;
				}

				//Calculate tample 
				smol_i32 sample = prediction + dequantized;
				{
					if(sample < -32768) sample = -32768;
					if(sample > +32767) sample = +32767;

					int index =  (smp + s * num_channels + chn);

					float float_sample = ((float)sample) * inv_sample_max;

					output[index] = float_sample;
				}

				{ // Update lms
					smol_i32 delta = dequantized >> 4;

					for(smol_i32 i = 0; i < 4; i++) {
						weights[chn][i] += (history[chn][i] < 0 ? -delta : delta);
					}
				
					for(smol_i32 i = 0; i < 3; i++) {
						history[chn][i] = history[chn][i+1];
					}
					history[chn][3] = sample;
				}
			}

		}
	}

	return num_samples;
}

smol_wav_dec_t smol_wav_dec_init(const smol_byte* data, smol_size_t size) {

	smol_wav_dec_t wavdec = { 0 };
	smol_audio_dec_t* dec = &wavdec.decoder;
	dec->data = data;
	dec->data_length = size;
	dec->data_offset = 0;

	if(smol_audio_dec_read_u32(dec) != 'FFIR')
		goto failed;

	smol_u32 wav_size = smol_audio_dec_read_u32(dec);

	if(smol_audio_dec_read_u32(dec) != 'EVAW')
		goto failed;

	//Skip these 8 bytes "fmt " and "WAV" chunk section size
	smol_audio_dec_read_u64(dec);

	wavdec.render_wave_format = smol_audio_dec_read_u16(dec);
	dec->num_channels = smol_audio_dec_read_u16(dec);
	dec->sample_rate = smol_audio_dec_read_u32(dec);
	smol_audio_dec_read_u32(dec);
	//Skip block align, what is it for?
	smol_audio_dec_read_u16(dec);
	wavdec.bits_per_sample = smol_audio_dec_read_u16(dec);
	if(smol_audio_dec_read_u32(dec) != 'atad')
		goto failed;

	smol_u32 num_bytes = smol_audio_dec_read_u32(dec);
	dec->num_frames = num_bytes / ((wavdec.bits_per_sample >> 3) * dec->num_channels);

	return wavdec;

failed:
	
#ifndef __cplusplus
	wavdec = (smol_wav_dec_t){ 0 };
#else
	wavdec = smol_wav_dec_t{};
#endif 
	return wavdec;

}

void smol_wav_dec_seek_to_frame(smol_wav_dec_t* dec, smol_u32 frame_index) {
	dec->decoder.data_offset = 44 + frame_index*(dec->bits_per_sample>>3)*dec->decoder.num_channels;
}

smol_u32 smol_wav_dec_decode_frames(smol_wav_dec_t* dec, float* output, smol_size_t output_size) {
	smol_u32 samples_read = output_size;
	if(samples_read > dec->decoder.num_frames * dec->decoder.num_channels)
		samples_read = dec->decoder.num_frames * dec->decoder.num_channels;
	switch(dec->render_wave_format) {
		case 1: //WAVE_FORMAT_PCM
			switch(dec->bits_per_sample) {
				case 8: {
					static const float inv_max_ubyte = 1.f / (float)0xFF;
					for(smol_u32 i = 0; i < samples_read; i++) {
						output[i] = ((float)((smol_u8)smol_audio_dec_read_u8(dec)) * inv_max_ubyte)*2.f - 1.f;
					}
				} break;
				case 16: {
					static const double inv_max_short = 1. / (double)0x8000;
					for(smol_u32 i = 0; i < samples_read; i++) {
						output[i] = (float)((double)((smol_i16)smol_audio_dec_read_u16(dec)) * inv_max_short);
					}
				} break;
				case 24: {
					static const double inv_max_24bit = 1. / (double)0x800000;
					for(smol_u32 i = 0; i < samples_read; i++) {
						smol_u8 bytes[] = { smol_audio_dec_read_u8(dec), smol_audio_dec_read_u8(dec), smol_audio_dec_read_u8(dec), 0 };
						smol_i32 sample = ((*((smol_i32*)bytes)<<8)>>8);
						output[i] = (float)((double)(sample) * inv_max_24bit);
					}
				} break;
				case 32: {
					static const double inv_max_32bit = 1. / (double)0x80000000;
					for(smol_u32 i = 0; i < samples_read; i++) {
						output[i] = (float)((double)((smol_i32)smol_audio_dec_read_u32(dec)) * inv_max_32bit);
					}
				} break;
			}
		break;
		case 3: //WAVE_FORMAT_IEEE_FLOAT
			for(smol_u32 i = 0; i < samples_read; i++) {
				output[i] = smol_audio_dec_read_f32(dec);
			}
		break;
	}
	return samples_read;
}

#ifdef SMOL_UTILS_H
smol_audiobuffer_t smol_create_audiobuffer_from_qoa_file(const char* filepath) {

	smol_audiobuffer_t buffer = { 0 };
	smol_size_t size;
	const void* data = smol_read_entire_file(filepath, &size);

	if(!data)
		return buffer;

	smol_audio_dec_t qoa_dec = smol_qoa_dec_init((const smol_byte*)data, size).decoder;
	
	
	{
		buffer.num_frames = qoa_dec.num_frames;
		buffer.num_channels = qoa_dec.num_channels;
		buffer.sample_rate = qoa_dec.sample_rate;
		buffer.stride = 1;
		buffer.duration = (double)qoa_dec.num_frames / qoa_dec.sample_rate;
		buffer.free_callback = free;
		smol_i64 num_samples = buffer.num_frames * buffer.num_channels;
		float* buf = buffer.samples = (float*)memset(SMOL_ALLOC(sizeof(float) * num_samples), 0, sizeof(float) * num_samples);

		for(
			smol_u32 n = 0; 
			(n = smol_qoa_dec_decode_frame(&qoa_dec, buf, num_samples)) && num_samples>=0; 
			num_samples -= n, buf += n*qoa_dec.num_channels
		);

		free(data);
	}

	return buffer;

}

smol_audiobuffer_t smol_create_audiobuffer_from_wav_file(const char* filepath) {

	smol_audiobuffer_t buffer = { 0 };
	smol_size_t size;
	const void* data = smol_read_entire_file(filepath, &size);

	if(!data) {
		fputs(stderr, "Can't create audiobuffer from wav file!");
		return buffer;
	}

	smol_wav_dec_t wav_dec = smol_wav_dec_init(data, size);
	smol_audio_dec_t* dec = &wav_dec.decoder;
	{
		buffer.num_frames = dec->num_frames;
		buffer.num_channels = dec->num_channels;
		buffer.sample_rate =  dec->sample_rate;
		buffer.stride = 1;
		buffer.duration = (double)dec->num_frames / dec->sample_rate;
		buffer.free_callback = free;
		smol_i64 num_samples = buffer.num_frames * buffer.num_channels;
		float* buf = buffer.samples = (float*)memset(SMOL_ALLOC(sizeof(float) * num_samples), 0, sizeof(float) * num_samples);

		for(
			smol_u32 n = 0; 
			(n = smol_wav_dec_decode_frames(&wav_dec, buf, num_samples)) && num_samples>=0; 
			num_samples -= n, buf += n*dec->num_channels
		);

		free(data);
	}
	
	return buffer;

}
#endif 

//TODO: This probably shouldn't use FILE* at all but all the stuff should be 
//stored to a memory buffer which then users can themselves, write into a file.
int smol_audiobuffer_save_wav(smol_audiobuffer_t* buffer, const char* file_path, smol_u16 bps) {
	

	FILE* file = NULL;
#ifndef _CRT_SECURE_NO_WARNINGS
	fopen_s(&file, file_path, "wb");
#else 
	file = fopen(file_path, "w");
#endif 
	
	if(!file) return 0;
	
	smol_u32 sample_rate = (smol_u32)buffer->sample_rate;
	smol_i16 num_channels = (smol_i16)buffer->num_channels;
	smol_u16 block_align = (smol_i16)(buffer->num_channels * (bps >> 3));
	smol_u32 byte_rate = (smol_u32)(buffer->sample_rate * block_align);
	smol_u16 sample_type = 1;

	if(bps == 32) sample_type = 3;
	smol_u32 sub_chunk_2_size = buffer->num_frames * block_align;

	fwrite((void*)"RIFF", 4, 1, file);
	fwrite((void*)"\0\0\0\0", 4, 1, file);
	fwrite((void*)"WAVE", 4, 1, file);
	fwrite((void*)"fmt ", 4, 1, file);
	fwrite((void*)"\0\0\0\0", 4, 1, file);
	fwrite((void*)&sample_type, 2, 1, file); // PCM

	fwrite((void*)&num_channels, 2, 1, file);
	fwrite((void*)&sample_rate, 4, 1, file);
	
	fwrite((void*)&byte_rate, 4, 1, file);
	fwrite((void*)&block_align, 2, 1, file);
	fwrite((void*)&bps, 2, 1, file);
	if(sample_type == 3) {
		smol_u16 valid_bits_per_sample = 32;
		smol_u16 channel_mask = (buffer->num_channels == 2) ? 3 : 4;
		smol_u8 bytes[16] = { 0 };
		fwrite((void*)&valid_bits_per_sample, 8, 2, file);
		fwrite((void*)&channel_mask, 8, 2, file);
		fwrite((void*)bytes, 16, 1, file);


	}
	smol_u32 cur = ftell(file) - 20;
	fseek(file, 16, SEEK_SET);
	fwrite(&cur, 4, 1, file);
	fseek(file, cur+20, SEEK_SET);

	fwrite((void*)"data", 4, 1, file);
	fwrite((void*)&sub_chunk_2_size, 4, 1, file);
	
	switch(bps) {
		case 8:
			for(smol_u32 j = 0; j < buffer->num_frames; j++) 
			for(smol_u32 i = 0; i < buffer->num_channels; i++)
			{
				smol_i8 sample = ((float)buffer->samples[j * buffer->num_channels * buffer->stride + i * buffer->stride]* .5f + .5f) * (float)0xFF;
				fwrite((void*)&sample, 1, 1, file);
			}
		break;
		case 16:
			for(smol_u32 j = 0; j < buffer->num_frames; j++) 
			for(smol_u32 i = 0; i < buffer->num_channels; i++)
			{
				smol_i16 sample = (double)buffer->samples[j * buffer->num_channels * buffer->stride + i * buffer->stride] * (double)0x7FFF;
				fwrite((void*)&sample, 2, 1, file);
			}
		case 24:
			for(smol_u32 j = 0; j < buffer->num_frames; j++) 
			for(smol_u32 i = 0; i < buffer->num_channels; i++)
			{
				smol_i32 sample = (double)buffer->samples[j * buffer->num_channels * buffer->stride + i * buffer->stride] * (double)0x7FFFFFFFF;
				fwrite((void*)&sample, 3, 1, file);
			}
		case 32:
			for(smol_u32 j = 0; j < buffer->num_frames; j++) 
			for(smol_u32 i = 0; i < buffer->num_channels; i++)
			{
				float sample = buffer->samples[j * buffer->num_channels * buffer->stride + i * buffer->stride];
				fwrite((void*)&sample, 4, 1, file);
			}
		break;
	}

	smol_u32 file_size = ftell(file) - 8;
	fseek(file, 4, SEEK_SET);
	fwrite((void*)&file_size, 4, 1, file);

	fclose(file);
	return 1;
}


#pragma endregion 

#ifdef SMOL_PLATFORM_WEB
unsigned char audio_context_stack[8192];

typedef struct smol_audio_callback_data_t {
	double sample_rate;
	double inv_sample_rate;
	int num_input_channels;
	int num_output_channels;
	const float* input_channels[32];
	float* output_channels[32];

} smol_audio_callback_data_t;

typedef struct smol_audio_context_t {

	EMSCRIPTEN_WEBAUDIO_T audio_context;
	EMSCRIPTEN_AUDIO_WORKLET_NODE_T worklet_node;
	smol_audio_callback_data_t* callback_data;
	int sample_rate;
	int num_input_channels;
	int num_output_channels;
	smol_audio_callback_proc* render_callback;
	void* render_callback_user_data;
} smol_audio_context_t;

smol_audio_context_t* smol__audio_context;


EM_BOOL smol_audio_callback(
	int numInputs, 
	const AudioSampleFrame* inputs, 
	int numOutputs, 
	AudioSampleFrame* outputs, 
	int numParams, 
	const AudioParamFrame* params, 
	void* userData4
) {

	smol_audio_callback_data_t* cb_data = smol__audio_context->callback_data;

	if(!(cb_data && smol__audio_context->render_callback))
		return EM_FALSE;


	if(numInputs) {
		if(cb_data->num_input_channels != inputs[0].numberOfChannels) {
			for(int i = 0; i < inputs[0].numberOfChannels; i++)
				cb_data->input_channels[i] = &inputs[0].data[i * 128];
			cb_data->num_input_channels = inputs[0].numberOfChannels;
		}
	}

	if(numOutputs) {
		if(cb_data->num_output_channels != outputs[0].numberOfChannels) {
			for(int i = 0; i < outputs[0].numberOfChannels; i++)
				cb_data->output_channels[i] = &outputs[0].data[i * 128];
			cb_data->num_output_channels = outputs[0].numberOfChannels;
		}
	}
	memset(outputs->data, 0, 128*sizeof(float)*outputs[0].numberOfChannels);

	 smol__audio_context->render_callback(
		cb_data->num_input_channels, 
		128, 
		cb_data->input_channels, 
		cb_data->num_output_channels, 
		128, 
		cb_data->output_channels, 
		cb_data->sample_rate,
		cb_data->inv_sample_rate,
		 smol__audio_context->render_callback_user_data
	);


	return EM_TRUE;
	

}

void audio_worklet_created(EMSCRIPTEN_WEBAUDIO_T audioContext, EM_BOOL success, void* userData3);

void audio_thread_initialized(EMSCRIPTEN_WEBAUDIO_T context, EM_BOOL success, void *userData) {

	if (!success) return; // Check browser console in a debug build for detailed errors
	{


		//smol_audio_context_t* context = (smol_audio_context*)userData;

		WebAudioWorkletProcessorCreateOptions options = { 0 };
		options.name = "smol_audio_worklet_processor";
		options.numAudioParams = 0;
		options.audioParamDescriptors = NULL;

		emscripten_create_wasm_audio_worklet_processor_async(context, &options, &audio_worklet_created, userData);

	}
}

void audio_worklet_created(EMSCRIPTEN_WEBAUDIO_T context, EM_BOOL success, void *userData3) {


	int num_outputs[] = { smol__audio_context->num_output_channels };

	if(success) {


		EmscriptenAudioWorkletNodeCreateOptions options = {0};
		options.numberOfInputs = 0;
		options.numberOfOutputs = 1;
		options.outputChannelCounts = num_outputs;

		
		smol__audio_context->callback_data = (smol_audio_callback_data_t*)SMOL_ALLOC(sizeof(smol_audio_callback_data_t));
		memset(smol__audio_context->callback_data, 0, sizeof(*smol__audio_context->callback_data));
		smol__audio_context->callback_data->sample_rate = (double)smol__audio_context->sample_rate;
		smol__audio_context->callback_data->inv_sample_rate = 1.0 / (double)smol__audio_context->sample_rate;
	

		EMSCRIPTEN_AUDIO_WORKLET_NODE_T worklet = emscripten_create_wasm_audio_worklet_node(
			context, 
			"smol_audio_worklet_processor", 
			&options, 
			smol_audio_callback, 
			NULL
		);

		smol__audio_context->worklet_node = worklet;

		  // Connect it to audio context destination
		EM_ASM({emscriptenGetAudioObject($0).connect(emscriptenGetAudioObject($1).destination)}, worklet, context);

	}

}


int smol_audio_playback_init(int sample_rate, int num_channels) {
	
	if(!smol__audio_context) {
		smol__audio_context = memset(malloc(sizeof(*smol__audio_context)), 0, sizeof(*smol__audio_context));
	}

	EmscriptenWebAudioCreateAttributes context_desc = {0};
	context_desc.sampleRate = sample_rate;
	context_desc.latencyHint = "interactive";
	EMSCRIPTEN_WEBAUDIO_T context = emscripten_create_audio_context(&context_desc);
	smol__audio_context->audio_context = context; 
	smol__audio_context->num_output_channels = num_channels;
	smol__audio_context->sample_rate = sample_rate;
	
	emscripten_start_wasm_audio_worklet_thread_async(
		context, 
		audio_context_stack, 
		sizeof(audio_context_stack), 
		&audio_thread_initialized, 
		&context_desc
	);


}


int smol_audio_shutdown() {
	EM_ASM({emscriptenGetAudioObject($0).disconnect()}, smol__audio_context->worklet_node);
	free(smol__audio_context);
	smol__audio_context = NULL;
}
#endif 

#ifdef SMOL_PLATFORM_WINDOWS

#ifdef SMOL_AUDIO_BACKEND_WASAPI
typedef HRESULT smol_CoCreateInstance_proc(const IID* const rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, const IID* const riid, LPVOID* ppv);
typedef HRESULT smol_CoInitializeEx_proc(LPVOID pvReserved, DWORD  dwCoInit);
typedef void __stdcall smol_CoTaskMemFree_proc(_Frees_ptr_opt_ LPVOID pv);

HMODULE smol__Ole32_module;
smol_CoInitializeEx_proc* smol_CoInitializEx;
smol_CoCreateInstance_proc* smol_CoCreateInstance;
smol_CoTaskMemFree_proc* smol_CoTaskMemFree;

typedef struct _smol_audio_context_t {
	
	smol_audio_device_t* audio_devices[2];
	int audio_device_count[2];

	IMMDevice* audio_render_device;
	IAudioClient* audio_render_client;
	IAudioRenderClient* audio_renderer;
	
	IMMDevice* audio_capture_device;
	IAudioClient* audio_capture_client;
	IAudioCaptureClient* audio_capturer;

	volatile UINT render_thread_running;
	volatile UINT capture_thread_running;

	HANDLE rendering_process_ended;
	HANDLE capture_process_ended;

	HANDLE render_buffer_end;
	HANDLE capturer_buffer_end;

	HANDLE render_thread;
	HANDLE capture_thread;

	union {
		WAVEFORMATEXTENSIBLE render_wave_format_ext;
		WAVEFORMATEX render_wave_format;
	};

	
	union {
		WAVEFORMATEXTENSIBLE capture_wave_format_ext;
		WAVEFORMATEX capture_wave_format;
	};


	smol_audio_callback_proc* render_callback;
	void* render_callback_user_data;

	smol_audio_callback_proc* capture_callback;
	void* capture_callback_user_data;

} smol_audio_context_t;


#define SMOL_SAFE_COM_RELEASE(comptr) \
	if((comptr)) \
		(comptr)->lpVtbl->Release((comptr)), \
		comptr=NULL

DEFINE_GUID(SMOL_CLSID_MMDeviceEnumerator, 0xBCDE0395, 0xE52F, 0x467C, 0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E);
DEFINE_GUID(SMOL_IID_IMMDeviceEnumerator,  0xA95664D2, 0x9614, 0x4F35, 0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6);
DEFINE_GUID(SMOL_IID_IAudioClient,         0x1CB9AD4C, 0xDBFA, 0x4c32, 0xB1, 0x78, 0xC2, 0xF5, 0x68, 0xA7, 0x03, 0xB2);
DEFINE_GUID(SMOL_IID_IAudioRenderClient,   0xF294ACFC, 0x3146, 0x4483, 0xA7, 0xBF, 0xAD, 0xDC, 0xA7, 0xC2, 0x60, 0xE2);
DEFINE_GUID(SMOL_IID_IAudioCaptureClient,  0xC8ADBD64, 0xE71E, 0x48A0, 0xA4, 0xDE, 0x18, 0x5C, 0x39, 0x5C, 0xD3, 0x17);


static smol_audio_context_t* smol__audio_context;

DWORD WINAPI smol_audio_playback_thread_proc(LPVOID lpParameter);
DWORD WINAPI smol_audio_capture_thread_proc(LPVOID lpParameter);

void smol_init_audio_context() {
	if(smol__audio_context == NULL) {
		smol__audio_context = memset(malloc(sizeof(smol__audio_context)), 0, sizeof(smol__audio_context));
	}
}

const char* smol_get_audio_device_name(IMMDevice* device) {
	
	static char buffer[128] = { 0 };

	if(device == NULL)
		goto failed;

	HRESULT hres = S_OK;

	LPWSTR id = NULL;
	IPropertyStore* store = NULL;

	hres = IMMDevice_OpenPropertyStore(device, STGM_READ, &store);

	if(FAILED(hres))
		goto failed;
				
	PROPVARIANT variant_device_name;
	hres = IPropertyStore_GetValue(store, &PKEY_Device_FriendlyName, &variant_device_name);

	if(SUCCEEDED(hres)) {
		BOOL subst = FALSE;
		int n = lstrlenW(variant_device_name.pwszVal);
		WideCharToMultiByte(CP_UTF8, 0, variant_device_name.pwszVal, n, buffer, 128, "?", &subst);
		buffer[n] = 0;
	}

	
	SMOL_SAFE_COM_RELEASE(store);

	return buffer;

failed:

	return NULL;

}

smol_audio_device_t* smol_enumerate_audio_devices(int is_capturer, int* device_count) {

	smol_init_audio_context();

	if(!smol__Ole32_module) {
		smol__Ole32_module = LoadLibrary(TEXT("Ole32.dll"));
		smol_CoInitializEx = (smol_CoInitializeEx_proc*)GetProcAddress(smol__Ole32_module, "CoInitializeEx");
		smol_CoCreateInstance = (smol_CoCreateInstance_proc*)GetProcAddress(smol__Ole32_module, "CoCreateInstance");
		smol_CoTaskMemFree = (smol_CoTaskMemFree_proc*)GetProcAddress(smol__Ole32_module, "CoTaskMemFree");
	}

	HRESULT hres = smol_CoInitializEx(NULL, COINIT_MULTITHREADED);
	IMMDeviceEnumerator* device_enumerator = NULL;
	IMMDeviceCollection* collection = NULL;

	if(FAILED(hres)) {
		goto failed;
	}

	hres = smol_CoCreateInstance(
		&SMOL_CLSID_MMDeviceEnumerator,
		NULL,
		CLSCTX_ALL,
		&SMOL_IID_IMMDeviceEnumerator,
		&device_enumerator
	);

	if(FAILED(hres)) {
		fprintf(stderr, "COM error: Failed to create audio device enumerator!\n");
		goto failed;
	}

	hres = IMMDeviceEnumerator_EnumAudioEndpoints(device_enumerator, is_capturer ? eCapture : eRender, DEVICE_STATE_ACTIVE, &collection);
	if(FAILED(hres)){
		fprintf(stderr, "COM error: Failed to enumerate audio enpoints!\n");
		goto failed;
	}

	UINT num_devices = 0;
	hres = IMMDeviceCollection_GetCount(collection, &num_devices);
	if(FAILED(hres)) {
		fprintf(stderr, "COM error: Failed acquire device count!\n");
		goto failed;
	}
	
	smol__audio_context->audio_devices[is_capturer] = (smol_audio_device_t*)malloc(sizeof(smol_audio_device_t) * num_devices);
	memset((void*)smol__audio_context->audio_devices[is_capturer], 0, sizeof(smol_audio_device_t)*num_devices);
	
	for(UINT i = 0; i < num_devices; i++) {
		IMMDevice* device = NULL;
		hres = IMMDeviceCollection_Item(collection, i, &device);
		if(FAILED(hres))
			continue;

		const char* name = NULL;
		if((name = smol_get_audio_device_name(device)) != NULL) {
			int index = smol__audio_context->audio_device_count[is_capturer]++;
			memcpy((void*)smol__audio_context->audio_devices[is_capturer][index].name, (const void*)name, 128-sizeof(void*));
			smol__audio_context->audio_devices[is_capturer][index].device_handle = device;
		}

		SMOL_SAFE_COM_RELEASE(device);
	}

	int count = smol__audio_context->audio_device_count[is_capturer];
	smol__audio_context->audio_devices[is_capturer] = realloc(smol__audio_context->audio_devices[is_capturer], count *  sizeof(smol_audio_device_t));

	*device_count = count;

	return smol__audio_context->audio_devices[is_capturer];
failed:
	SMOL_SAFE_COM_RELEASE(collection);
	SMOL_SAFE_COM_RELEASE(device_enumerator);
	return NULL;
}



int smol_audio_playback_init(int sample_rate, int num_channels, int device) {

	smol_init_audio_context();

	if(!smol__Ole32_module) {
		smol__Ole32_module = LoadLibrary(TEXT("Ole32.dll"));
		smol_CoInitializEx = (smol_CoInitializeEx_proc*)GetProcAddress(smol__Ole32_module, "CoInitializeEx");
		smol_CoCreateInstance = (smol_CoCreateInstance_proc*)GetProcAddress(smol__Ole32_module, "CoCreateInstance");
		smol_CoTaskMemFree = (smol_CoTaskMemFree_proc*)GetProcAddress(smol__Ole32_module, "CoTaskMemFree");
	}

	WAVEFORMATEXTENSIBLE result_format = { 0 };
	WAVEFORMATEX* render_wave_format = NULL;
	WAVEFORMATEX* final_format = NULL;

	HRESULT hres = smol_CoInitializEx(NULL, COINIT_MULTITHREADED);

	if(FAILED(hres)) {
		fprintf(stderr, "FAILED to CoInitialize!\n");
		return 0;
	}





	smol__audio_context->audio_render_device = NULL;
	
	if(device < 0) {

		IMMDeviceEnumerator* enumerator = NULL;
		hres = smol_CoCreateInstance(&SMOL_CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, &SMOL_IID_IMMDeviceEnumerator, (void**)&enumerator);
		if(FAILED(hres)) {
			fputs(stderr, "FAILED to CoCreateInstance for device enumerator!");
			goto failure;
		}


		hres = IMMDeviceEnumerator_GetDefaultAudioEndpoint(enumerator, eRender, eConsole, &smol__audio_context->audio_render_device);
		if(FAILED(hres)) {
			fputs(stderr, "FAILED to acquire the default audio enpoint!");
		}

		SMOL_SAFE_COM_RELEASE(enumerator);
		if(FAILED(hres)) {
			fputs(stderr, "FAILED to release enumerator!");
			goto failure;
		}

	}
	else {
		int count = 0;
		if(!smol__audio_context->audio_devices[0]) {
			smol_enumerate_audio_devices(0, &count);
		}
		smol__audio_context->audio_render_device = (IMMDevice*)smol__audio_context->audio_devices[0][device].device_handle;
	}

	if(FAILED(hres)) {
		fputs(stderr, "FAILED to Get default audio endpoint!");
		goto failure;
	}


	hres = IMMDevice_Activate(smol__audio_context->audio_render_device, &SMOL_IID_IAudioClient, CLSCTX_ALL, NULL, &smol__audio_context->audio_render_client);
	if(FAILED(hres)) {
		fputs(stderr, "FAILED to activate audio device!");
		goto failure;
	}

	hres = IAudioClient_GetMixFormat(smol__audio_context->audio_render_client, &render_wave_format);
	if(FAILED(hres)) {
		fputs(stderr, "FAILED to get audio renderer mix format!");
	}

	/*TODO: stuff?*/
	if(render_wave_format->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
			WAVEFORMATEXTENSIBLE* ext = (WAVEFORMATEXTENSIBLE*)render_wave_format;

			USHORT id = EXTRACT_WAVEFORMATEX_ID(&ext->SubFormat);
				
			switch(id) {
				case WAVE_FORMAT_PCM:
					
				break;
				case WAVE_FORMAT_IEEE_FLOAT:

				break;
			}
			result_format = *ext;
	}	
	else {
		result_format.Format = *render_wave_format;
	}


	REFERENCE_TIME requested_duration = 10*100000LL; 

	hres = IAudioClient_IsFormatSupported(smol__audio_context->audio_render_client, AUDCLNT_SHAREMODE_SHARED, &result_format.Format, &final_format);
	if(FAILED(hres)) {
		fputs(stderr, "FAILED in testing format support!");
		goto failure;
	}
	hres = IAudioClient_Initialize(smol__audio_context->audio_render_client, AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, requested_duration, 0, &result_format, NULL);

	if(FAILED(hres)) {
		fputs(stderr, "FAILED to initialize audio renderer!");
		goto failure;
	}

	hres = IAudioClient_GetService(smol__audio_context->audio_render_client, &SMOL_IID_IAudioRenderClient, &smol__audio_context->audio_renderer);
	
	if(FAILED(hres)) {
		fputs(stderr, "FAILED to acquire audio renderer client!");
		goto failure;
	}

	
	if(final_format) {
		if(final_format->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
			smol__audio_context->render_wave_format_ext = *(WAVEFORMATEXTENSIBLE*)final_format;
		else
			smol__audio_context->render_wave_format = *final_format;
		
	} else {
		if(result_format.Format.wFormatTag == WAVE_FORMAT_EXTENSIBLE)
			smol__audio_context->render_wave_format_ext = result_format;
		else 
			smol__audio_context->render_wave_format = result_format.Format;
	}	

	if(render_wave_format) smol_CoTaskMemFree((LPVOID)render_wave_format), render_wave_format = NULL;

	if(FAILED(hres)) {
		fputs(stderr, "FAILED to start audio renderer!");
		goto failure;
	}

	smol__audio_context->render_buffer_end = CreateEvent(0, FALSE, FALSE, NULL);
	smol__audio_context->rendering_process_ended = CreateEvent(0, FALSE, FALSE, NULL);

	hres = IAudioClient_SetEventHandle(smol__audio_context->audio_render_client, smol__audio_context->render_buffer_end);
	if(FAILED(hres)) {
		fputs(stderr, "FAILED to set event handle for audio client!");
		goto failure;
	}

	smol__audio_context->render_thread = CreateThread(NULL, 8192, &smol_audio_playback_thread_proc, NULL, 0, NULL);
	return 1;

failure:

	if(render_wave_format) smol_CoTaskMemFree((LPVOID)render_wave_format);

	SMOL_SAFE_COM_RELEASE(smol__audio_context->audio_renderer);
	SMOL_SAFE_COM_RELEASE(smol__audio_context->audio_render_client);
	SMOL_SAFE_COM_RELEASE(smol__audio_context->audio_render_device);
	
	return 0;

}

int smol_audio_capture_init(int sample_rate, int num_channels, int device) {

	smol_init_audio_context();
	
	if(!smol__Ole32_module) {
		smol__Ole32_module = LoadLibrary(TEXT("Ole32.dll"));
		smol_CoInitializEx = (smol_CoInitializeEx_proc*)GetProcAddress(smol__Ole32_module, "CoInitializeEx");
		smol_CoCreateInstance = (smol_CoCreateInstance_proc*)GetProcAddress(smol__Ole32_module, "CoCreateInstance");
		smol_CoTaskMemFree = (smol_CoTaskMemFree_proc*)GetProcAddress(smol__Ole32_module, "CoTaskMemFree");
	}

	WAVEFORMATEXTENSIBLE result_format = { 0 };
	WAVEFORMATEX* capture_wave_format = NULL;
	WAVEFORMATEX* final_format = NULL;

	HRESULT hres = smol_CoInitializEx(NULL, COINIT_MULTITHREADED);


	IMMDevice* endpoint = NULL;
	if(device == -1) {
		IMMDeviceEnumerator* enumerator = NULL;
		hres = smol_CoCreateInstance(&SMOL_CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, &SMOL_IID_IMMDeviceEnumerator, (void**)&enumerator);
		if(FAILED(hres)) {
			fputs(stderr, "FAILED to CoCreateInstance for device enumerator!");
			goto failure;
		}


		hres = IMMDeviceEnumerator_GetDefaultAudioEndpoint(enumerator, eCapture, eConsole, &smol__audio_context->audio_render_device);
		if(FAILED(hres)) {
			fputs(stderr, "FAILED to acquire the default audio enpoint!");
		}

		SMOL_SAFE_COM_RELEASE(enumerator);
		if(FAILED(hres)) {
			fputs(stderr, "FAILED to release enumerator!");
			goto failure;
		}
	}
	else {
		if(smol__audio_context->audio_devices[1] == NULL) {
			int device_count;
			smol_enumerate_audio_devices(1, &device_count);
		}
		endpoint = smol__audio_context->audio_devices[1][device].device_handle;
	}

	if(endpoint) {
		IMMDevice_Activate(endpoint, &SMOL_IID_IAudioClient, CLSCTX_ALL, NULL, &smol__audio_context->audio_capture_client);
	}

	hres = IAudioClient_GetMixFormat(smol__audio_context->audio_capture_client, &capture_wave_format);
	if(FAILED(hres)) {
		fputs(stderr, "FAILED to get audio renderer mix format!");
	}

	/*TODO: stuff?*/
	if(capture_wave_format->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
		WAVEFORMATEXTENSIBLE* ext = (WAVEFORMATEXTENSIBLE*)capture_wave_format;

		USHORT id = EXTRACT_WAVEFORMATEX_ID(&ext->SubFormat);
				
		switch(id) {
			case WAVE_FORMAT_PCM:
					
			break;
			case WAVE_FORMAT_IEEE_FLOAT:

			break;
		}
		result_format = *ext;
	}	
	else {
		result_format.Format = *capture_wave_format;
	}


	REFERENCE_TIME requested_duration = 0*100000LL; 

	hres = IAudioClient_IsFormatSupported(smol__audio_context->audio_capture_client, AUDCLNT_SHAREMODE_SHARED, &result_format.Format, &final_format);
	if(FAILED(hres)) {
		fputs(stderr, "FAILED in testing format support!");
		goto failure;
	}
	hres = IAudioClient_Initialize(smol__audio_context->audio_capture_client, AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, requested_duration, 0, &result_format, NULL);

	if(FAILED(hres)) {
		fputs(stderr, "FAILED to initialize audio renderer!");
		goto failure;
	}

	hres = IAudioClient_GetService(smol__audio_context->audio_capture_client, &SMOL_IID_IAudioCaptureClient, &smol__audio_context->audio_capturer);
	
	if(FAILED(hres)) {
		fputs(stderr, "FAILED to acquire audio renderer client!");
		goto failure;
	}

	
	if(final_format) {
		if(final_format->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
			smol__audio_context->capture_wave_format_ext = *(WAVEFORMATEXTENSIBLE*)final_format;
		else
			smol__audio_context->capture_wave_format = *final_format;
		
	} else {
		if(result_format.Format.wFormatTag == WAVE_FORMAT_EXTENSIBLE)
			smol__audio_context->capture_wave_format_ext = result_format;
		else 
			smol__audio_context->capture_wave_format = result_format.Format;
	}	

	if(capture_wave_format) smol_CoTaskMemFree((LPVOID)capture_wave_format), capture_wave_format = NULL;

	if(FAILED(hres)) {
		fputs(stderr, "FAILED to start audio capturer!");
		goto failure;
	}

	smol__audio_context->capturer_buffer_end = CreateEvent(0, FALSE, FALSE, NULL);
	smol__audio_context->capture_process_ended = CreateEvent(0, FALSE, FALSE, NULL);

	hres = IAudioClient_SetEventHandle(smol__audio_context->audio_capture_client, smol__audio_context->capturer_buffer_end);
	if(FAILED(hres)) {
		fputs(stderr, "FAILED to set event handle for audio client!");
		goto failure;
	}

	smol__audio_context->capture_thread = CreateThread(NULL, 8192, &smol_audio_capture_thread_proc, NULL, 0, NULL);
	return 1;

failure:
	if(capture_wave_format) smol_CoTaskMemFree((LPVOID)capture_wave_format);

	SMOL_SAFE_COM_RELEASE(smol__audio_context->audio_capturer);
	SMOL_SAFE_COM_RELEASE(smol__audio_context->audio_capture_client);
	SMOL_SAFE_COM_RELEASE(smol__audio_context->audio_capture_device);

	return 0;
}

#define smol_audio_playback_init(sample_rate, num_channels) smol_audio_playback_init(sample_rate, num_channels, -1)

DWORD WINAPI smol_audio_playback_thread_proc(LPVOID lpParameter) {
	
	HRESULT result = S_OK;
	int processing = SUCCEEDED(IAudioClient_Start(smol__audio_context->audio_render_client));

	UINT32 buffer_padding = 0;
	BYTE* buffer_data = NULL;
	UINT32 buffer_frame_size = 0;
	result = IAudioClient_GetBufferSize(smol__audio_context->audio_render_client, &buffer_frame_size);

	float* mix_buffer = (float*)_aligned_malloc(buffer_frame_size*sizeof(float), 16);
	memset(mix_buffer, 0, buffer_frame_size*sizeof(float));
	float* channels[32];

	WORD num_channels = smol__audio_context->render_wave_format.nChannels;
	double sample_rate = smol__audio_context->render_wave_format.nSamplesPerSec;
	double inv_sample_rate = 1.0 / sample_rate;

	UINT chunk_size = buffer_frame_size / num_channels;
	for(UINT i = 0; i < num_channels; i++) {
		channels[i] = &mix_buffer[(i*chunk_size) >> 2];
	}


	while(WaitForSingleObject(smol__audio_context->rendering_process_ended, 0) != WAIT_OBJECT_0) {

		result = IAudioClient_GetCurrentPadding(smol__audio_context->audio_render_client, &buffer_padding);
		if(FAILED(result))
			goto reset;
		
		UINT32 samples_to_write = (buffer_frame_size - buffer_padding) / num_channels;

		result = IAudioRenderClient_GetBuffer(smol__audio_context->audio_renderer, samples_to_write, &buffer_data);
		if(FAILED(result))
			goto reset;

		if(buffer_data) {


			if(smol__audio_context->render_wave_format.wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
				USHORT id = EXTRACT_WAVEFORMATEX_ID(&smol__audio_context->render_wave_format_ext.SubFormat);
				
				if(id == WAVE_FORMAT_IEEE_FLOAT) {
					for(int i = 0; i < samples_to_write; i++) {
						for(int j = 0; j < num_channels; j++) {
							channels[j][i] = 0.f;
						}
					}
					smol__audio_context->render_callback(
						0, 
						0, 
						NULL, 
						num_channels, 
						samples_to_write, 
						channels, 
						sample_rate, 
						inv_sample_rate, 
						smol__audio_context->render_callback_user_data
					);

					for(int i = 0; i < samples_to_write; i++) {
						for(int j = 0; j < num_channels; j++) {
							((float*)buffer_data)[i*num_channels+j] = channels[j][i];
						}
					}
				}

			}

			result = IAudioRenderClient_ReleaseBuffer(smol__audio_context->audio_renderer, samples_to_write, 0);
		}

		reset:

		if(SUCCEEDED(result) && !processing) {
			processing = SUCCEEDED(IAudioClient_Start(smol__audio_context->audio_render_client));
		}

		WaitForSingleObject(smol__audio_context->render_buffer_end, INFINITE);
		continue;
		
		/* Recovery here */


	}

	WaitForSingleObject(smol__audio_context->render_buffer_end, INFINITE);
	_aligned_free(mix_buffer);

	return 0;
}

TCHAR *GetLastErrorMessage(DWORD error) 
{ 
	static TCHAR msg[512] = { 0 };
	if(!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, error,0,msg, 511, NULL)) {
		return (GetLastErrorMessage(GetLastError()));   
	}
	
	return msg; 
}

#ifdef UNICODE
#define PrintErrorMessage(ec) \
	if(ec != S_OK) wprintf("%s\n", GetLastErrorMessage(ec)), SMOL_BREAKPOINT()
#else 
#define PrintErrorMessage(ec) \
	if(ec != S_OK) printf("%s\n", GetLastErrorMessage(ec)), SMOL_BREAKPOINT()
#endif 

DWORD WINAPI smol_audio_capture_thread_proc(LPVOID lpParameter) {

	IAudioClient* client = smol__audio_context->audio_capture_client;
	IAudioCaptureClient* capturer = smol__audio_context->audio_capturer;

	HRESULT result = S_OK;

	result = IAudioClient_Start(smol__audio_context->audio_capture_client);

	PrintErrorMessage(result);

	int processing = SUCCEEDED(result);

	int num_channels = smol__audio_context->capture_wave_format.nChannels;

	UINT packet_size;
	UINT num_frames;
	result = IAudioClient_GetBufferSize(client, &num_frames);
	float* buffers[8] = { 0 };

	PrintErrorMessage(result);

	int planar_buffer_size = num_frames * num_channels * sizeof(float);
	float* planar_buffer = _aligned_malloc(planar_buffer_size, 16);
	for(int i = 0; i < num_channels; i++) {
		buffers[i] = &planar_buffer[i * (num_frames / num_channels)];
	}
	DWORD wave_format = smol__audio_context->capture_wave_format.wFormatTag;
	double sample_rate = smol__audio_context->capture_wave_format.nSamplesPerSec;
	double inv_sample_rate = 1.0 / sample_rate;

	const DWORD error_mask = AUDCLNT_ERR(0);

	while(WaitForSingleObject(smol__audio_context->capture_process_ended, 0) != WAIT_OBJECT_0) {
		


		result = IAudioCaptureClient_GetNextPacketSize(capturer, &packet_size);
		memset(planar_buffer, 0, planar_buffer_size);
		
		WaitForSingleObject(smol__audio_context->render_buffer_end, INFINITE);

		PrintErrorMessage(result);

		BYTE* memory = NULL;
		UINT num_frames = 0;
		DWORD flags = 0;
		BYTE* packet_data = NULL;
		result = IAudioCaptureClient_GetBuffer(capturer, &memory, &num_frames, &flags, NULL, NULL);
		if(FAILED(result)) printf("%x\n", (result & (~error_mask)));
		PrintErrorMessage(result);
		
		if(SUCCEEDED(result) && wave_format == WAVE_FORMAT_IEEE_FLOAT) {

			for(int i = 0; i < packet_size * num_channels; i++) {
				buffers[i % num_channels][i / num_channels] = ((float*)packet_data)[i];
			}
		
			if(smol__audio_context->capture_callback) {
				smol__audio_context->capture_callback(
					num_channels, 
					packet_size, 
					buffers, 
					0, 
					0, 
					NULL, 
					sample_rate, 
					inv_sample_rate, 
					smol__audio_context->capture_callback_user_data
				);
			}
			
			result = IAudioCaptureClient_ReleaseBuffer(capturer, packet_size);
			
			PrintErrorMessage(result);
		}
		


	}

	_aligned_free(planar_buffer);


	return 0;

}

int smol_audio_shutdown() {


	//Signal audio renderer thread to stop processing.
	if(smol__audio_context->render_thread) {
		SetEvent(smol__audio_context->rendering_process_ended);
		if(WaitForSingleObject(smol__audio_context->render_thread, INFINITE) == WAIT_OBJECT_0) {

			SMOL_SAFE_COM_RELEASE(smol__audio_context->audio_renderer);
			SMOL_SAFE_COM_RELEASE(smol__audio_context->audio_render_client);
			SMOL_SAFE_COM_RELEASE(smol__audio_context->audio_render_device);

		}
	}

	//Signal capture thread to stop processing.
	if(smol__audio_context->capture_thread) {
		SetEvent(smol__audio_context->capture_process_ended);
		if(WaitForSingleObject(smol__audio_context->capture_thread, INFINITE) == WAIT_OBJECT_0) {

			SMOL_SAFE_COM_RELEASE(smol__audio_context->audio_capturer);
			SMOL_SAFE_COM_RELEASE(smol__audio_context->audio_capture_client);
			SMOL_SAFE_COM_RELEASE(smol__audio_context->audio_capture_device);

		}
	}

	free(smol__audio_context);
	smol__audio_context = NULL;

	return 1;
}

#endif 
#undef COBJMACROS
#endif

#ifdef SMOL_PLATFORM_LINUX

typedef struct smol_audio_context_t {
    snd_pcm_t *alsa_handle;
	pthread_t render_thread;
	int num_channels;
	int sample_rate;
	volatile int thread_running;
	volatile smol_audio_callback_proc* render_callback;
	volatile void* render_callback_user_data;
} smol_audio_context_t;

static smol_audio_context_t* smol__audio_context;

typedef int smol_pthread_create_proc(pthread_t *__restrict __newthread, const pthread_attr_t *__restrict __attr, void *(*__start_routine) (void *), void *__restrict __arg) __THROWNL __nonnull ((1, 3));
typedef int smol_pthread_join_proc(pthread_t __th, void **__thread_return);

void smol_init_audio_context() {
	if(smol__audio_context == NULL) {
		smol__audio_context = memset(malloc(sizeof(*smol__audio_context)), 0, sizeof(*smol__audio_context));
	}
}

void* smol_audio_thread_callback(void*);

int smol_audio_playback_init(int sample_rate, int num_channels) {

	smol_init_audio_context();

	int error = 0;

	if((error = snd_pcm_open(
		&smol__audio_context->alsa_handle, 
		"default", 
		SND_PCM_STREAM_PLAYBACK, 
		0
	)) < 0) {
		fprintf(stderr, "Error during opening alsa renderer: %s\n", snd_strerror(error));
		return 0;
	}

	snd_pcm_t* alsa_handle = smol__audio_context->alsa_handle;

	// Prepare the parameter structure and set default parameters
	snd_pcm_hw_params_t *params;
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_hw_params_any(alsa_handle, params);

	// Set other parameters
	snd_pcm_hw_params_set_access(alsa_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(alsa_handle, params, SND_PCM_FORMAT_S16_LE);
	snd_pcm_hw_params_set_rate(alsa_handle, params, sample_rate, 0);
	snd_pcm_hw_params_set_channels(alsa_handle, params, num_channels);
	snd_pcm_hw_params_set_period_size(alsa_handle, params, sample_rate/100, 0);
	snd_pcm_hw_params_set_periods(alsa_handle, params, 4, 0);

	if((error = snd_pcm_hw_params(alsa_handle, params)) < 0) {
		fprintf(stderr, "Error during setting alsa hardware parameters: %s\n", snd_strerror(error));
	}


	smol__audio_context->sample_rate = sample_rate;
	smol__audio_context->num_channels = num_channels;

	pthread_create(&smol__audio_context->render_thread, NULL, &smol_audio_thread_callback, NULL);

	return 1;
}

int smol_audio_shutdown() {
	void* ret;
	smol__audio_context->thread_running = 0;
	pthread_join(smol__audio_context->render_thread, &ret);

	free((void*)smol__audio_context);
	smol__audio_context = NULL;
}

void* smol_audio_thread_callback(void* data) {
	(void)data;

	float* channels[32];
	snd_pcm_t* pcm_handle = smol__audio_context->alsa_handle;
	snd_pcm_uframes_t buffer_size;
	snd_pcm_uframes_t period_size;
	snd_pcm_get_params(pcm_handle, &buffer_size, &period_size);
	int num_channels = smol__audio_context->num_channels;
	double sample_rate = smol__audio_context->sample_rate;
	double inv_sample_rate = smol__audio_context->sample_rate;

	float* channel_memory = (float*)aligned_alloc(period_size * num_channels * sizeof(float), 16);
	short* buffer_data = (short*)malloc(period_size * num_channels * sizeof(short));
	smol__audio_context->thread_running = 1;
	
	for(int i = 0; i < num_channels; i++) {
		channels[i] = channel_memory + i*period_size;
	}


	snd_pcm_start(pcm_handle);

	snd_pcm_prepare(pcm_handle);
	while(smol__audio_context->thread_running) {
		if(!smol__audio_context->render_callback) {
			usleep(500000);
			continue;
		}

		int avail_frames = snd_pcm_avail_update(pcm_handle);
        if(avail_frames >= period_size) {
			for(int i = 0; i < avail_frames; i++) {
				for(int j = 0; j < num_channels; j++) {
					channels[j][i] = 0.f;
				}
			}
			smol__audio_context->render_callback(
				0, 
				0, 
				NULL, 
				num_channels, 
				avail_frames, 
				channels, 
				sample_rate, 
				inv_sample_rate, 
				smol__audio_context->render_callback_user_data
			);

			for(int i = 0; i < avail_frames; i++) {
				for(int j = 0; j < num_channels; j++) {
					((short*)buffer_data)[i*num_channels+j] = (short)(channels[j][i] * 32767.0f);
				}
			}
			snd_pcm_sframes_t frames_written = snd_pcm_writei(pcm_handle, (void*)buffer_data, avail_frames);
			if (frames_written < 0) {
                fprintf(stderr, "Error writing audio data: %s\n", snd_strerror(frames_written));
            }
		}
	}
	snd_pcm_drain(pcm_handle);
	snd_pcm_close(pcm_handle);
	free(channel_memory);
	free(buffer_data);

	return NULL;
}

#endif 

int smol_audio_set_playback_callback(smol_audio_callback_proc* render_callback, void* user_data) {
	smol__audio_context->render_callback = render_callback;
	smol__audio_context->render_callback_user_data = user_data;
	return 1;
}

int smol_audio_set_capture_callback(smol_audio_callback_proc* capture_callback, void* user_data) {
	smol__audio_context->capture_callback = capture_callback;
	smol__audio_context->capture_callback_user_data = user_data;
	return 1;
}
#endif 
#endif 