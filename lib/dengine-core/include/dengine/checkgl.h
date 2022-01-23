#ifndef CHECKGL_H
#define CHECKGL_H

#ifdef __cplusplus
extern "C" {
#endif

int dengine_checkgl(const char* file, const int line);

#ifdef __cplusplus
}
#endif

#define DENGINE_CHECKGL dengine_checkgl(__FILE__, __LINE__);

#endif // CHECKGL_H
