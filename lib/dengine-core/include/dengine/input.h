/*! \file input.h
 *  Get input values
 */

#ifndef INPUT_H
#define INPUT_H

#include "dengine_config.h"

#include "dengine/window.h" //windowset
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#define DENGINE_INPUT_MSEBTN_PRIMARY 0
#define DENGINE_INPUT_MSEBTN_SECONDARY 1
#define DENGINE_INPUT_MSEBTN_MIDDLE 2

void dengine_input_set_window(DengineWindow* window);

int dengine_input_get_key_once(char key);

int dengine_input_get_key(char key);

int dengine_input_get_mousebtn_once(int btn);

int dengine_input_get_mousebtn(int btn);

//double dengine_input_get_mousescroll_x();

double dengine_input_get_mousescroll_y();

double dengine_input_get_mousepos_x();

double dengine_input_get_mousepos_y();

const char* dengine_input_gamepad_get_name(int pad);

int dengine_input_gamepad_get_isconnected(int pad);

int dengine_input_gamepad_get_btn(int pad, int btn);

float dengine_input_gamepad_get_axis(int pad, int axis);

int dengine_input_gamepad_vibration_set_basic(int pad, float leftmotor, float rightmotor);

const char* dengine_input_gamepad_vibration_get_error();

#ifdef __cplusplus
}
#endif

#endif // INPUT_H
