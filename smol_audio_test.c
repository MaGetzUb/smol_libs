#define _CRT_SECURE_NO_WARNINGS
#pragma GCC diagnostic ignored "-Wmissing-braces"
#include <math.h>
#include <stdio.h>

#define SMOL_UTILS_IMPLEMENTATION
#include "smol_utils.h"

#define SMOL_AUDIO_IMPLEMENTATION
#include "smol_audio.h"

#define SMOL_FRAME_IMPLEMENTATION
#include "smol_frame.h"

#define SMOL_CANVAS_IMPLEMENTATION
#include "smol_canvas.h"

#define SMOL_INPUT_IMPLEMENTATION
#include "smol_input.h"


#include "smol_font_16x16.h"

smol_mixer_t mixer = { 0 };

float samples[2][4096];
int num_samples;

//double phase = 0.;

void draw_audio(
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
	num_samples = num_output_samples;
	for(int j = 0; j < num_output_samples; j++) {
		samples[0][j] = outputs[0][j];
		samples[1][j] = outputs[1][j];
	}
}


float smol_floorf(float f) {
	int i = (int)f;
	if(f > i) return i - 1;
	return i;
}

float smol_ceilf(float f) {
	int i = (int)f;
	if(f < i) return i + 1;
	return i;
}

float smol_roundf(float f) {
	return smol_floorf(f + .5f);
}

double smol_floord(double f) {
	long long i = (long long)f;
	if(f > i) return i - 1;
	return i;
}

double smol_ceild(double f) {
	long long i = (long)f;
	if(f < i) return i + 1;
	return i;
}

double smol_roundd(double f) {
	return smol_floord(f + .5);
}



SMOL_INLINE unsigned long long byte_swap64(unsigned long long u64) {
	
	unsigned long long res = (
		(u64 & 0xFF00000000000000ULL) >> 0x38 | 
		(u64 & 0x00FF000000000000ULL) >> 0x28 | 
		(u64 & 0x0000FF0000000000ULL) >> 0x18 | 
		(u64 & 0x000000FF00000000ULL) >> 0x08 | 
		(u64 & 0x00000000FF000000ULL) << 0x08 | 
		(u64 & 0x0000000000FF0000ULL) << 0x18 | 
		(u64 & 0x000000000000FF00ULL) << 0x28 |
		(u64 & 0x00000000000000FFULL) << 0x38
	);
	
	return res;

}

SMOL_INLINE unsigned int byte_swap32(unsigned int u32) {
	
	unsigned int res = (
		(u32 & 0xFF000000) >> 0x18 | 
		(u32 & 0x00FF0000) >> 0x08 | 
		(u32 & 0x0000FF00) << 0x08 | 
		(u32 & 0x000000FF) << 0x18
	);
	
	return res;
}

SMOL_INLINE unsigned short byte_swap16(unsigned short u16) {
	
	unsigned short res = (
		((u16) & 0xFF00) >> 8 | 
		((u16) & 0x00FF) << 8
	);
	
	return res;

}

typedef struct synth_t {
	double freq;
	double decay;
	double phase_offset[2];
} synth_t;

float synth_callback(smol_mixer_t* mixer, int voice, int channel, double sample_rate, double inv_sample_rate, void* user_data) {
	synth_t* synth = (synth_t*)user_data;
	float sample = 0.f;
		
	//sample += cosf(voice->time_offset * 2.f * 6.28318 * synth->freq + synth->phase_offset[channel]);
	double stime = mixer->voices[voice].time_offset * synth->freq + synth->phase_offset[channel];
	long long itime = stime;
	double ftime = stime - itime;

	sample += sinf(mixer->voices[voice].time_offset * 6.28318 * synth->freq  + synth->phase_offset[channel]);
	
	////Saw:
	//sample += (ftime*2.f - 1.f);
	//
	////Square:
	//sample += ((ftime < 0.5f) * 2.f - 1.f);
	//
	////Triangle:
	//sample += 1.f - fabsf(ftime-0.5)*4.f;

	sample *= 1.f;
	float d = mixer->voices[voice].time_offset / synth->decay;
	float decay = exp(-d);
	if(decay < 1e-3)
		smol_mixer_stop_voice(mixer, voice);
	return sample*decay;
}

