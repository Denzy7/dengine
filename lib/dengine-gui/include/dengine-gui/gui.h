#ifndef GUI_H
#define GUI_H

#include "dengine/texture.h"

int denginegui_init();

int denginegui_set_font(void* ttf, const float fontsize, unsigned int bitmap_size);

void denginegui_text(float x,float y, const char* text, float* rgba);

void denginegui_panel(float x,float y, float width, float height, Texture* texture, float* uv, float* rgba);

int denginegui_button(float x,float y, float width, float height, const char* text, float* rgba);

#endif // GUI_H
