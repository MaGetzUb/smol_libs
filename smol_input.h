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

#ifndef SMOL_INPUTS_H
#define SMOL_INPUTS_H

//Prevent implementation duplication
#ifndef SMOL_FRAME_H
#include "smol_frame.h"
#endif 

//smol_inputs_flush - Clears all the key state changes, and preservers key pressed states
void smol_inputs_flush(void);

//smol_inputs_update - Updates the internal inputs state by filtering events
//Arguments:
// - smol_frame_event_t* event -- a pointer to an event structure instance
//Returns: int -- 1 is event was handled, 0 if was not.
int smol_inputs_update(smol_frame_event_t* event);



//smol_key_hit - Tests was key pressed
//Arguments: 
// - smol_key key -- A SMOLK_* keycode for a key
//Returns: int -- containing 1, if key was pressed, 0 if wasn't.
int smol_key_hit(smol_key key);

//smol_key_up - Tests was key released
//Arguments: 
// - smol_key key -- A SMOLK_* keycode for a key
//Returns: int -- containing 1, if key was released, 0 if wasn't.
int smol_key_up(smol_key key);

//smol_key_down - Tests is key being hold down
//Arguments: 
// - smol_key key -- A SMOLK_* keycode for a key
//Returns: int -- containing 1, if key is being hold down, 0 if it isn't.
int smol_key_down(smol_key key);

//smol_chr() - Gets most recently typed character
//Returns: unsigned int - containing utf32 codepoint
unsigned int smol_chr(void);

//smol_mouse_hit - Tests was mouse button pressed
//Arguments: 
// - int button -- A mouse button index
//Returns: int -- containing 1, if key was pressed, 0 if wasn't.
int smol_mouse_hit(int button);

//smol_mouse_up - Tests was mouse button released
//Arguments: 
// - int button -- A mouse button index
//Returns: int -- containing 1, if key was released, 0 if wasn't.
int smol_mouse_up(int button);

//smol_mouse_down - Tests is mouse button being hold down
//Arguments: 
// - int button -- A mouse button index
//Returns: int -- containing 1, if mouse button is being hold down, 0 if it isn't.
int smol_mouse_down(int button);



//smol_mouse_x - Get the mouse x position in active frame space.
//Returns: int -- containing mouse coordinate on x - axis.
int smol_mouse_x(void);

//smol_mouse_y - Get the mouse x position in active frame space.
//Returns: int -- containing mouse coordinate on y - axis.
int smol_mouse_y(void);

//smol_mouse_z - Get the accumulated mouse vertical wheel orientation in active frame.
//Returns: int -- containing mouse vertical wheel orientation.
int smol_mouse_z(void);

//smol_mouse_w - Get the accumulated mouse horizontal wheel orientation in active frame.
//Returns: int -- containing mouse horizontal wheel orientation.
int smol_mouse_w(void);



//smol_mouse_move_x - Get the delta of mouse coordinate difference on y-axis.
//Returns: int -- containing mouse coordinate difference on x-axis.
int smol_mouse_move_x(void);

//smol_mouse_move_y - Get the delta of mouse coordinate difference on y-axis.
//Returns: int -- containing mouse coordinate difference on y-axis.
int smol_mouse_move_y(void);

//smol_mouse_move_z - Get the delta of mouse vertical wheel orientation in active frame.
//Returns: int -- containing mouse vertical wheel orientation delta.
int smol_mouse_move_z(void);

//smol_mouse_move_w - Get the delta of mouse horizontal wheel orientation in active frame.
//Returns: int -- containing mouse horizontal wheel orientation delta.
int smol_mouse_move_w(void);

#endif 

#ifdef SMOL_INPUT_IMPLEMENTATION

typedef enum {
	SMOL_INPUT_STATE_DOWN = 1,
	SMOL_INPUT_STATE_CHANGE = 2,
	SMOL_INPUT_STATE_HIT = 3
} smol_input_state;

unsigned char smol__key_states[256];
unsigned char smol__mouse_button_states[5];
unsigned int smol__input_codepoint;
int smol__mouse_x;
int smol__mouse_y;
int smol__mouse_z;
int smol__mouse_w;

int smol__mouse_move_x;
int smol__mouse_move_y;
int smol__mouse_move_z;
int smol__mouse_move_w;


void smol_inputs_flush(void) {

	smol__mouse_move_x = 0;
	smol__mouse_move_y = 0;
	smol__mouse_move_z = 0;
	smol__mouse_move_w = 0;

	for(int i = 0; i < 5; i++) {
		smol__mouse_button_states[i] &= SMOL_INPUT_STATE_DOWN;
	}
	for(int i = 0; i < 256; i++) {
		smol__key_states[i] &= SMOL_INPUT_STATE_DOWN;
	}
}

int smol_inputs_update(smol_frame_event_t* event) {

	switch(event->type) {
		case SMOL_FRAME_EVENT_KEY_DOWN:
		case SMOL_FRAME_EVENT_KEY_UP:
			if(event->key.code == SMOLK_UNKNOWN)
				return 1;
			if((smol__key_states[event->key.code] & SMOL_INPUT_STATE_DOWN) != (event->type == SMOL_FRAME_EVENT_KEY_DOWN)) {
				smol__key_states[event->key.code] ^= SMOL_INPUT_STATE_DOWN;
				smol__key_states[event->key.code] |= SMOL_INPUT_STATE_CHANGE;
			}
			return 1;
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
			return 1;
		break;
		case SMOL_FRAME_EVENT_MOUSE_VER_WHEEL:
			smol__mouse_z = event->mouse.z;
			smol__mouse_move_z = event->mouse.dz;
			return 1;
		break;
		case SMOL_FRAME_EVENT_MOUSE_HOR_WHEEL:
			smol__mouse_z = event->mouse.w;
			smol__mouse_move_z = event->mouse.dw;
			return 1;
		break;
		case SMOL_FRAME_EVENT_TEXT_INPUT:
			smol__input_codepoint = event->input.codepoint;
			return 1;
		break;
		default: break;
	}

	return 0;
}

unsigned int smol_chr(void) {
	return smol__input_codepoint;
}

int smol_key_hit(smol_key key) {
	return smol__key_states[key] == SMOL_INPUT_STATE_HIT;
}

int smol_key_up(smol_key key) {
	return smol__key_states[key] == SMOL_INPUT_STATE_CHANGE;
}

int smol_key_down(smol_key key) {
	return smol__key_states[key] & SMOL_INPUT_STATE_DOWN;
}

int smol_mouse_hit(int button) {
	return smol__mouse_button_states[button] == SMOL_INPUT_STATE_HIT;
}

int smol_mouse_up(int button) {
	return smol__mouse_button_states[button] == SMOL_INPUT_STATE_CHANGE;
}

int smol_mouse_down(int button) {
	return smol__mouse_button_states[button] & SMOL_INPUT_STATE_DOWN;
}

int smol_mouse_x(void) {
	return smol__mouse_x;
}

int smol_mouse_y(void) {
	return smol__mouse_y;
}

int smol_mouse_z(void) {
	return smol__mouse_z;
}

int smol_mouse_w(void) {
	return smol__mouse_w;
}

int smol_mouse_move_x(void) {
	return smol__mouse_move_x;
}

int smol_mouse_move_y(void) {
	return smol__mouse_move_y;
}

int smol_mouse_move_z(void) {
	
	return smol__mouse_move_z;
}

int smol_mouse_move_w(void) {
	return smol__mouse_move_w;
}

#endif 
