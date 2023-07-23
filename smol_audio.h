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

#ifndef SMOL_PLATFORM_WINDOWS
#	define SMOL_PLATFORM_WINDOWS
#endif 

#include <Windows.h>

#if WINVER >= _WIN32_WINNT_VISTA
#define COBJMACROS
#define SMOL_AUDIO_BACKEND_WASAPI
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <initguid.h>
#else 
#define SMOL_AUDIOI_BACKEND_DSOUND
#define CINTERFACE
#include <dsound.h>
#endif 
#endif 

#ifdef __EMSCRIPTEN__
#ifndef SMOL_PLATFORM_WEB
#	define SMOL_PLATFORM_WEB
#endif 
#include <emscripten/emscripten.h>
#include <emscripten/webaudio.h>
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

typedef struct _smol_mixer_t smol_mixer_t;
typedef struct _smol_voice_t smol_voice_t;
typedef float smol_voice_sample_gen_proc(smol_voice_t* voice, int channel, double sample_rate, double inv_sample_rate, void* user_data);
int smol_audio_init(int sample_rate, int num_channels);
int smol_audio_shutdown();
int smol_audio_set_callback(smol_audio_callback_proc* callback, void* user_data);

#ifdef SMOL_AUDIO_IMPLEMENTATION

typedef struct _smol_voice_t {
	double time_offset;
	double time_scale;
	float gain;
	float balance[3]; //XYZ
	volatile voice_state state;
	smol_voice_sample_gen_proc* callback;
	void* user_data;
} smol_voice_t;

typedef struct _smol_mixer_t {
	smol_voice_t voices[64];
	unsigned long long active_voices_mask; //Bitmask 
	smol_audio_callback_proc* post_mix_callback;
	void* post_mix_user_data;
	int num_active_voices;
} smol_mixer_t;


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

int smol_mixer_playing_voice_count(smol_mixer_t* mixer) {
	return mixer->num_active_voices;
}

int smol_mixer_next_free_handle(smol_mixer_t* mixer) {
	if(mixer->active_voices_mask == 0xFFFFFFFFFFFFFFFFULL)
		return -1;
	return smol_bsf(~mixer->active_voices_mask);
}

int smol_mixer_play_voice(smol_mixer_t* mixer, smol_voice_sample_gen_proc* callback, void* userdata, float gain, float balance[3]) {

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
	voice->callback = callback;
	voice->user_data = userdata;
	mixer->active_voices_mask |= (1 << free_index);
	mixer->num_active_voices++;

	return free_index;
}

void smol_mixer_pause_voice(smol_mixer_t* mixer, int handle) {
	mixer->voices[handle].state = SMOL_VOICE_STATE_PAUSED;
}

void smol_mixer_stop_voice(smol_mixer_t* mixer, int handle) {
	mixer->voices[handle].state = SMOL_VOICE_STATE_STOPPED;
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
				float sample = voice->callback(&mixer->voices[i], k, sample_rate, inv_sample_rate, voice->user_data);
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

#define smol_audio_set_mixer_as_callback(mixer) smol_audio_set_callback(&smol_mixer_mix, (void*)mixer)



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
	smol_audio_callback_proc* callback;
	void* callback_user_data;
} smol_audio_context_t;

smol_audio_context_t smol__audio_context;


