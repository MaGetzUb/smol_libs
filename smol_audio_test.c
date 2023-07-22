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

#if 1
smol_canvas_t canvas;

void audio_callback(
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

	static float phase = 0.f;
	smol_canvas_clear(&canvas, SMOLC_DARK_BLUE);

	for(int j = 0; j < num_output_samples; j++) {
		for(int i = 0; i < num_output_channels; i++) {
			outputs[i][j] = sinf((phase + ((float)i) * 3.14159f * 0.5f) * 440.f) * 0.0625f;
		}

		if(j > 1) {
			smol_canvas_set_color(&canvas, SMOLC_ORANGE);
			smol_canvas_draw_line(&canvas, (j - 1) * 800 / num_output_samples, 300 + outputs[0][j - 1] * 300.f, j * 800 / num_output_samples, 300 + outputs[0][j - 1] * 300.f);

			smol_canvas_set_color(&canvas, SMOLC_SKYBLUE);
			smol_canvas_draw_line(&canvas, (j - 1) * 800 / num_output_samples, 300 + outputs[1][j - 1] * 300.f, j * 800 / num_output_samples, 300 + outputs[1][j - 1] * 300.f);
			
		}
		phase += (inv_sample_rate * 6.283185);
		phase = fmodf(phase, 1.f);
	}
}

int main() {
	
	canvas = smol_canvas_create(800, 600);

	smol_audio_init(48000, 2);
	smol_audio_set_callback(&audio_callback, NULL);

	smol_frame_t* frame = smol_frame_create(800, 600, "audio?");


	printf("Audio context created.\n");



	while(!smol_frame_is_closed(frame)) {
		smol_frame_update(frame);

		smol_inputs_flush();

		for(smol_frame_event_t ev; smol_frame_acquire_event(frame, &ev);) {
			smol_inputs_update(&ev);
		}

		

		smol_canvas_present(&canvas, frame);
	}

	smol_audio_shutdown();
	smol_frame_destroy(frame);

	return 0;
}


#else 

typedef void(*audio_callback_proc)(smol_u32 channel, float* buffer, smol_u32 num_frames);

EMSCRIPTEN_KEEPALIVE
audio_callback_proc smol__audio_callback;

#ifdef __EMSCRIPTEN__

EMSCRIPTEN_KEEPALIVE
void smol_audio_exec_callback(uint32_t channel, float* buffer, uint32_t numFrames) {
	if(smol__audio_callback)
		smol__audio_callback(channel, buffer, numFrames);
}

EMSCRIPTEN_KEEPALIVE
void smol_audio_set_callback(audio_callback_proc callback) {
	smol__audio_callback = callback;
}



#endif 

void callback(smol_u32 channel, float* buffer, smol_u32 num_frames) {
	for(smol_u32 i = 0; i < num_frames; i++) {
		buffer[i] = -0.5f + (float)i / num_frames;
	}
}

typedef struct audio_t {
	double invSampleRate;
	double phase[2];
} audio_t;

smol_u8 thread_stack[8192];
audio_t audio_data = { 0 };

EM_BOOL audio_callback(
	int numInputs, 
	const AudioSampleFrame* inputs, 
	int numOutputs, 
	AudioSampleFrame* outputs, 
	int numParams, 
	const AudioParamFrame* params, 
	void* userData4
) {

	audio_t* audio = (audio_t*)userData4;

	for(int i = 0; i < numOutputs; i++) {
		for(int j = 0; j < outputs[i].numberOfChannels; j++) {
			for(int k = 0; k < 128; k++) {
				outputs[i].data[k+128*j] = sinf(audio->phase[j]*220.f) * 0.0625;
				audio->phase[j] += 6.28315 * audio->invSampleRate;
			}
		}
	}
	return EM_TRUE;
}

void audio_worklet_created(EMSCRIPTEN_WEBAUDIO_T audioContext, EM_BOOL success, void* userData3);

void audio_thread_initialized(EMSCRIPTEN_WEBAUDIO_T context, EM_BOOL success, void *userData)
{
	if (!success) return; // Check browser console in a debug build for detailed errors
	{

		
		printf("Audio thread initialized.\n");

		EmscriptenWebAudioCreateAttributes* context_desc = (EmscriptenWebAudioCreateAttributes*)userData;

		WebAudioWorkletProcessorCreateOptions options = { 0 };
		options.name = "smol_audio_worklet_processor";
		options.numAudioParams = 0;
		options.audioParamDescriptors = NULL;


		emscripten_create_wasm_audio_worklet_processor_async(context, &options, &audio_worklet_created, &audio_data);
	}
}

