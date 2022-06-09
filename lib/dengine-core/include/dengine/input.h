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

typedef enum
{
    DENGINE_INPUT_MSEBTN_PRIMARY,
    DENGINE_INPUT_MSEBTN_SECONDARY,
    DENGINE_INPUT_MSEBTN_MIDDLE
}MouseButton;

typedef enum
{
    DENGINE_INPUT_PAD0,
    DENGINE_INPUT_PAD1,
    DENGINE_INPUT_PAD2,
    DENGINE_INPUT_PAD3,

    /* INTERNAL. DO NOT USE */
    DENGINE_INPUT_PAD_COUNT,
}GamepadID;

typedef enum
{
    DENGINE_INPUT_PAD_BUTTON_DPAD_UP,
    DENGINE_INPUT_PAD_BUTTON_DPAD_DOWN,
    DENGINE_INPUT_PAD_BUTTON_DPAD_LEFT,
    DENGINE_INPUT_PAD_BUTTON_DPAD_RIGHT,
    DENGINE_INPUT_PAD_BUTTON_START,
    DENGINE_INPUT_PAD_BUTTON_BACK,
    DENGINE_INPUT_PAD_BUTTON_LS,
    DENGINE_INPUT_PAD_BUTTON_RS,
    DENGINE_INPUT_PAD_BUTTON_LB,
    DENGINE_INPUT_PAD_BUTTON_RB,
    DENGINE_INPUT_PAD_BUTTON_A,
    DENGINE_INPUT_PAD_BUTTON_B,
    DENGINE_INPUT_PAD_BUTTON_X,
    DENGINE_INPUT_PAD_BUTTON_Y,

    /* INTERNAL. DO NOT USE */
    DENGINE_INPUT_PAD_BUTTON_COUNT,
}GamepadButton;

typedef enum
{
    DENGINE_INPUT_PAD_AXIS_LT,
    DENGINE_INPUT_PAD_AXIS_RT,
    DENGINE_INPUT_PAD_AXIS_LX,
    DENGINE_INPUT_PAD_AXIS_LY,
    DENGINE_INPUT_PAD_AXIS_RX,
    DENGINE_INPUT_PAD_AXIS_RY,

    /* INTERNAL. DO NOT USE */
    DENGINE_INPUT_PAD_AXIS_COUNT
}GamepadAxis;

void dengine_input_set_window(DengineWindow* window);

int dengine_input_get_key_once(char key);

int dengine_input_get_key(char key);

int dengine_input_get_mousebtn_once(MouseButton btn);

int dengine_input_get_mousebtn(MouseButton btn);

//double dengine_input_get_mousescroll_x();

double dengine_input_get_mousescroll_y();

double dengine_input_get_mousepos_x();

double dengine_input_get_mousepos_y();

/* GAMEPAD. WORK IN PROGRESS... */

//int dengine_input_gamepad_poll();

//const char* dengine_input_gamepad_get_name(GamepadID pad);

//int dengine_input_gamepad_get_isconnected(GamepadID pad);

//int dengine_input_gamepad_get_btn(GamepadID pad, GamepadButton btn);

//int dengine_input_gamepad_get_btn_once(GamepadID pad, GamepadButton btn);

//float dengine_input_gamepad_get_axis(GamepadID pad, GamepadAxis axis);

//int dengine_input_gamepad_vibration_set_basic(GamepadID pad, float leftmotor, float rightmotor);

//const char* dengine_input_gamepad_vibration_get_error();

#ifdef __cplusplus
}
#endif

#endif // INPUT_H
