#include <stddef.h>

#include <dengine/window.h>
#include <dengine/loadgl.h>

#include <dengine-utils/filesys.h>
#include <dengine-utils/logging.h>

#include <dengine-gui/gui.h>
#include <dengine-gui/embfonts.h>

int main(int argc, char *argv[])
{
    dengine_window_init();
    dengineutils_filesys_init();

    DengineWindow* shared = dengine_window_create(320, 180, "Shared", NULL);
    DengineWindow* win1 = dengine_window_create(320, 180, "win1", shared);
    DengineWindow* win2 = dengine_window_create(320, 180, "win2", shared);

    dengine_window_set_position(shared, 0, 0);
    dengine_window_set_position(win1, 320, 0);
    dengine_window_set_position(win2, 320 * 2, 0);

    dengine_window_makecurrent(shared);
    dengine_window_loadgl(shared);

    dengineutils_logging_log("INFO::GL: %s", glGetString(GL_VERSION));

    denginegui_init();
    denginegui_set_font(OpenSans_Light_ttf, 32.0, 512);

    while (dengine_window_isrunning(shared)) {
        dengine_window_poll(shared);

        dengine_window_makecurrent(shared);
        glClearColor(0.5, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        denginegui_text(0, 0, "Text in Shared", NULL);
        dengine_window_swapbuffers(shared);

        dengine_window_makecurrent(win1);
        glClearColor(0.0, 0.5, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        denginegui_text(0, 0, "Text in Win1", NULL);
        dengine_window_swapbuffers(win1);

        dengine_window_makecurrent(win2);
        glClearColor(0.0, 0.0, 0.5, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        denginegui_text(0, 0, "Text in Win2", NULL);
        dengine_window_swapbuffers(win2);
    }
    dengine_window_destroy(shared);
    dengine_window_destroy(win1);
    dengine_window_destroy(win2);

    dengine_window_terminate();
    dengineutils_filesys_terminate();

    return 0;
}