float note_to_frequency(int note) {
	float freq = 55.f * powf(2, (float)note / 12.f);;
	return freq;
}

volatile int filter = 1; //Linear
static float(*sampler[])(smol_audiobuffer_t*, int, double) = {
	smol_audiobuffer_sample_nearest,
	smol_audiobuffer_sample_linear,
	smol_audiobuffer_sample_cubic
};

float sample_audiobuffer(smol_mixer_t* mixer, int voice, int channel, double sample_rate, double inv_sample_rate, void* user_data) {
	smol_audiobuffer_t* smol_audiobuffer_data = (smol_audiobuffer_t*)user_data;
	
	double time_offset = mixer->voices[voice].time_offset;
	
	if(time_offset > smol_audiobuffer_data->duration)
		mixer->voices[voice].time_offset = 0.; //Loop like there's no end
	if(time_offset < 0.)
		mixer->voices[voice].time_offset = smol_audiobuffer_data->duration;
		//voice->state = SMOL_VOICE_STATE_STOPPED;

	//long long sample_index = (long long)(voice->time_offset * smol_audiobuffer_data->sample_rate);
	//return smol_audiobuffer_data->samples[sample_index*smol_audiobuffer_data->num_channels+channel];
	return sampler[filter](smol_audiobuffer_data, channel, time_offset)*1.f;
}


