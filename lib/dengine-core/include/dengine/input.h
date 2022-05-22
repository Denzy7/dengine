/*! \file input.h
 *  Get input values
 */

#ifndef INPUT_H
#define INPUT_H

#include "dengine_config.h"

#ifdef __cplusplus
extern "C" {
#endif

void dengine_input_init();

int dengine_input_get_key_once(int key);

int dengine_input_get_key(int key);

int dengine_input_get_mousebtn_once(int btn);

int dengine_input_get_mousebtn(int btn);

double dengine_input_get_mousescroll_x();

double dengine_input_get_mousescroll_y();

double dengine_input_get_mousepos_x();

double dengine_input_get_mousepos_y();

const char* dengine_input_gamepad_get_name(int pad);

int dengine_input_gamepad_get_isconnected(int pad);

int dengine_input_gamepad_get_btn(int pad, int btn);

float dengine_input_gamepad_get_axis(int pad, int axis);

int dengine_input_gamepad_vibration_set_basic(int pad, float leftmotor, float rightmotor);

const char* dengine_input_gamepad_vibration_get_error();

void dengine_input_pollevents();

#ifdef __cplusplus
}
#endif

#endif // INPUT_H