EM_BOOL smol_audio_callback(
	int numInputs, 
	const AudioSampleFrame* inputs, 
	int numOutputs, 
	AudioSampleFrame* outputs, 
	int numParams, 
	const AudioParamFrame* params, 
	void* userData4
) {

	smol_audio_callback_data_t* cb_data = smol__audio_context.callback_data;

	if(!(cb_data && smol__audio_context.callback))
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

	smol__audio_context.callback(
		cb_data->num_input_channels, 
		128, 
		cb_data->input_channels, 
		cb_data->num_output_channels, 
		128, 
		cb_data->output_channels, 
		cb_data->sample_rate,
		cb_data->inv_sample_rate,
		smol__audio_context.callback_user_data
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


	int num_outputs[] = { smol__audio_context.num_output_channels };

	if(success) {


		EmscriptenAudioWorkletNodeCreateOptions options = {0};
		options.numberOfInputs = 0;
		options.numberOfOutputs = 1;
		options.outputChannelCounts = num_outputs;

		
		smol__audio_context.callback_data = (smol_audio_callback_data_t*)malloc(sizeof(smol_audio_callback_data_t));
		memset(smol__audio_context.callback_data, 0, sizeof(*smol__audio_context.callback_data));
		smol__audio_context.callback_data->sample_rate = (double)smol__audio_context.sample_rate;
		smol__audio_context.callback_data->inv_sample_rate = 1.0 / (double)smol__audio_context.sample_rate;
	

		EMSCRIPTEN_AUDIO_WORKLET_NODE_T worklet = emscripten_create_wasm_audio_worklet_node(
			context, 
			"smol_audio_worklet_processor", 
			&options, 
			smol_audio_callback, 
			NULL
		);

		smol__audio_context.worklet_node = worklet;

		  // Connect it to audio context destination
		EM_ASM({emscriptenGetAudioObject($0).connect(emscriptenGetAudioObject($1).destination)}, worklet, context);

	}

}


int smol_audio_init(int sample_rate, int num_channels) {
	
	EmscriptenWebAudioCreateAttributes context_desc = {0};
	context_desc.sampleRate = sample_rate;
	context_desc.latencyHint = "interactive";
	EMSCRIPTEN_WEBAUDIO_T context = emscripten_create_audio_context(&context_desc);
	smol__audio_context.audio_context = context; 
	smol__audio_context.num_output_channels = num_channels;
	smol__audio_context.sample_rate = sample_rate;
	
	emscripten_start_wasm_audio_worklet_thread_async(
		context, 
		audio_context_stack, 
		sizeof(audio_context_stack), 
		&audio_thread_initialized, 
		&context_desc
	);


}


int smol_audio_shutdown() {
	EM_ASM({emscriptenGetAudioObject($0).disconnect()}, smol__audio_context.worklet_node);
}
#endif 

#ifdef SMOL_PLATFORM_WINDOWS

#ifdef SMOL_AUDIO_BACKEND_WASAPI
typedef HRESULT smol_CoCreateInstance_proc(const IID* const rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, const IID* const riid, LPVOID* ppv);
typedef HRESULT smol_CoInitializeEx_proc(LPVOID pvReserved, DWORD  dwCoInit);
typedef void __stdcall smol_CoTaskMemFree_proc(_Frees_ptr_opt_ LPVOID pv);

smol_CoInitializeEx_proc* smol_CoInitializEx;
smol_CoCreateInstance_proc* smol_CoCreateInstance;
smol_CoTaskMemFree_proc* smol_CoTaskMemFree;

typedef struct smol_audio_context_t {
	IMMDevice* audio_device;
	IAudioClient* audio_client;
	IAudioRenderClient* audio_renderer;
	volatile UINT running;
	HANDLE processing_ended;
	HANDLE buffer_end;
	HANDLE audio_thread;
	union {
		WAVEFORMATEXTENSIBLE wave_format_ext;
		WAVEFORMATEX wave_format;
	};
	smol_audio_callback_proc* callback;
	void* callback_user_data;
} smol_audio_context_t;

#define SMOL_SAFE_COM_RELEASE(comptr) \
	if((comptr)) \
		(comptr)->lpVtbl->Release((comptr)), \
		comptr=NULL

DEFINE_GUID(SMOL_CLSID_MMDeviceEnumerator, 0xBCDE0395, 0xE52F, 0x467C, 0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E);
DEFINE_GUID(SMOL_IID_IMMDeviceEnumerator,  0xA95664D2, 0x9614, 0x4F35, 0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6);
DEFINE_GUID(SMOL_IID_IAudioClient,         0x1CB9AD4C, 0xDBFA, 0x4c32, 0xB1, 0x78, 0xC2, 0xF5, 0x68, 0xA7, 0x03, 0xB2);
DEFINE_GUID(SMOL_IID_IAudioRenderClient,   0xF294ACFC, 0x3146, 0x4483, 0xA7, 0xBF, 0xAD, 0xDC, 0xA7, 0xC2, 0x60, 0xE2);

smol_audio_context_t smol__audio_context;

DWORD WINAPI smol_audio_thread_proc(LPVOID lpParameter);

int smol_audio_init(int sample_rate, int num_channels) {

	HMODULE module = LoadLibrary(TEXT("Ole32.dll"));

	WAVEFORMATEXTENSIBLE result_format = { 0 };
	WAVEFORMATEX* wave_format = NULL;
	WAVEFORMATEX* final_format = NULL;

	smol_CoInitializEx = (smol_CoInitializeEx_proc*)GetProcAddress(module, "CoInitializeEx");
	smol_CoCreateInstance = (smol_CoCreateInstance_proc*)GetProcAddress(module, "CoCreateInstance");
	smol_CoTaskMemFree = (smol_CoTaskMemFree_proc*)GetProcAddress(module, "CoTaskMemFree");

	HRESULT hres = smol_CoInitializEx(NULL, COINIT_MULTITHREADED);

	if(FAILED(hres)) {
		fprintf(stderr, "FAILED to CoInitialize!\n");
		return 0;
	}



	IMMDeviceEnumerator* enumerator = NULL;
	hres = smol_CoCreateInstance(&SMOL_CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, &SMOL_IID_IMMDeviceEnumerator, (void**)&enumerator);
	if(FAILED(hres)) {
		fprintf(stderr, "FAILED to CoCreateInstance for device enumerator!");
		goto failure;
	}


	smol__audio_context.audio_device = NULL;
	hres = IMMDeviceEnumerator_GetDefaultAudioEndpoint(enumerator, eRender, eConsole, &smol__audio_context.audio_device);
	

	if(FAILED(hres)) {
		fprintf(stderr, "FAILED to Get default audio endpoint!");
		goto failure;
	}

	hres = IMMDeviceEnumerator_Release(enumerator);
	if(FAILED(hres)) {
		fprintf(stderr, "FAILED to release enumerator!");
		goto failure;
	}

	hres = IMMDevice_Activate(smol__audio_context.audio_device, &SMOL_IID_IAudioClient, CLSCTX_ALL, NULL, &smol__audio_context.audio_client);
	if(FAILED(hres)) {
		fprintf(stderr, "FAILED to activate audio device!");
		goto failure;
	}

	hres = IAudioClient_GetMixFormat(smol__audio_context.audio_client, &wave_format);
	if(FAILED(hres)) {
		fprintf(stderr, "FAILED to get audio renderer mix format!");
	}

	/*TODO: stuff?*/
	if(wave_format->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
			WAVEFORMATEXTENSIBLE* ext = (WAVEFORMATEXTENSIBLE*)wave_format;

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
		result_format.Format = *wave_format;
	}


	REFERENCE_TIME requested_duration = 10*100000LL; 

	hres = IAudioClient_IsFormatSupported(smol__audio_context.audio_client, AUDCLNT_SHAREMODE_SHARED, &result_format.Format, &final_format);
	if(FAILED(hres)) {
		fprintf(stderr, "FAILED in testing format support!");
		goto failure;
	}
	hres = IAudioClient_Initialize(smol__audio_context.audio_client, AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, requested_duration, 0, &result_format, NULL);

	if(FAILED(hres)) {
		fprintf(stderr, "FAILED to initialize audio renderer!");
		goto failure;
	}

	hres = IAudioClient_GetService(smol__audio_context.audio_client, &SMOL_IID_IAudioRenderClient, &smol__audio_context.audio_renderer);
	
	if(FAILED(hres)) {
		fprintf(stderr, "FAILED to acquire audio renderer client!");
		goto failure;
	}

	
	if(final_format) {
		if(final_format->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
			smol__audio_context.wave_format_ext = *(WAVEFORMATEXTENSIBLE*)final_format;
		else
			smol__audio_context.wave_format = *final_format;
		
	} else {
		if(result_format.Format.wFormatTag == WAVE_FORMAT_EXTENSIBLE)
			smol__audio_context.wave_format_ext = result_format;
		else 
			smol__audio_context.wave_format = result_format.Format;
	}	

	if(wave_format) smol_CoTaskMemFree((LPVOID)wave_format), wave_format = NULL;

	if(FAILED(hres)) {
		fprintf(stderr, "FAILED to start audio renderer!");
		goto failure;
	}

	smol__audio_context.buffer_end = CreateEvent(0, FALSE, FALSE, NULL);
	smol__audio_context.processing_ended = CreateEvent(0, FALSE, FALSE, NULL);

	hres = IAudioClient_SetEventHandle(smol__audio_context.audio_client, smol__audio_context.buffer_end);
	if(FAILED(hres)) {
		fprintf(stderr, "FAILED to set event handle for audio client!");
		goto failure;
	}

	smol__audio_context.audio_thread = CreateThread(NULL, 8192, &smol_audio_thread_proc, NULL, 0, NULL);
	return 1;

failure:

	if(wave_format) smol_CoTaskMemFree((LPVOID)wave_format);

	SMOL_SAFE_COM_RELEASE(smol__audio_context.audio_renderer);
	SMOL_SAFE_COM_RELEASE(smol__audio_context.audio_client);
	SMOL_SAFE_COM_RELEASE(smol__audio_context.audio_device);
	SMOL_SAFE_COM_RELEASE(enumerator);
	return 0;

}

DWORD WINAPI smol_audio_thread_proc(LPVOID lpParameter) {
	
	HRESULT result = S_OK;
	int processing = SUCCEEDED(IAudioClient_Start(smol__audio_context.audio_client));

	UINT32 buffer_padding = 0;
	BYTE* buffer_data = NULL;
	UINT32 buffer_frame_size = 0;
	result = IAudioClient_GetBufferSize(smol__audio_context.audio_client, &buffer_frame_size);

	float* mix_buffer = (float*)_aligned_malloc(buffer_frame_size*sizeof(float), 16);
	memset(mix_buffer, 0, buffer_frame_size*sizeof(float));
	float* channels[32];

	WORD num_channels = smol__audio_context.wave_format.nChannels;
	double sample_rate = smol__audio_context.wave_format.nSamplesPerSec;
	double inv_sample_rate = 1.0 / sample_rate;

	UINT chunk_size = buffer_frame_size / num_channels;
	for(UINT i = 0; i < num_channels; i++) {
		channels[i] = &mix_buffer[(i*chunk_size) >> 2];
	}


	while(WaitForSingleObject(smol__audio_context.processing_ended, 0) != WAIT_OBJECT_0) {

		result = IAudioClient_GetCurrentPadding(smol__audio_context.audio_client, &buffer_padding);
		if(FAILED(result))
			goto reset;
		
		UINT32 samples_to_write = (buffer_frame_size - buffer_padding) / num_channels;

		result = IAudioRenderClient_GetBuffer(smol__audio_context.audio_renderer, samples_to_write, &buffer_data);
		if(FAILED(result))
			goto reset;

		if(buffer_data) {


			if(smol__audio_context.wave_format.wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
				USHORT id = EXTRACT_WAVEFORMATEX_ID(&smol__audio_context.wave_format_ext.SubFormat);
				
				if(id == WAVE_FORMAT_IEEE_FLOAT) {
					for(int i = 0; i < samples_to_write; i++) {
						for(int j = 0; j < num_channels; j++) {
							channels[j][i] = 0.f;
						}
					}
					smol__audio_context.callback(
						0, 
						0, 
						NULL, 
						num_channels, 
						samples_to_write, 
						channels, 
						sample_rate, 
						inv_sample_rate, 
						smol__audio_context.callback_user_data
					);

					for(int i = 0; i < samples_to_write; i++) {
						for(int j = 0; j < num_channels; j++) {
							((float*)buffer_data)[i*num_channels+j] = channels[j][i];
						}
					}
				}

			}

			result = IAudioRenderClient_ReleaseBuffer(smol__audio_context.audio_renderer, samples_to_write, 0);
		}

		reset:

		if(SUCCEEDED(result) && !processing) {
			processing = SUCCEEDED(IAudioClient_Start(smol__audio_context.audio_client));
		}

		WaitForSingleObject(smol__audio_context.buffer_end, INFINITE);
		continue;
		
		/* Recovery here */


	}

	WaitForSingleObject(smol__audio_context.buffer_end, INFINITE);
	_aligned_free(mix_buffer);

	return 0;
}

int smol_audio_shutdown() {


	SetEvent(smol__audio_context.processing_ended);
	if(WaitForSingleObject(smol__audio_context.audio_thread, INFINITE) == WAIT_OBJECT_0) {

		SMOL_SAFE_COM_RELEASE(smol__audio_context.audio_renderer);
		SMOL_SAFE_COM_RELEASE(smol__audio_context.audio_client);
		SMOL_SAFE_COM_RELEASE(smol__audio_context.audio_device);

		return 1;
	}

	return 0;
}

#endif 
#undef COBJMACROS
#endif


int smol_audio_set_callback(smol_audio_callback_proc* callback, void* user_data) {
	smol__audio_context.callback = callback;
	smol__audio_context.callback_user_data = user_data;
	return 1;
}

#endif 
#endif 