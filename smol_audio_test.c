#define _CRT_SECURE_NO_WARNINGS

#include <math.h>
#define SMOL_AUDIO_IMPLEMENTATION
#include "smol_audio.h"

#define SMOL_FRAME_IMPLEMENTATION
#include "smol_frame.h"

#define SMOL_CANVAS_IMPLEMENTATION
#include "smol_canvas.h"

#define SMOL_INPUT_IMPLEMENTATION
#include "smol_input.h"

#define SMOL_UTILS_IMPLEMENTATION
#include "smol_utils.h"

#include "smol_font_16x16.h"

smol_canvas_t canvas;
smol_mixer_t mixer = { 0 };

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

	for(int j = 0; j < num_output_samples; j++) {
	//	for(int i = 0; i < num_output_channels; i++)
	//		outputs[i][j] = sinf(phase * 440. * 6.28318) * 0.0625;

	//	phase += inv_sample_rate;

		if(j > 1) {

			smol_canvas_set_color(&canvas, SMOLC_ORANGE);
			smol_canvas_draw_line(&canvas, (j - 1) * 800 / num_output_samples, 300 + outputs[0][j - 1] * 300.f, j * 800 / num_output_samples, 300 + outputs[0][j] * 300.f);

			smol_canvas_set_color(&canvas, SMOLC_SKYBLUE);
			smol_canvas_draw_line(&canvas, (j - 1) * 800 / num_output_samples, 300 + outputs[1][j - 1] * 300.f, j * 800 / num_output_samples, 300 + outputs[1][j] * 300.f);
			
		}
	}
}


typedef struct synth_t {
	double freq;
	double decay;
	double phase_offset[2];
} synth_t;

float saw_synth_callback(smol_voice_t* voice, int channel, double sample_rate, double inv_sample_rate, void* user_data) {
	synth_t* synth = (synth_t*)user_data;
	float sample = sinf(voice->time_offset * 6.28318 * synth->freq  + synth->phase_offset[channel]);
	//sample += cosf(voice->time_offset * 2.f * 6.28318 * synth->freq + synth->phase_offset[channel]);
	double stime = voice->time_offset * synth->freq + synth->phase_offset[channel];
	long long itime = stime;
	double ftime = stime - itime;
	//Saw:
	//float sample = ftime*2.f-1.f;
	
	//Square:
	//float sample = ftime < 0.5f ? -1.f : 1.f;
	
	//Triangle:
	//float sample = 1.f - fabsf(ftime-0.5)*4.f;
	float decay = exp(-voice->time_offset/synth->decay);
	if(decay < 1e-3)
		voice->state = SMOL_VOICE_STATE_STOPPED;
	return sample*decay;
}

float note_to_frequency(int note) {
	return 55 * powf(2, (float)note / 12);
}

int main() {
	
	smol_font_t font = {
		.glyphs = (char*)PXF_SMOL_FONT_16X16_DATA,
		.glyph_width = PXF_SMOL_FONT_16X16_WIDTH,
		.glyph_height = PXF_SMOL_FONT_16X16_HEIGHT,
		.geometry = (smol_font_hor_geometry_t*)PXF_SMOL_FONT_16X16_OFFSET_X_WIDTH
	};

	canvas = smol_canvas_create(800, 600);

	mixer.post_mix_user_data = NULL;
	mixer.post_mix_callback = &draw_audio;

	smol_audio_init(48000, 2);
	smol_audio_set_mixer_as_callback(&mixer);
	//smol_audio_set_callback(draw_audio, NULL);

	synth_t synth[64] = {0.};
	synth[0].freq = 440.f;
	synth[0].decay = 1.f;

	//smol_mixer_play_voice(&mixer, saw_synth_callback, &synth, 0.0625, (float[3]) { 0.f, 0.f, 0.f });
	smol_frame_t* frame = smol_frame_create(800, 600, "audio?");


	while(!smol_frame_is_closed(frame)) {
		smol_frame_update(frame);
		smol_mixer_update(&mixer);
		smol_inputs_flush();

		for(smol_frame_event_t ev; smol_frame_acquire_event(frame, &ev);) {
			smol_inputs_update(&ev);
		}

		if(smol_mouse_hit(1)) {
			int next_index = smol_mixer_next_free_handle(&mixer);
			if(next_index != 0xFFFFFFFF) {
				synth[next_index].freq = note_to_frequency(smol_rnd(31, 61));//smol_rndf(110.f, 880.f);
				synth[next_index].decay = smol_rndf(0.25f, 1.f);
				synth[next_index].phase_offset[0] = smol_rndf(-1.f, 1.f)*3.14159f;
				synth[next_index].phase_offset[1] = smol_rndf(-1.f, 1.f)*3.14159f;
				smol_mixer_play_voice(&mixer, saw_synth_callback, &synth[next_index], 0.0625f, (float[3]) { 0.f, 0.f, 0.f });
			}
		}

		smol_canvas_clear(&canvas, SMOLC_DARKER_BLUE);
		{
			char buf[128] = { 0 };
			int num = smol_mixer_playing_voice_count(&mixer);
			smol_canvas_set_color(&canvas, SMOLC_WHITE);
			
			smol_canvas_draw_text(&canvas, 10, 10, font, 1, "Click with mouse to play a synthesized sound.");

			sprintf(buf, "Voice count: %d", num);
			smol_canvas_draw_text(&canvas, 10, 26, font, 1, buf);

			for(int i = 63; i >= 0; i--)
				buf[i] = "01"[(mixer.active_voices_mask >> i) & 1];
			smol_canvas_draw_text(&canvas, 10, 42, font, 1, buf);


		}
		smol_canvas_present(&canvas, frame);
	}

	smol_audio_shutdown();
	smol_frame_destroy(frame);

	return 0;
}
