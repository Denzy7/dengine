#ifndef GUI_H
#define GUI_H

#include "dengine/texture.h"

#ifdef __cplusplus
extern "C" {
#endif

int denginegui_init();

/* some 3.1 drivers (Sandy Bridge Win32: 9.17.10.4459) as well as
 * some random GLES 2.0 implementations of glBufferSubData are kinda broken :(  
 *
 * switch between glBufferData and glBufferSubData to see which has better
 * performance
 */
void denginegui_use_subdata(int state);

void denginegui_terminate();

int denginegui_set_font(const void* ttf, const float fontsize, const uint32_t bitmap_size);

void denginegui_set_button_repeatable(int state);

void denginegui_set_panel_discard(int state);

void denginegui_set_panel_discard_threshold(float threshold);

float denginegui_get_fontsz();

void denginegui_text(float x,float y, const char* text, float* rgba);

void denginegui_panel(float x,float y, float width, float height, Texture* texture, float* uv, float* rgba);

int denginegui_button(float x,float y, float width, float height, const char* text, float* rgba);

#ifdef __cplusplus
}
#endif

#endif // GUI_H