int main() {
	
	smol_font_t font = {
		.glyphs = (char*)PXF_SMOL_FONT_16X16_DATA,
		.glyph_width = PXF_SMOL_FONT_16X16_WIDTH,
		.glyph_height = PXF_SMOL_FONT_16X16_HEIGHT,
		.geometry = (smol_font_hor_geometry_t*)PXF_SMOL_FONT_16X16_OFFSET_X_WIDTH
	};
	
	int A_4 = 36;
	int Bb4 = 37;
	int B_4 = 38;
	int C_4 = 39;
	int Db4 = 40;
	int D_4 = 41;
	int Eb4 = 42;
	int E_4 = 43;
	int F_4 = 44;
	int Gb4 = 45;
	int G_4 = 46;
	int Ab4 = 47;

	float notes[] = { 
		note_to_frequency(A_4), 
		note_to_frequency(B_4), 
		note_to_frequency(C_4), 
		note_to_frequency(E_4), 
		note_to_frequency(F_4),
	};

	const char* note_names[] = {
		"A4",
		"B4",
		"C4",
		"E4",
		"F4"
	};


	smol_canvas_t canvas;
	canvas = smol_canvas_create(800, 600);
	smol_frame_t* frame = smol_frame_create(800, 600, "audio?");

	mixer.post_mix_user_data = NULL;
	mixer.post_mix_callback = &draw_audio;


	smol_audio_init(48000, 2);
	smol_audio_set_mixer_as_callback(&mixer);
	//smol_audio_set_callback(draw_audio, NULL);

	
	smol_audiobuffer_t buffer = smol_create_audiobuffer_from_wav_file(
		//"test.wav"
		"G:/Dev/FulcrumEngine/FFT-Test/CybermorphicSymphony.wav"
	);
	smol_audiobuffer_t buffer2 = smol_create_audiobuffer_from_qoa_file("res/music/lorn_drawn_out_like_an_ache.qoa");

	smol_audiobuffer_save_wav(&buffer2, "test.wav", 8);

	int voice = smol_mixer_play_voice(&mixer, sample_audiobuffer, &buffer, 1.f, (float[3]) { 0.f, 0.f, 0.f });

	synth_t synth[64] = {0.};
	synth[0].freq = 440.f;
	synth[0].decay = 1.f;
	double target_rate = 1.;

	while(!smol_frame_is_closed(frame)) {
		smol_frame_update(frame);
		smol_mixer_update(&mixer);
		smol_inputs_flush();

		for(smol_frame_event_t ev; smol_frame_acquire_event(frame, &ev);) {
			smol_inputs_update(&ev);
		}

		smol_canvas_clear(&canvas, SMOLC_DARKER_BLUE);

		if(smol_mouse_hit(1)) {
			int next_index = smol_mixer_next_free_handle(&mixer);
			if(next_index != 0xFFFFFFFF) {
				int idx = smol_rnd(0, 5);
				synth[next_index].freq = notes[idx];//note_to_frequency(smol_rnd(31, 61));//smol_rndf(110.f, 880.f);
				synth[next_index].decay = smol_rndf(0.25f, 1.f);
				synth[next_index].phase_offset[0] = smol_rndf(-1.f, 1.f)*3.14159f;
				synth[next_index].phase_offset[1] = smol_rndf(-1.f, 1.f)*3.14159f;
				smol_mixer_play_voice(&mixer, synth_callback, &synth[next_index], 0.125f, (float[3]) { -1.f + (float)smol_rnd(0, 200)/99.f, 0.f, 0.f });
			}
		}

		{

			for(int j = 0; j < num_samples; j++) {
				if(j > 1) {

					smol_canvas_set_color(&canvas, SMOLC_ORANGE);
					smol_canvas_draw_line(&canvas, (j - 1) * 800 / num_samples, 300 + samples[0][j - 1] * 300.f, j * 800 / num_samples, 300 + samples[0][j] * 300.f);

					smol_canvas_set_color(&canvas, SMOLC_SKYBLUE);
					smol_canvas_draw_line(&canvas, (j - 1) * 800 / num_samples, 300 + samples[1][j - 1] * 300.f, j * 800 / num_samples, 300 + samples[1][j] * 300.f);
			
				}
			}

			if(smol_mouse_down(2)) {
				 target_rate = (((double)smol_mouse_x() / 800.) * 4.f)-2.f;
			}

			if(smol_key_hit(SMOLK_SPACE)) {
				target_rate = 1.f;
			}

			mixer.voices[voice].time_scale += (target_rate - mixer.voices[voice].time_scale) * 0.125;

			static int offset = 0;
			if(smol_key_hit(SMOLK_NUM1)) filter = 0;
			if(smol_key_hit(SMOLK_NUM2)) filter = 1;
			if(smol_key_hit(SMOLK_NUM3)) filter = 2;
			if(smol_key_down(SMOLK_RIGHT)) offset += 10;
			if(smol_key_down(SMOLK_LEFT)) offset -= 10;

			char buf[128] = { 0 };
			int num = smol_mixer_playing_voice_count(&mixer);
			smol_canvas_set_color(&canvas, SMOLC_WHITE);
			
			smol_canvas_draw_text(&canvas, 10, 10, font, 1, "Click with mouse to play a synthesized sound.");

			sprintf(buf, "Voice count: %d", num);
			smol_canvas_draw_text(&canvas, 10, 26, font, 1, buf);

			for(int i = 63; i >= 0; i--)
				buf[i] = "01"[(mixer.active_voices_mask >> i) & 1];
			smol_canvas_draw_text(&canvas, 10, 42, font, 1, buf);

			sprintf(buf, "Sampler: %s (Change with keys 1, 2, 3)", (const char* []) { "Nearest", "Linear", "Cubic Hermite" }[filter]);
			smol_canvas_draw_text(&canvas, 10, 58, font, 1, buf);

			sprintf(buf, "Playback samplerate: %lf", mixer.voices[voice].time_scale * 48000.);
			smol_canvas_draw_text(&canvas, 10, 74, font, 1, buf);


		}
		smol_canvas_present(&canvas, frame);
	}
	smol_audiobuffer_destroy(&buffer);
	smol_audio_shutdown();
	smol_frame_destroy(frame);

	return 0;
}