void audio_worklet_created(EMSCRIPTEN_WEBAUDIO_T context, EM_BOOL success, void *userData3) {


	int num_ouputs[] = { 2 };

	if(success) {

		
		printf("Audio worklet created.\n");

		EmscriptenAudioWorkletNodeCreateOptions options = {0};
		options.numberOfInputs = 0;
		options.numberOfOutputs = 1;
		options.outputChannelCounts = num_ouputs;

		EMSCRIPTEN_AUDIO_WORKLET_NODE_T worklet = emscripten_create_wasm_audio_worklet_node(context, "smol_audio_worklet_processor", &options, audio_callback, &audio_data);

		  // Connect it to audio context destination
		EM_ASM({emscriptenGetAudioObject($0).connect(emscriptenGetAudioObject($1).destination)},  worklet, context);

		
		printf("Audio worklet connected.\n");

	}

}


int main() {

	//smol_audio_init(2);
	//smol_audio_set_callback(&callback);

	smol_frame_t* frame = smol_frame_create(800, 600, "audio?");

	smol_canvas_t canvas = smol_canvas_create(800, 600);


	EmscriptenWebAudioCreateAttributes context_desc = {0};
	context_desc.sampleRate = 48000;
	context_desc.latencyHint = "interactive";
	EMSCRIPTEN_WEBAUDIO_T context = emscripten_create_audio_context(&context_desc);

	printf("Audio context created.\n");

	audio_data.invSampleRate = 1.0 / (double)context_desc.sampleRate;
	
	emscripten_start_wasm_audio_worklet_thread_async(context, thread_stack, sizeof(thread_stack), &audio_thread_initialized, &context_desc);




	while(!smol_frame_is_closed(frame)) {
		smol_frame_update(frame);

		smol_inputs_flush();

		for(smol_frame_event_t ev; smol_frame_acquire_event(frame, &ev);) {
			smol_inputs_update(&ev);
		}

		smol_canvas_clear(&canvas, SMOLC_DARK_BLUE);

		smol_canvas_present(&canvas, frame);
	}


	return 0;
}



/*
EM_ASYNC_JS(void, smol_audio_init, (int num_channels), {	
	if(Module["audio_context"] === undefined) {


		const audioCtx = new AudioContext({ latency: "interactive", });
		
		Module.audio_context = audioCtx;

		console.log("Audio context created.");
		Module.audio_callback = function(channel, buffer, frames) {
			Module.ccall('smol_audio_exec_callback', 'void', ['number', 'number', 'number'], channel, buffer, frames);
		};
		
		console.log(Module.audio_callback);

		const audioWorkletCode = `

			import Module from '${ new URL(import.meta.url, location.href) }';

			class smol_audio_processor extends AudioWorkletProcessor {


			constructor(options) {
				super();
				
				console.log(options);
				
				this.sampleRate = options.processorOptions.customData.sampleRate;
				this.invSampleRate = 1.0 / this.sampleRate;

				this.phase = [0.0, Math.PI/4];

				console.log(this.sampleRate);
				console.log(this.invSampleRate);

				console.log(Module.call);
				console.log(Module._malloc);

			}

			process(inputs, outputs, parameters) {

				for(var i = 0; i < outputs[0].length; i++) {
					//Module.audio_callback(i, outputs[i].byteOffset, outputs[i].length);
					for(var j = 0; j < outputs[0][i].length; j++) {
						outputs[0][i][j] = Math.sin(this.phase[i]*220.0) * 0.0625;
						this.phase[i] += this.invSampleRate * 2.0 * 3.141592653589;
					}
				}

				return true;
			}
		}

		registerProcessor('smol_audio_worklet_processor', smol_audio_processor);
		console.log("smol_audio_worklet_processor registered.")
		`;
		let blob = new Blob([audioWorkletCode], { type: 'application/javascript' });

		let reader = new FileReader();
		reader.readAsDataURL(blob);
		let source = audioCtx.createBufferSource();

		try {
			var url = await new Promise((res) => {
				reader.onloadend = function() {
					res(reader.result);
				}
			});
			console.log(url);
			await audioCtx.audioWorklet.addModule(url).then(() => {

				let audio_processor = new AudioWorkletNode(audioCtx, 'smol_audio_worklet_processor', {
						outputChannelCount: [ num_channels ],
						numberOfOutputs: 1, 
						processorOptions: {
							customData: { 
								sampleRate: audioCtx.sampleRate 
							}
						}
					}
				);
				
				audio_processor.connect(audio_processor).connect(audioCtx.destination);
				//audio_processor.start();

				audio_processor.port.onmessage = event => {
					// Handling data from the processor.
					console.log(event.data);
				};

				audio_processor.port.postMessage('Hello!');

			});
		}
		catch(error) {
			console.log("Audio worklet creation failed! " + error);
		}
		
	}

});
*/
#endif 