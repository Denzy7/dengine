#ifndef GUI_H
#define GUI_H

#include "dengine/texture.h"

#ifdef __cplusplus
extern "C" {
#endif

int denginegui_init();

void denginegui_terminate();

int denginegui_set_font(const void* ttf, const float fontsize, const uint32_t bitmap_size);

float denginegui_get_fontsz();

void denginegui_text(float x,float y, const char* text, float* rgba);

void denginegui_panel(float x,float y, float width, float height, Texture* texture, float* uv, float* rgba);

int denginegui_button(float x,float y, float width, float height, const char* text, float* rgba);

#ifdef __cplusplus
}
#endif

#endif // GUI_H
