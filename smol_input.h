/*
Copyright © 2023 Marko Ranta (Discord: Coderunner#2271)

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

#ifndef SMOL_INPUTS_H
#define SMOL_INPUTS_H

//Prevent implementation duplication
#ifndef SMOL_FRAME_H
#include "smol_frame.h"
#endif 

typedef enum {
	SMOL_INPUT_STATE_DOWN = 1,
	SMOL_INPUT_STATE_CHANGE = 2,
	SMOL_INPUT_STATE_UP = 3
} smol_input_state;


void smol_inputs_flush();
void smol_inputs_update(smol_frame_event_t* event);

int smol_key_hit(smol_key key);
int smol_key_up(smol_key key);
int smol_key_down(smol_key key);

int smol_mouse_hit(int button);
int smol_mouse_up(int button);
int smol_mouse_down(int button);

int smol_mouse_x();
int smol_mouse_y();
int smol_mouse_z();
int smol_mouse_w();

int smol_mouse_move_x();
int smol_mouse_move_y();
int smol_mouse_move_z();
int smol_mouse_move_w();

#endif 

#ifdef SMOL_INPUT_IMPLEMENTATION

unsigned char smol__key_states[256];
unsigned char smol__mouse_button_states[5];
int smol__mouse_x;
int smol__mouse_y;
int smol__mouse_z;
int smol__mouse_w;

int smol__mouse_move_x;
int smol__mouse_move_y;
int smol__mouse_move_z;
int smol__mouse_move_w;


void smol_inputs_flush() {
	for(int i = 0; i < 5; i++) {
		smol__mouse_button_states[i] &= SMOL_INPUT_STATE_DOWN;
	}
	for(int i = 0; i < 256; i++) {
		smol__key_states[i] &= SMOL_INPUT_STATE_DOWN;
	}
}

void smol_inputs_update(smol_frame_event_t* event) {

	switch(event->type) {
		case SMOL_FRAME_EVENT_KEY_DOWN:
		case SMOL_FRAME_EVENT_KEY_UP:
			if((smol__key_states[event->key.code] & SMOL_INPUT_STATE_DOWN) != (event->type == SMOL_FRAME_EVENT_KEY_DOWN)) {
				smol__key_states[event->key.code] ^= SMOL_INPUT_STATE_DOWN;
				smol__key_states[event->key.code] |= SMOL_INPUT_STATE_CHANGE;
			}
		break;
		case SMOL_FRAME_EVENT_MOUSE_BUTTON_DOWN:
		case SMOL_FRAME_EVENT_MOUSE_BUTTON_UP:
			if((smol__mouse_button_states[event->mouse.button] & SMOL_INPUT_STATE_DOWN) != (event->type == SMOL_FRAME_EVENT_MOUSE_BUTTON_DOWN)) {
				smol__mouse_button_states[event->mouse.button] ^= SMOL_INPUT_STATE_DOWN;
				smol__mouse_button_states[event->mouse.button] |= SMOL_INPUT_STATE_CHANGE;
			}
		case SMOL_FRAME_EVENT_MOUSE_MOVE:
			smol__mouse_x = event->mouse.x;
			smol__mouse_y = event->mouse.y;
			smol__mouse_move_x = event->mouse.dx;
			smol__mouse_move_y = event->mouse.dy;
		break;
		case SMOL_FRAME_EVENT_MOUSE_VER_WHEEL:
			smol__mouse_z = event->mouse.z;
			smol__mouse_move_z = event->mouse.dz;
		break;
		case SMOL_FRAME_EVENT_MOUSE_HOR_WHEEL:
			smol__mouse_z = event->mouse.w;
			smol__mouse_move_z = event->mouse.dw;
		break;
	}

}


int smol_key_hit(smol_key key) {
	return smol__key_states[key] == SMOL_INPUT_STATE_CHANGE;
}

int smol_key_up(smol_key key) {
	return smol__key_states[key] == SMOL_INPUT_STATE_UP;
}

int smol_key_down(smol_key key) {
	return smol__key_states[key] & SMOL_INPUT_STATE_DOWN;
}

int smol_mouse_hit(int button) {
	return smol__mouse_button_states[button] == SMOL_INPUT_STATE_CHANGE;
}

int smol_mouse_up(int button) {
	return smol__mouse_button_states[button] == SMOL_INPUT_STATE_UP;
}

int smol_mouse_down(int button) {
	return smol__mouse_button_states[button] & SMOL_INPUT_STATE_DOWN;
}

int smol_mouse_x() {
	return smol__mouse_x;
}

int smol_mouse_y() {
	return smol__mouse_y;
}

int smol_mouse_z() {
	return smol__mouse_z;
}

int smol_mouse_w() {
	return smol__mouse_w;
}

int smol_mouse_move_x() {
	return smol__mouse_move_x;
}

int smol_mouse_move_y() {
	return smol__mouse_move_y;
}

int smol_mouse_move_z() {
	
	return smol__mouse_move_z;
}

int smol_mouse_move_w() {
	return smol__mouse_move_w;
}

#endif 
