/*! \file input.h
 *  Get input values
 */

#ifndef INPUT_H
#define INPUT_H

#include "dengine_config.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    DENGINE_INPUT_MSEBTN_PRIMARY,
    DENGINE_INPUT_MSEBTN_SECONDARY,
    DENGINE_INPUT_MSEBTN_MIDDLE,

    DENGINE_INPUT_MSEBTN_COUNT
}MouseButton;

typedef enum
{
    DENGINE_INPUT_PAD0,
    DENGINE_INPUT_PAD1,
    DENGINE_INPUT_PAD2,
    DENGINE_INPUT_PAD3,

    /* INTERNAL */
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

    /* INTERNAL */
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

    /* INTERNAL */
    DENGINE_INPUT_PAD_AXIS_COUNT
}GamepadAxis;

typedef struct
{
    float x;
    float y;
    int isdown;
    int oneshot;
}SoftInput;

typedef struct
{
    uint32_t button;     
    uint8_t oneshot; /*!< 1 = has been triggerd. always return 0 until 
                       its set to one again. it alleviates some race conditions
                       that may occur by ensuring one thread only sets oneshot, i.e
                       window thread only touches one shot when button is released
                       while main thread sets it when once*/
    uint8_t state; /*!< 1 = down, 0 = not. if you set to 0 also set
                        oneshot to 0 too othewise it won't work again */

}ButtonInfo;

typedef struct 
{
    int mouse_x; /*!< left to right of screen */
    int mouse_y; /*!< bottom to top of screen */
    int mouse_scroll_y; /*!< +1 wheel up, -1 wheel down */
    ButtonInfo mouse_btns[DENGINE_INPUT_MSEBTN_COUNT];

    ButtonInfo keys[5];
    SoftInput touches[5];
}StandardInput;

/* usually you never need to set input source as this is done
 * by default when creating a native window. this is useful for embedding*/
void dengine_input_set_input(StandardInput* input);

int dengine_input_get_key_once(uint32_t key);

int dengine_input_get_key(uint32_t key);

int dengine_input_get_mousebtn_once(MouseButton btn);

int dengine_input_get_mousebtn(MouseButton btn);

/* touches are grouped by tiny rectangles specified by width and height.
 * and originating from x and y
 *
 * it makes it eaiser to group them but beware of ghost touches
 * where two touches are very close and it counts, for example
 * when using get_touch_once. 
 *
 * get_touch_once should be only used when the rectangle won't overlap such as
 * in a touchscreen GUI. goes without saying overlapping GUI is a bad idea
 * and won't make the user angry :(
 *
 * tip: you can get touch with x and y = 0, screen width and height which may be useful
 * for rotation of a 3d camera
 */
int dengine_input_get_touch(int x, int y, int width, int height);
int dengine_input_get_touch_once(int x, int y, int width, int height);

//double dengine_input_get_mousescroll_x();

double dengine_input_get_mousescroll_y();

double dengine_input_get_mousepos_x();

double dengine_input_get_mousepos_y();

/* GAMEPAD. WORK IN PROGRESS... */

const char* dengine_input_gamepad_get_name(GamepadID pad);

int dengine_input_gamepad_get_isconnected(GamepadID pad);

int dengine_input_gamepad_get_btn(GamepadID pad, GamepadButton btn);

int dengine_input_gamepad_get_btn_once(GamepadID pad, GamepadButton btn);

float dengine_input_gamepad_get_axis(GamepadID pad, GamepadAxis axis);

int dengine_input_gamepad_vibration_set_basic(GamepadID pad, float leftmotor, float rightmotor);

const char* dengine_input_gamepad_vibration_get_error();

#ifdef __cplusplus
}
#endif

#endif // INPUT_H
