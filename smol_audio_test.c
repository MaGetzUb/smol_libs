#define _CRT_SECURE_NO_WARNINGS

#define SMOL_FRAME_IMPLEMENTATION
#include "smol_frame.h"

#define SMOL_CANVAS_IMPLEMENTATION
#include "smol_canvas.h"

#define SMOL_INPUT_IMPLEMENTATION
#include "smol_input.h"

typedef void audio_callback_proc(uint32_t channel, float* buffer,uint32_t numFrames);

EMSCRIPTEN_KEEPALIVE
audio_callback_proc smol__audio_callback;

#ifdef __EMSCRIPTEN__

EMSCRIPTEN_KEEPALIVE
void smol_audio_exec_callback(uint32_t channel, float* buffer, uint32_t numFrames) {
	if(smol__audio_callback)
		smol__audio_callback(channel, buffer, numFrames);
}

EMSCRIPTEN_KEEPALIVE
void worklet_message() {
	puts("poggers");
}

EM_ASYNC_JS(void, set_audio_callback, (audio_callback_proc proc), {

	if(Module["audio_context"] === undefined) {

		const audioCtx = new AudioContext();
		console.log("Audio context created.");
		Module.audio_callback = Module.cwrap('smol_audio_exec_callback', 'void', ['number', 'number', 'number']);

		const audioWorkletCode = `
			class smol_audio_processor extends AudioWorkletProcessor {


			constructor(options) {
				super();
				
				this.callback = options.callback;
				this.sampleRate = options.sampleRate;
				this.invSampleRate = 1.0 / this.sampleRate;

				console.log(callBack);
				console.log(sampleRate);
				console.log(invSampleRate);

			}

			process(inputs, outputs, parameters) {

				for(var i = 0; i < outputs.length; i++) {
					this.callback(i, outputs[i].byteOffset, outputs[i].length);
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
			await audioCtx.audioWorklet.addModule(await new Promise((res) = > {
				reader.onloadend = function() {
					res(reader.result);
				}
			})).then(() => {

				let audio_processor = new AudioWorkletNode(audioContext, 'smol_audio_worklet_processor', {}
					processorOptions: {
						customData: { callback: audio_callback, sampleRate: audioCtx.sampleRate }
					}
				);
				

				source.buffer = audioBuffer;
				source.connect(audio_processor).connect(audioCtx.destination);
				source.start();

				audio_processor.port.onmessage = (event) => {
					// Handling data from the processor.
					console.log(event.data);
				};

				audio_processor.port.postMessage('Hello!');

			});
		}
		catch(error) {
			console.log("Audio worklet creation failed! " + error);
		}
		
		Module.audio_context = audioCtx;
		smol__audio_callback = proc;
	}

});

#endif 

void smol__audio_callback();

int main() {

	smol_frame_t* frame = smol_frame_create(800, 600, "audio?");

	smol_canvas_t canvas = smol_canvas_create(800, 600);

	set_audio_callback(NULL);

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