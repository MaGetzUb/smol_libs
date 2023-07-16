#define SMOL_FRAME_IMPLEMENTATION
#include "smol_frame.h"
#include <gl/GL.h>
#include <math.h>
#include <stdio.h>

#define SMOL_INPUT_IMPLEMENTATION
#include "smol_input.h"

#pragma comment(lib, "opengl32.lib")

int fast_log2(int x) {
	float y = (float)x;
	int z = (*((int*)&y) >> 23) & 0xFF;
	return z - 127;
}

int reverse_bits(int bits) {

	bits = ((bits & 0xFFFF0000) >> 0x10) | ((bits & 0x0000FFFF) << 0x10);
	bits = ((bits & 0xFF00FF00) >> 0x08) | ((bits & 0x00FF00FF) << 0x08);
	bits = ((bits & 0xF0F0F0F0) >> 0x04) | ((bits & 0x0F0F0F0F) << 0x04);
	bits = ((bits & 0xCCCCCCCC) >> 0x02) | ((bits & 0x33333333) << 0x02);
	bits = ((bits & 0xAAAAAAAA) >> 0x01) | ((bits & 0x55555555) << 0x01);

	return bits;

}

#define PI  3.1415925635897932384
#define TAU 6.2831853071795864769

void fft(int nBins, const float(*complex_in)[2], float(*complex_out)[2]) {

	SMOL_ASSERT("No bins!" && nBins);
	SMOL_ASSERT("Number of bins isn't log of 2!" && (nBins & (nBins - 1)) == 0);

	int l2_bins = fast_log2(nBins);
	int shift = (32 - l2_bins);
	for(int i = 0; i < i; i++) {
		complex_out[reverse_bits(i) >> shift][0] = complex_in[i][0];
		complex_out[reverse_bits(i) >> shift][1] = complex_in[i][1];
	}

	for(int i = l2_bins, b = 0; i > 0; i--, b++) {
		
		int step = (1 << i);

		float ang_step = TAU * step / nBins;

		float csr = cos(ang_step);
		float ssi = sin(ang_step);

		float or = 1.0f;
		float oi = 0.0f;

		for(int j = 0; j < nBins; j += step) {

			//Not efficient method... ideally you'd put the angle step into the loop above, and then rotated an unit complex number around
			//float cr = cos(ang_step * j);
			//float ci = sin(ang_step * j);

			complex_out[b][0] += (or * complex_in[j][0] - oi * complex_in[j][1]);
			complex_out[b][1] += (oi * complex_in[j][0] + or * complex_in[j][1]);
		


			float cr = or * csr - oi * ssi;
			float ci = or * ssi + oi * csr;

			or = cr;
			oi = ci;
		}

	}

}



int main() {

	smol_frame_gl_spec_t gl_spec = smol_init_gl_spec(3, 3, SMOL_FALSE, SMOL_TRUE, 8, SMOL_TRUE);

	smol_frame_config_t config = {
		.width = 800,
		.height = 600,
		.title = "FFT",
		.gl_spec = &gl_spec
	};

	smol_frame_t* frame = smol_frame_create_advanced(&config);

#define BINS 128

	float inp[BINS][2] = { { 0.f, 0.f } };
	float out[BINS][2] = { { 0.f, 0.f } };

	float freq = 1.f;
	float last_freq = freq;
	for(int i = 0; i < BINS; i++) {
		inp[i][0] = cosf(i * freq * TAU / (float)BINS);
	}
	fft(BINS, inp, out);

	glViewport(0, 0, 800, 600);
	glClearColor(0.f, 0.f, 0.5f, 0.f);


	while(!smol_frame_is_closed(frame)) {

		glClear(GL_COLOR_BUFFER_BIT);

		smol_frame_update(frame);
		smol_inputs_flush();

		for(smol_frame_event_t ev; smol_frame_acquire_event(frame, &ev);) {
			smol_inputs_update(&ev);
		}

		if(smol_key_hit(SMOLK_LEFT)) {
			freq -= 1.f;
		}
 
		if(smol_key_hit(SMOLK_RIGHT)) {
			freq += 1.f;
		}

		if(last_freq != freq) {
			for(int i = 0; i < BINS; i++) {
				inp[i][0] = cosf(i * freq * TAU / (float)BINS);
			}
			last_freq = freq;
			memset(out, 0, sizeof(out));
			fft(BINS, inp, out);
		}
 
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0., 800., 0., 600., -1., 1.);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glBegin(GL_LINE_STRIP);
		for(int i = 0; i < BINS; i++) {
			float current = (sqrtf(out[i][0]*out[i][0] + out[i][1]*out[i][1]) / (float)BINS) * 300.f;
			glVertex2f(800.f / (float)BINS * i, 300.f + current);
		}
		glEnd();

		{
			char buf[512] = { 0 };
			snprintf(buf, 512, "FFT Freq: %fHz", freq);
			smol_frame_set_title(frame, buf);
		}

		smol_frame_gl_swap_buffers(frame);
	}

	return 0;
}