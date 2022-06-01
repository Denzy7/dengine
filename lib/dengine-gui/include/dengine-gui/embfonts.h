#ifndef EMBFONTS_H
#define EMBFONTS_H
#include "dengine_config.h"

//OPENSANS
    #ifdef DENGINE_FONT_EMBED_OPENSANS_REGULAR
    extern const unsigned int OpenSans_Regular_ttf_len;
    extern unsigned char OpenSans_Regular_ttf[];
    #endif

    #ifdef DENGINE_FONT_EMBED_OPENSANS_ITALIC
    extern const unsigned int OpenSans_Italic_ttf_len;
    extern unsigned char OpenSans_Italic_ttf[];
    #endif

    extern const unsigned int OpenSans_Light_ttf_len;
    extern unsigned char OpenSans_Light_ttf[];

// SOURCE SANS
    #ifdef DENGINE_FONT_EMBED_SOURCESANSPRO_REGULAR
    extern const unsigned int SourceSansPro_Regular_ttf_len;
    extern unsigned char SourceSansPro_Regular_ttf[];
    #endif

    #ifdef DENGINE_FONT_EMBED_SOURCESANSPRO_ITALIC
    extern const unsigned int SourceSansPro_Italic_ttf_len;
    extern unsigned char SourceSansPro_Italic_ttf[];
    #endif

    #ifdef DENGINE_FONT_EMBED_SOURCESANSPRO_LIGHT
    extern const unsigned int SourceSansPro_Light_ttf_len;
    extern unsigned char SourceSansPro_Light_ttf[];
    #endif

// ROBOTO
    #ifdef DENGINE_FONT_EMBED_ROBOTO_REGULAR
    extern const unsigned int Roboto_Regular_ttf_len;
    extern unsigned char Roboto_Regular_ttf[];
    #endif

    #ifdef DENGINE_FONT_EMBED_ROBOTO_ITALIC
    extern const unsigned int Roboto_Italic_ttf_len;
    extern unsigned char Roboto_Italic_ttf[];
    #endif

    #ifdef DENGINE_FONT_EMBED_ROBOTO_LIGHT
    extern const unsigned int Roboto_Light_ttf_len;
    extern unsigned char Roboto_Light_ttf[];
    #endif

#endif // EMBFONTS_H
