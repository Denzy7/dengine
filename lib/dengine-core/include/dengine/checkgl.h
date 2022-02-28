/*! \file checkgl.h
 *  Checking for GL errors
 */

#ifndef CHECKGL_H
#define CHECKGL_H

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief Check for GL error function. Use DENGINE_CHECKGL define instead
 * \param file Filename (will only show its relative directory)
 * \param line Line
 * \return An integer with what violation occured
 */
int dengine_checkgl(const char* file, const int line);

#ifdef __cplusplus
}
#endif

/*! \def DENGINE_CHECKGL
 *  \brief Check for GL errors. If returns non-zero, write to stderr, stdout and print os messagebox
 */
#define DENGINE_CHECKGL dengine_checkgl(__FILE__, __LINE__)

#endif // CHECKGL_H
