#ifndef GUI_H
#define GUI_H

int denginegui_init();

int denginegui_set_font(void* ttf, const float fontsize, unsigned int bitmap_size);

void denginegui_text(float x,float y, const char* text, float* rgb);
#endif // GUI_H
